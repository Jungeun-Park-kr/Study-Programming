// 주의사항
// 1. sectormap.h에 정의되어 있는 상수 변수를 우선적으로 사용해야 함
// 2. sectormap.h에 정의되어 있지 않을 경우 본인이 이 파일에서 만들어서 사용하면 됨
// 3. 필요한 data structure가 필요하면 이 파일에서 정의해서 쓰기 바람(sectormap.h에 추가하면 안됨)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "sectormap.h"

#define PAGES_PER_DEVICE (PAGES_PER_BLOCK * BLOCKS_PER_DEVICE)
struct AddressMappingTable {
    int lsn;
    int psn;
};

struct BlockInformationTable {
    int pbn;
    int full;
    int isn;
};

struct AddressMappingTable AMT[DATAPAGES_PER_DEVICE];
struct BlockInformationTable BIT[BLOCKS_PER_DEVICE];
int freePBN;
int curpsn;

int dd_write(int ppn, char *pagebuf);
int dd_read(int ppn, char *pagebuf);
int dd_erase(int pbn);

void ftl_open();
void ftl_read(int lsn, char *sectorbuf);
void ftl_write(int lsn, char *sectorbuf);
SpareData set_spare(int lsn, int is_invalid, char *dummy);
//void spare_to_str(char *dest, SpareData spare);
void erase_garbage_block();
int search_empty_psn();
void ftl_print();


void ftl_open()
{   
    int i;
    for(i = 0; i <= DATAPAGES_PER_DEVICE; i++) {
        AMT[i].lsn = i;
        AMT[i].psn = -1;
    }
    for(i = 0; i < BLOCKS_PER_DEVICE; i++) {
        BIT[i].pbn = i;
        BIT[i].full = 0;
        BIT[i].isn = 0;
    }
    freePBN = BIT[BLOCKS_PER_DEVICE-1].pbn; //최초 파일 생성시 마지막 pbn이 free block임

    return;
}

void ftl_read(int lsn, char *sectorbuf)
{
    char pagebuf[PAGE_SIZE];
    /*
    int psn = -1;
    int i;
    for (i = 0; i < DATABLKS_PER_DEVICE; i++)
        if(AMT[i].lsn == lsn)
            psn = AMT[i].psn;
    */
    if(AMT[lsn].psn == -1) {
        fprintf(stderr, "no data in lsn(%d)\n", lsn);
        exit(1);
    }
    memset(pagebuf, 0, PAGE_SIZE);
    dd_read(AMT[lsn].psn, pagebuf);
    memcpy(sectorbuf, pagebuf, SECTOR_SIZE);

	return;
}


void ftl_write(int lsn, char *sectorbuf)
{
    
    char pagebuf[PAGE_SIZE];
    char sparebuf[SPARE_SIZE];
    char dummy[SPARE_SIZE-8];
    memset(dummy, 0xFF, SPARE_SIZE-8);
    memset(pagebuf, 0, PAGE_SIZE);
    memset(sparebuf, 0, SPARE_SIZE);
    int emptyPSN = search_smallest_psn();
    //printf("emptyPSN : %d\n", emptyPSN);
    SpareData spare;
    
    if (0 <= emptyPSN && emptyPSN < PAGES_PER_DEVICE) { //free block 제외, 빈 섹터 있는경우    
        if((AMT[lsn].lsn == lsn) && (AMT[lsn].psn != -1)) { //update 해야 하는경우 : 기존데이터 invalid 후 write()
            dd_read(AMT[lsn].psn, pagebuf);
            memcpy(sparebuf, pagebuf+SECTOR_SIZE, SPARE_SIZE);
            spare = set_spare(lsn, TRUE, sparebuf+SECTOR_SIZE);
            BIT[AMT[lsn].psn/PAGES_PER_BLOCK].isn++;
            //printf("BIT[%d]의 isn : %d\n", AMT[lsn].psn/PAGES_PER_BLOCK, BIT[AMT[lsn].psn/PAGES_PER_BLOCK].isn);
            
            memcpy(pagebuf+SECTOR_SIZE, &spare, sizeof(SpareData)); 
            dd_write(AMT[lsn].psn, pagebuf); //기존에 있던 pbn을 invalid로 재저장
        }
        //빈 psd에 입력받은 data write()
        AMT[lsn].psn = emptyPSN; 
        spare = set_spare(lsn, FALSE, dummy);
                
        memcpy(pagebuf, sectorbuf, SECTOR_SIZE);
        memcpy(pagebuf+SECTOR_SIZE, &spare, sizeof(SpareData));
        BIT[AMT[lsn].psn/PAGES_PER_BLOCK].full++;
        
        if(dd_write(emptyPSN, pagebuf) == -1) {
            fprintf(stderr, "fwrite error\n");
        }
        return ;
    }
    else { //빈 섹터가 없는 경우 : garbage block을 erase하고 write함
        erase_garbage_block();
        ftl_write(lsn, sectorbuf);
    }
	return;
}

