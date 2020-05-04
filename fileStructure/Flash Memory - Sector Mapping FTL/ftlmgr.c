#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "sectormap.h"

#define PAGES_PER_DEVICE (PAGES_PER_BLOCK * BLOCKS_PER_DEVICE)

struct AddressMappingTable { //Sector Mapping을 위한 FTL table
	int lsn; 
	int psn;
};

struct BlockInformationTable { //block 정보를 저장하는 table
	int pbn; //블록의 pbn
	int full; //해당 블록에 몇 개의 page가 차있는지 저장
	int isn; //해당 블록의 invalid한 page가 몇 개 있는지 저장
};

struct AddressMappingTable AMT[DATAPAGES_PER_DEVICE];
struct BlockInformationTable BIT[BLOCKS_PER_DEVICE];
int freePBN;

int dd_write(int ppn, char *pagebuf);
int dd_read(int ppn, char *pagebuf);
int dd_erase(int pbn);

void ftl_open();
void ftl_read(int lsn, char *sectorbuf);
void ftl_write(int lsn, char *sectorbuf);
SpareData set_spare(int lsn, int is_invalid, char *dummy);
void erase_garbage_block();
int search_smallest_psn();
void ftl_print();


void ftl_open()
{
	int i;
	for (i = 0; i <= DATAPAGES_PER_DEVICE; i++) {
		AMT[i].lsn = i;
		AMT[i].psn = -1;
	}
	for (i = 0; i < BLOCKS_PER_DEVICE; i++) {
		BIT[i].pbn = i;
		BIT[i].full = 0;
		BIT[i].isn = 0;
	}
	freePBN = BIT[BLOCKS_PER_DEVICE - 1].pbn; //최초 파일 생성시 마지막 pbn이 free block임

	return;
}

void ftl_read(int lsn, char *sectorbuf)
{
	char pagebuf[PAGE_SIZE];
	if (AMT[lsn].psn == -1) { //입력받은 lsn에 AMT에 매핑되는 psn이 0이면 에러처리
		fprintf(stderr, "no data in lsn(%d)\n", lsn);
		exit(1);
	}
	memset(pagebuf, 0, PAGE_SIZE);
	dd_read(AMT[lsn].psn, pagebuf); //매핑되는 psn의 내용을 읽어옴
	memcpy(sectorbuf, pagebuf, SECTOR_SIZE); //sector data만 저장

	return;
}


void ftl_write(int lsn, char *sectorbuf)
{
	char pagebuf[PAGE_SIZE];
	char sparebuf[SPARE_SIZE];
	char dummy[SPARE_SIZE - 8];
	memset(dummy, 0xFF, SPARE_SIZE - 8);
	memset(pagebuf, 0, PAGE_SIZE);
	memset(sparebuf, 0, SPARE_SIZE);
	int emptyPSN = search_smallest_psn();
	SpareData spare;

	if (0 <= emptyPSN && emptyPSN < PAGES_PER_DEVICE) { //free block 제외, 빈 섹터 있는경우    
		if ((AMT[lsn].lsn == lsn) && (AMT[lsn].psn != -1)) { //update 해야 하는경우 : 기존데이터 invalid 후 write()
			dd_read(AMT[lsn].psn, pagebuf);
			memcpy(sparebuf, pagebuf + SECTOR_SIZE, SPARE_SIZE);
			spare = set_spare(lsn, TRUE, sparebuf + SECTOR_SIZE);
			BIT[AMT[lsn].psn / PAGES_PER_BLOCK].isn++;
			
			memcpy(pagebuf + SECTOR_SIZE, &spare, sizeof(SpareData));
			dd_write(AMT[lsn].psn, pagebuf); //기존에 있던 pbn을 invalid로 재저장
		}
		//빈 psd에 입력받은 data write()
		AMT[lsn].psn = emptyPSN;
		spare = set_spare(lsn, FALSE, dummy);

		memcpy(pagebuf, sectorbuf, SECTOR_SIZE);
		memcpy(pagebuf + SECTOR_SIZE, &spare, sizeof(SpareData));
		BIT[AMT[lsn].psn / PAGES_PER_BLOCK].full++;

		if (dd_write(emptyPSN, pagebuf) == -1) {
			fprintf(stderr, "fwrite error\n");
		}
		return;
	}
	else { //빈 섹터가 없는 경우 : garbage block을 erase하고 write함
		erase_garbage_block();
		ftl_write(lsn, sectorbuf); //빈 섹터가 생길때까지 재귀호출 해서 erase 반복!
	}
	return;
}

