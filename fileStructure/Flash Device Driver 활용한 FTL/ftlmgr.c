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

//
// flash memory를 처음 사용할 때 필요한 초기화 작업, 예를 들면 address mapping table에 대한
// 초기화 등의 작업을 수행한다. 따라서, 첫 번째 ftl_write() 또는 ftl_read()가 호출되기 전에
// file system에 의해 반드시 먼저 호출이 되어야 한다.
//

int AMT[DATAPAGES_PER_DEVICE][2]; //(free block제외) 실  Data 페이지 개수의 행, 2개의 열을 가진 Address Mapping Table을 선언
//struct SpareData[DATAPAGES_PER_DEVICE]; //spare부분을 처리하기 위한 구조체 배열을 free 제외한 실제 사용 페이지 개수만큼 선언

void ftl_open()
{
	//
	// address mapping table 초기화
	// free block's pbn 초기화
    // address mapping table에서 lbn 수는 DATABLKS_PER_DEVICE 동일
	int i;
    for (i = 0; i < DATAPAGES_PER_DEVICE; i++) {
        //SpareData[i].lsn = i; //lsn
        //SpareData[i].is_invalid = TRUE;
        AMT[i][0] = i;
        AMT[i][1] = -1;
    }
	return;
}

//
// 이 함수를 호출하기 전에 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 한다.
// 즉, 이 함수에서 메모리를 할당받으면 안된다.
//
void ftl_read(int lsn, char *sectorbuf)
{

	return;
}


void ftl_write(int lsn, char *sectorbuf)
{

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
