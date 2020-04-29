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

//extern struct SpareData;
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
    printf("ftl_open() 실행\n");
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
    int psn = -1;
    int i;
    for (i = 0; i < DATABLKS_PER_DEVICE; i++)
        if(AMT[i].lsn == lsn)
            psn = AMT[i].psn;

    if(psn == -1) {
        fprintf(stderr, "no data in lsn(%d)\n", lsn);
        exit(1);
    }
    memset(pagebuf, 0, PAGE_SIZE);
    dd_read(psn, pagebuf);
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
    
    SpareData spare;
    printf("ftl wirte 실행은됨, emptypsn : %d\n",emptyPSN);
    if (emptyPSN >= 0) { //free block 제외, 빈 섹터 있는경우    
        if((AMT[lsn].lsn == lsn) && (AMT[lsn].psn != -1)) { //update 해야 하는경우 : 기존데이터 invalid 후 write()
            dd_read(AMT[lsn].psn, pagebuf);
            memcpy(sparebuf, pagebuf+SECTOR_SIZE, SPARE_SIZE);
            spare = set_spare(lsn, TRUE, sparebuf+SECTOR_SIZE);
            BIT[AMT[lsn].psn/PAGES_PER_BLOCK].isn++;
            
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
        printf("pagebuf:%s, 크기 : %ld\n", pagebuf,sizeof(pagebuf));
        BIT[AMT[lsn].psn/PAGES_PER_BLOCK].full++;
        ftl_print();
        printf("dd_Write()전 - emptyPSN : %d\n", emptyPSN);
        if(dd_write(emptyPSN, pagebuf) == -1) {
            fprintf(stderr, "fwrite error\n");
            exit(1);
        }
        printf("ftl_write 성공!\n");
        return ;
    }
    
    else { //빈 섹터가 없는 경우 : garbage block을 erase하고 wrte함
        erase_garbage_block();
        ftl_write(lsn, sectorbuf);
    }
    printf("ftl_write 성공!\n");
	return;
}

SpareData set_spare(int lsn, int is_invalid, char *dummy) { //파라미터로 입력한 spare data의 구성요소를 spareData구조체로 만들어줌
    SpareData spare;

    printf("set_spare() 함수 호출성공\n");
    spare.lpn = lsn;
    spare.is_invalid = is_invalid;
    printf("대입까지성공\n");
    printf("파라미터 더미  : %s\n", dummy);
    strncpy(spare.dummy, dummy, 8);
    printf("dummydata 복사성공\n");   
    return spare;
}

void spare_to_str(char *dest, SpareData spare){ //spareData구조체의 멤버들을 문자열에 저장되도록 변경해 dest에 저장.
    sprintf(dest, "%d%d%s", spare.lpn, spare.is_invalid, spare.dummy);
    
    return;
}

void erase_garbage_block() { //garbage block의 valid sector를 freeblock으로 이동시킨 후, garbage block을 삭제하는 함수
    int garbagePBN, maxISN = -1;
    int i, j=0, sppn, eppn;
    char pagebuf[PAGE_SIZE];
    int isInvalid;
    char tmpbuf[PAGE_SIZE];

    for (i = 0; i < DATABLKS_PER_DEVICE; i++)
        if (BIT[i].isn > maxISN) {
            garbagePBN = BIT[i].pbn;
            maxISN = BIT[i].isn;
        }
    //free block 제외시 만약 남은 sector가 없는데 wrtie()요청이 들어온다면??? 
    // ====> error 처리 해야함!!!!!
    sppn = garbagePBN*PAGES_PER_BLOCK;
    eppn = (garbagePBN+1)/PAGES_PER_BLOCK;
    for (i = sppn; i < eppn; i++) {
        dd_read(i, pagebuf);
        memcpy(&isInvalid, pagebuf+SECTOR_SIZE+4, sizeof(int));
        if(!isInvalid) { //valid한 데이터일 경우
            dd_write(freePBN*4+j, pagebuf);
            j++;
        }
    }
    BIT[freePBN].full = j;
    dd_erase(garbagePBN);
    freePBN = garbagePBN;
    BIT[freePBN].full = 0;
    BIT[freePBN].isn = 0;
}

int search_empty_psn() { //free block을 제외하고, 빈 sector가 있으면 해당 psn을 리턴. 없으면 FALSE리턴
    int i;
    printf("search_empty()\n");
    for (i = 0; i < DATABLKS_PER_DEVICE; i++) {
        if (AMT[i].psn == -1)
            return AMT[i].lsn;
    }
    return FALSE;
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