SpareData set_spare(int lsn, int is_invalid, char *dummy) { //파라미터로 입력한 spare data의 구성요소를 spareData구조체로 만들어줌
    SpareData spare;

    spare.lpn = lsn;
    spare.is_invalid = is_invalid;
    memcpy(spare.dummy, dummy, sizeof(char)*8);
    return spare;
}

void erase_garbage_block() { //garbage block의 valid sector를 freeblock으로 이동시킨 후, garbage block을 삭제하는 함수
    //printf("erase 실행\n");
    int garbagePBN, maxISN = -1;
    int i, j=0, sppn, eppn, newppn;
    char pagebuf[PAGE_SIZE];
    char tmpbuf[PAGE_SIZE];
    SpareData spare;
    memset(pagebuf, 0, PAGE_SIZE);
    memset(tmpbuf, 0, PAGE_SIZE);

    for (i = 0; i < DATABLKS_PER_DEVICE; i++)
        if (BIT[i].isn > maxISN) {
            garbagePBN = BIT[i].pbn;
            maxISN = BIT[i].isn;
        }
    
    if(maxISN == -1) { //free block을 제외하고,invalid sector가 없는데도 erase()요청이 온 경우 => 더 이상 쓸수 있는 자리 없으므로 에러처리
        fprintf(stderr, "no invaild data!! can't write anymore...\n");
        exit(1);
    }

    //garage block 안에 있던 valid data를 free blcok으로 이동시킨 후, garbage block을 erase()하고, 테이블을 초기화함.
    sppn = garbagePBN*PAGES_PER_BLOCK;
    eppn = (garbagePBN+1)*PAGES_PER_BLOCK;
    for (i = sppn; i < eppn; i++) {
        dd_read(i, pagebuf);
        memcpy(&spare, pagebuf+SECTOR_SIZE, sizeof(SpareData));
        //printf("lpn : %d, inInvalid : %d\n", lpn, isInvalid);
        if(spare.is_invalid == FALSE) { //valid한 데이터일 경우
            newppn = freePBN * PAGES_PER_BLOCK + j;
            dd_write(newppn, pagebuf);
            AMT[spare.lpn].psn = newppn;
            j++;
        }
    }
    BIT[freePBN].full = j;
    printf("기존 freePBN - BIT[%d].full = %d\n",freePBN, j);
    dd_erase(garbagePBN);
    freePBN = garbagePBN;
    BIT[freePBN].full = 0;
    BIT[freePBN].isn = 0;
    printf("new freePBN - BIT[%d].full = %d\n", freePBN,BIT[freePBN].full);
}