SpareData set_spare(int lsn, int is_invalid, char *dummy) { //파라미터로 입력한 spare data의 구성요소를 spareData구조체로 만들어줌
	SpareData spare;

	spare.lpn = lsn;
	spare.is_invalid = is_invalid;
	memcpy(spare.dummy, dummy, sizeof(char) * 8);
	return spare;
}

void erase_garbage_block() { //garbage block의 valid sector를 freeblock으로 이동시킨 후, garbage block을 삭제하는 함수
	int garbagePBN, maxISN = -1;
	int i, j = 0, sppn, eppn, newppn;
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

	if (maxISN == -1) { //free block을 제외하고,invalid sector가 없는데도 erase()요청이 온 경우 => 더 이상 쓸수 있는 자리 없으므로 에러처리
		fprintf(stderr, "no invaild data!! can't write anymore...\n");
		exit(1);
	}

	//garage block 안에 있던 valid data를 free blcok으로 이동시킨 후, garbage block을 erase()하고, 테이블을 초기화함.
	sppn = garbagePBN * PAGES_PER_BLOCK;
	eppn = (garbagePBN + 1)*PAGES_PER_BLOCK;
	for (i = sppn; i < eppn; i++) {
		dd_read(i, pagebuf);
		memcpy(&spare, pagebuf + SECTOR_SIZE, sizeof(SpareData));
		if (spare.is_invalid == FALSE) { //valid한 데이터일 경우
			newppn = freePBN * PAGES_PER_BLOCK + j;
			dd_write(newppn, pagebuf);
			AMT[spare.lpn].psn = newppn;
			j++;
		}
	}
	BIT[freePBN].full = j;
	dd_erase(garbagePBN);
	freePBN = garbagePBN;
	BIT[freePBN].full = 0;
	BIT[freePBN].isn = 0;
}

int search_smallest_psn() { //free block을 제외하고, 빈 sector가 있으면 새로 저장할 psn을 리턴. 없으면 -1리턴
	int i, st, ed;
	int max = -99;
	char tmpbuf[PAGE_SIZE];
	char pagebuf[PAGE_SIZE];
	SpareData spare;

	int isfull = TRUE;
	for (i = 0; i < BLOCKS_PER_DEVICE; i++) {
		if (i == freePBN) //free block은 포함하지 않음
			continue;

		if (BIT[i].full != PAGES_PER_BLOCK) { 
			isfull = FALSE;
			break;
		}
	}
	if (isfull) { //꽉찼으면 -1 리턴함
		return -1;
	}
    //빈 sector가 있을 경우, 저장할 psn 번호를 리턴해줌
	memset(tmpbuf, (char)0xFF, PAGE_SIZE);
	st = freePBN * PAGES_PER_BLOCK;
	ed = (freePBN + 1) * PAGES_PER_BLOCK;

	for (i = 0; i < PAGES_PER_DEVICE; i++) {
		if (st <= i && i < ed) //free블록 제외
			continue;
		dd_read(i, pagebuf);
		if (!memcmp(pagebuf, tmpbuf, PAGE_SIZE)) { //비어있는 psn
			return i;
		}
		memcpy(&spare, pagebuf + SECTOR_SIZE, sizeof(SpareData));
		if (spare.is_invalid == TRUE) {
			if (max < i)
				max = i;
		}
	}
	max++;
	if (max < 0)
		return max;
	else
		return max;
}

void ftl_print()
{
	int i, j;

	printf("%3s %3s\n", "lpn", "ppn");

	for (i = 0; i < DATAPAGES_PER_DEVICE; i++) { 
		printf("%-3d %-3d\n", AMT[i].lsn, AMT[i].psn);
	}
	printf("free block's pbn=%d\n", freePBN);

	return;
}
