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
void spare_to_str(char *dest, SpareData spare);
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
    int emptyPSN = search_empty_psn(); 
    //printf("emptyPSN : %d\n", emptyPSN);
    SpareData spare;
    
    if (0 <= emptyPSN && emptyPSN < DATAPAGES_PER_DEVICE) { //free block 제외, 빈 섹터 있는경우    
        if((AMT[lsn].lsn == lsn) && (AMT[lsn].psn != -1)) { //update 해야 하는경우 : 기존데이터 invalid 후 write()
            dd_read(AMT[lsn].psn, pagebuf);
            memcpy(sparebuf, pagebuf+SECTOR_SIZE, SPARE_SIZE);
            spare = set_spare(lsn, TRUE, sparebuf+SECTOR_SIZE);
            BIT[AMT[lsn].psn/PAGES_PER_BLOCK].isn++;
            printf("BIT[%d]의 isn : %d\n", AMT[lsn].psn/PAGES_PER_BLOCK, BIT[AMT[lsn].psn/PAGES_PER_BLOCK].isn);
            
            spare_to_str(sparebuf, spare);
            memcpy(pagebuf+SECTOR_SIZE, sparebuf, SPARE_SIZE);
            dd_write(AMT[lsn].psn, pagebuf); //기존에 있던 pbn을 invalid로 재저장
        }
        //빈 psn에 처음 쓰는 경우 바로 write()
        AMT[lsn].psn = emptyPSN;
        spare = set_spare(lsn, FALSE, dummy);
        spare_to_str(sparebuf, spare);
                
        memcpy(pagebuf, sectorbuf, SECTOR_SIZE);
        memcpy(pagebuf+SECTOR_SIZE, sparebuf, SPARE_SIZE);
        BIT[AMT[lsn].psn/PAGES_PER_BLOCK].full++;
        
        if(dd_write(emptyPSN, pagebuf) == -1) {
            fprintf(stderr, "fwrite error\n");
            exit(1);
        }
        return ;
    }
    else { //빈 섹터가 없는 경우 : garbage block을 erase하고 wrte함
        erase_garbage_block();
        ftl_write(lsn, sectorbuf);
    }
	return;
}

SpareData set_spare(int lsn, int is_invalid, char *dummy) { //파라미터로 입력한 spare data의 구성요소를 spareData구조체로 만들어줌
    SpareData spare;

    spare.lpn = lsn;
    spare.is_invalid = is_invalid;
    strncpy(spare.dummy, dummy, 8);
    return spare;
}

void spare_to_str(char *dest, SpareData spare){ //spareData구조체의 멤버들을 문자열에 저장되도록 변경해 dest에 저장.
    sprintf(dest, "%d%d%s", spare.lpn, spare.is_invalid, spare.dummy);
    
    return;
}

void erase_garbage_block() { //garbage block의 valid sector를 freeblock으로 이동시킨 후, garbage block을 삭제하는 함수
    //printf("erase 실행\n");
    int garbagePBN, maxISN = -1;
    int i, j=0, sppn, eppn;
    char pagebuf[PAGE_SIZE];
    char tmpbuf[PAGE_SIZE];
    char sectorbuf[SECTOR_SIZE];
    int isInvalid;
    int lpn;
    SpareData spare;
    memset(pagebuf, 0, PAGE_SIZE);
    memset(tmpbuf, 0, PAGE_SIZE);

    for (i = 0; i < DATABLKS_PER_DEVICE; i++)
        if (BIT[i].isn > maxISN) {
            garbagePBN = BIT[i].pbn;
            maxISN = BIT[i].isn;
        }
    
    printf("garbagePBN : %d, maxISN : %d\n", garbagePBN, maxISN);

    if(maxISN == -1) { //free block을 제외하고,invalid sector가 없는데도 erase()요청이 온 경우 => 더 이상 쓸수 있는 자리 없으므로 에러처리
        fprintf(stderr, "no invaild data!! can't write anymore...\n");
        exit(1);
    }

    sppn = garbagePBN*PAGES_PER_BLOCK;
    eppn = (garbagePBN+1)*PAGES_PER_BLOCK;
    for (i = sppn; i < eppn; i++) {
        dd_read(i, pagebuf);
        printf("sectorbuf : %d, %d\n", pagebuf+SECTOR_SIZE, pagebuf+SECTOR_SIZE+4);
        //memcpy(&spare, pagebuf+SECTOR_SIZE, sizeof(SpareData));
        memcpy(&lpn, pagebuf+SECTOR_SIZE, sizeof(int));
        memcpy(&isInvalid , pagebuf+SECTOR_SIZE+4, sizeof(int));
        //lpn = spare.lpn;
        //isInvalid = spare.is_invalid;
        printf("lpn : %d, inInvalid : %d\n", lpn, isInvalid);
        exit(1);
        if(isInvalid == 0) { //valid한 데이터일 경우
            //printf("isInvald : %d", isInvalid);
            dd_write(freePBN*4+j, pagebuf);
            AMT[lpn].psn = i;
            j++;
        }
    }
    BIT[freePBN].full = j;
    
    dd_erase(garbagePBN);
    freePBN = garbagePBN;
    BIT[freePBN].full = 0;
    BIT[freePBN].isn = 0;
    //printf("erawes 완료\n");
}

int search_empty_psn() { //free block을 제외하고, 빈 sector가 있으면 새로 저장할 psn을 리턴. 없으면 -1리턴
    int i;
    int max = -99;

    for (i = 0; i < DATAPAGES_PER_DEVICE; i++) //제일 큰 psn번호 찾기
        if (AMT[i].psn > max)
            max = AMT[i].psn;
    max++;
    //printf("max : %d\n", max);
    for (i = 0; i < DATAPAGES_PER_DEVICE; i++) //빈 sector있으면 제일 큰 psn 번호 다음 번호 리턴해줌
        if (AMT[i].psn == -1 && max < DATAPAGES_PER_DEVICE)
            return max; 

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