int search_smallest_psn() { //free block을 제외하고, 빈 sector가 있으면 새로 저장할 psn을 리턴. 없으면 -1리턴
    int i, st, ed;
    int max = -99;
    char tmpbuf[PAGE_SIZE];
    char pagebuf[PAGE_SIZE];
    SpareData spare;


    int isfull = TRUE;
    for(i = 0; i < BLOCKS_PER_DEVICE; i++) {
        if(i == freePBN)
            continue;

        if(BIT[i].full != PAGES_PER_BLOCK) {
            isfull = FALSE;
            break;
        }
    }
    if(isfull) {
        printf("꽉찼음\n");
        return -1;
    }



    memset(tmpbuf, (char)0xFF, PAGE_SIZE);
    st = freePBN * PAGES_PER_BLOCK;
    ed = (freePBN+1) * PAGES_PER_BLOCK;

    for (i = 0; i < PAGES_PER_DEVICE; i++) {
        if(st <= i && i < ed) //free블록 제외
            continue;
        dd_read(i, pagebuf);
        if (!memcmp(pagebuf, tmpbuf, PAGE_SIZE)) { //비어있는 psn
            printf("처음 씀(%d)!\n", i);
            return i;
        }
        memcpy(&spare, pagebuf+SECTOR_SIZE, sizeof(SpareData));
        if (spare.is_invalid == TRUE) { 
            if (max < i)
                max = i;
        }
    }    
    max++;
    if(max < 0)
        return max;
    else
        return max;
}

int get_empty_psn() {
    char pagebuf[PAGE_SIZE];
    char tmpbuf[PAGE_SIZE];
    SpareData spare;
    int i, sfr, efr;
    int minpsd = -99;

    memset(tmpbuf, (char)0xFF, PAGE_SIZE);
    sfr = freePBN * PAGES_PER_BLOCK;
    efr = (freePBN+1) * PAGES_PER_BLOCK;

    for(i = 0; i < PAGES_PER_DEVICE; i++) {
        if (sfr < i && i < efr)
            continue;
        dd_read(i, pagebuf);
        if(!memcmp(tmpbuf, pagebuf, PAGE_SIZE)) //아직 psn에 아무런 데이터가 없는경우, 해당 번호에 바로 저장하면됨
            return i;
        memcpy(&spare, pagebuf+SECTOR_SIZE, sizeof(SpareData));
        if (spare.is_invalid == FALSE && i > minpsd) //사용가능한 psn이 있는경우, 해당 번호 리턴
            return i;
    }
    
    return -1; //더이상 빈자리가 없는경우
}

int search_empty_psn() { //free block을 제외하고, 빈 sector가 있으면(=> valid sector이면) 새로 저장할 psn을 리턴. 없으면 -1리턴
    /* 각 psn이 비었는지의 결과를 위해 psn 크기만큼의 배열을 생성한다
     * dd_read()로 각 psn의 내용을 읽어와서, spareData 부분의 is_invalid = false 인 배열에 true(할당되어있음)을 저장한다.
     * 모든 psn을 확인한 후, true들의 개수와  DATAPAGES_PER_DEVICE와 같으면(꽉찼으면) false를 리턴한다.
     * DATAPAGES_PER_DEVIE 보다 작으면, 아직 공간이 남아있는 것이고, 해당 lsn에 매핑되는 psn과 상관없이 그 자리에 바로 쓸 수 있다(이전psn이든, -1이든)
     *                                  따라서 가장 작은 true를 가진 psn번호를 리턴한다.
     */
    
    int i, max;
    char pagebuf[PAGE_SIZE];
    int validPSN[PAGES_PER_DEVICE];
    memset(validPSN, FALSE, 1);
    SpareData spare;

    for (i = 0; i < DATAPAGES_PER_DEVICE; i++) { 
        dd_read(AMT[i].psn, pagebuf);
        memcpy(&spare, pagebuf+SECTOR_SIZE, sizeof(SpareData));
        if(spare.is_invalid == FALSE) {
            validPSN[AMT[i].psn] = TRUE;
            continue;
        }
    }

    //freeblock에 있는 pbn은 사용 불가능하니까 false 처리
    max = (freePBN+1)*PAGES_PER_BLOCK;
    for (i = freePBN*4; i <max; i++)
        validPSN[i] = FALSE;

    max = PAGES_PER_DEVICE * BLOCKS_PER_DEVICE;
    for (i = 0; i < max; i++) 
        if(validPSN[i] == TRUE)
            return i;
    
    return -1;
}

void ftl_print()
{
    int i, j;
    
    printf("%3s %3s\n", "lpn", "ppn");

    for(i = 0; i < DATAPAGES_PER_DEVICE; i++) {
        printf("%3d %3d\n", AMT[i].lsn, AMT[i].psn);
    }
    printf("free block's pbn = %d\n", freePBN);

	return;
}
