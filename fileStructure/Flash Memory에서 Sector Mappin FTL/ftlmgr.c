// 주의사항
// 1. sectormap.h에 정의되어 있는 상수 변수를 우선적으로 사용해야 함
// 2. sectormap.h에 정의되어 있지 않을 경우 본인이 이 파일에서 만들어서 사용하면 됨
// 3. 필요한 data structure가 필요하면 이 파일에서 정의해서 쓰기 바람(sectormap.h에 추가하면 안됨)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "sectormap.h"
// 필요한 경우 헤더 파일을 추가하시오.
typedef struct {
    int pbn; //pbn(struct 배열의 인덱스와 같음)
    int full; //이 block내에 valid sector 개수 (꽉 찼는지 확인위해 필요)
    int isn; //block 내의 invalid sector 개수 (garbage blcok인지 확인위해 필요)
} BlockInformationTable;
//
// flash memory를 처음 사용할 때 필요한 초기화 작업, 예를 들면 address mapping table에 대한
// 초기화 등의 작업을 수행한다. 따라서, 첫 번째 ftl_write() 또는 ftl_read()가 호출되기 전에
// file system에 의해 반드시 먼저 호출이 되어야 한다.
//

BlockInformationTable BIT[BLOCKS_PER_DEVICE]; //삭제할 블록을 관리하기 위한 block Information Table 배열 생성
int AMT[DATAPAGES_PER_DEVICE][2]; //(free block제외) 실  Data 페이지 개수의 행, 2개의 열을 가진 Address Mapping Table을 선언

FILE *flashfp;

void ftl_open()
{
	// address mapping table 초기화
	// free block's pbn 초기화
    // address mapping table에서 lbn 수는 DATABLKS_PER_DEVICE 동일
	int i;
    for (i = 0; i < DATAPAGES_PER_DEVICE; i++) {
        AMT[i][0] = i;
        AMT[i][1] = -1;
    }
    for (i = 0; i < BLOCKS_PER_DEVICE; i++) {
        BIT[i].pbn = i;
        BIT[i].full = 0;
        BIT[i].isn = 0;
    }
	return;
}

//
// 이 함수를 호출하기 전에 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 한다.
// 즉, 이 함수에서 메모리를 할당받으면 안된다.
//
void ftl_read(int lsn, char *sectorbuf) //파라미터 sectorbuf는 읽어온 sector data가 저장될 공간!
{
    /*
     * 1. lsn에 매핑되는 psn을 가져온다 (매핑 테이블에 없는경우, no data임을 리턴한다)
     * 2. psn에 저장된 데이터를 읽어와 버퍼에 저장한다.
     * 3. 버퍼에 저장된 내용 중 sector영역의 데이터만을 sectorbuf에 저장한다
     */
	return;
}


void ftl_write(int lsn, char *sectorbuf)
{

    /* <이 함수가 호출된 경우, dd_write()호출 전에 할 작업>
     * 
     * 1.빈 블록(free block)이 한 개 이상 존재하는가?
     * 2.한 개의 빈 블록(free block)을 제외하고, 빈 섹터가 존재하는가?
     * => 이 두가지 조건 만족시 : write() 수행.
     *                   아니면 : merge() 수행 후, write()수행
     */

    /* 1. lsn에 매핑되는 ppn값을 확인한다
     * 2-1. ppn이 -1인 경우 : lpn5에는 최초 쓰기 작업
     * 2-2. ppn이 -1 아닌경우 : 이미 존재하는 데이터를 update 해야함
     * 3. 비어있는 ppn 중 하나를 선택해 테이블을 갱신한다.
     * 4. 
     */

    //아래는 스도코드
    freenum = countFreeBlock();
    emptyPSN = searchEmptyPSN();
    if (emptyPSN exists && freenum >=1) {
        Search LSN from AMT;
        if (LST exists in AMT) {
            Mark the PSN mapped to LSN as invalid(*);
            Call Update_BIT(PSN);
        }
        Write Data to EmptyPSN;
        Call Update_AMT (LSN, EmptyPSN);
    }
    else { //빈 섹터 없는경우, garbage block을 merge한 후, write함
        merge_blocks();
        ftl_write(LSN, Data);
    }

	return;
}

void ftl_print()
{
    int i, j;
    printf("%3s %3s\n", "lpn", "ppn");
    for(i = 0; i < DATAPAGES_PER_DEVICE; i++) {
        printf("%3d %3d\n", i, AMT[i][1]);
    }
    printf("free block's pbn = %d\n", BLOCKS_PER_DEVICE);
	return;
}

//추가 구현할 함수들
void merge_blocks(){ //victim block 2개를 선정해 merge한다.
    
    FreePBN = SelectFreeBlock(); //full이 0인 block
    VictimPBN = select_garbage_block();
    for (each sector in VictimPBN) //victim블록의 섹터데이터가 valid한것만 free 블록에 copy
       if (data of PSN is valid) then
           CopyData (PSN, FreePBN)
               Call Update_SMT();
        
    dd_erase(VictimPBN);
    Call Update_BIT();
    return;
}

void select_garbage_block() {

    for (each PBN in Block Information Table)
        VictimPBN = SearchMaxISN //isn이 가장 큰 블록을 찾음
        
    return VictimPBN;
}

/*
int main(void) {
 
    char *fname = "flashMemory";
    char pagebuf[PAGE_SIZE];
    char sectorbuf[SECTOR_SIZE];
    char sparebuf[SPARE_SIZSE];
    int i, pageNum;
 
    //flash memory file 생성 및 초기화
    if((flashfp = fopen(fname, "w+")) == NULL) {
        fprintf(stderr, "fopen error for %s\n",fname);
        exit(1);
    }
    memset(pagebuf, (char)0xFF, PAGE_SIZE);
    pageNum = BLOCKS_PER_DEVICE * PAGES_PER_BLOCK;
    for (i = 0; i < pageNum; i++) {
        if (dd_write(i, pagebuf) == -1) {
            fprintf(stderr, "fwrite error\n");
            exit(1);
        }
    }
    //fclose(flashfp);
    
    ftl_open();
    sprintf(sectorbuf, "first sector data!");
    ftl_write(3, sectorbuf);
    sprintf(sectorbuf, "second sector data~");
    ftl_write(5, sectorbuf);
    sprintf(sectorbuf, "third sector data:)");
    ftl_write(7, sectorbuf);
    sprintf("fourth sector data on lpn3!!@@$%#");
    ftl_print(3, sectorbuf);
}
*/
