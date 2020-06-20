#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "person.h"

#define RECORDS_PER_PAGE (int)(PAGE_SIZE/RECORD_SIZE)
#define true 1
#define false 0

//헤더 페이지 구조체
struct Head{
	int pagenum;
	int recordnum;
	int rmvpage;
	int rmvrecord;
};
typedef struct Head Head;

int recordmax; 

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓸 때나
// 모든 I/O는 위의 두 함수를 먼저 호출해야 합니다. 즉, 페이지 단위로 읽거나 써야 합니다.
//페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET);
	fread((void *)pagebuf, PAGE_SIZE, 1, fp);
}

// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 위치에 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET);
	fwrite((void *)pagebuf, PAGE_SIZE, 1, fp);
}

// 주어진 레코드 파일에서 레코드를 읽어 heap을 만들어 나간다. Heap은 배열을 이용하여 저장되며, 
// heap의 생성은 Chap9에서 제시한 알고리즘을 따른다. 레코드를 읽을 때 페이지 단위를 사용한다는 것에 주의해야 한다.
void buildHeap(FILE *inputfp, char **heaparray)
{
	int pagenum, recordnum; //, rmvpage, rmvrecord;
	int i, j, jump;
	char tmpbuf[PAGE_SIZE], pagebuf[PAGE_SIZE], recbuf[RECORD_SIZE];
	Head head;

	readPage(inputfp, tmpbuf, 0); //읽어온 헤드정보 저장
    memcpy(&head, tmpbuf, sizeof(Head));
    pagenum = head.pagenum; //전체 페이지 수 (헤더포함)
    recordnum = head.recordnum;  //전체 레코드 수
	int idx = 0; //레코드 총 개수
	//페이지단위로 읽어와서 저장
	for(i = 1; i < pagenum; i++) {
		readPage(inputfp, pagebuf, i); //페이지 읽어오기
		for(j = 0; j<RECORDS_PER_PAGE; j++) { //페이지 내의 레코드 하나씩 힙에 넣어주기(힙 구축)
			jump = j*RECORD_SIZE;
			memcpy(recbuf, pagebuf+jump, RECORD_SIZE); //레코드 읽어오기
			strcpy(heaparray[idx], recbuf); //레코드 하나씩 힙(heap array)에 추가하기
			//현재 인덱스 번호 개수 까지 힙 정렬해주기
			//in here
			
			idx++;
		}
	}
}

// 완성한 heap을 이용하여 주민번호를 기준으로 오름차순으로 레코드를 정렬하여 새로운 레코드 파일에 저장한다.
// Heap을 이용한 정렬은 Chap9에서 제시한 알고리즘을 이용한다.
// 레코드를 순서대로 저장할 때도 페이지 단위를 사용한다.
void makeSortedFile(FILE *outputfp, char **heaparray)
{	
	int i=0,j, jump;
	char pagebuf[PAGE_SIZE], recordbuf[RECORD_SIZE];
	memset(pagebuf, 0, PAGE_SIZE);
	for(i = 1; i < heaparray[i] != NULL; i++) { //모든 레코드 다 쓸때까지 페이지 단위 반복
		//종료조건 확인필요*******heaparray[i] != NULL ??????**************
		for (j = 0; j < RECORDS_PER_PAGE; j++) { //i번째 페이지에 레코드 꽉채우기
			jump = j*RECORD_SIZE;
			memcpy(pagebuf+jump, heaparray[i], RECORD_SIZE); //i번째 정렬 레코드 복사
		}
		//i번째 heap array 내용을 페이지 단위로 쓰기
		writePage(outputfp, pagebuf, i);
	}
}

int main(int argc, char *argv[])
{
	FILE *inputfp;	// 입력 레코드 파일의 파일 포인터
	FILE *outputfp; // 정렬된 레코드 파일의 파일 포인터
	int i, j, jump;

	char pagebuf[PAGE_SIZE], recbuf[RECORD_SIZE], tmpbuf[PAGE_SIZE];
	char **heaparray;

	if (argc != 4) {
		fprintf(stderr, "usage : %s s <input record file name. <output record file name>\n", argv[0]);
		exit(1);
	}

	if ((inputfp = fopen(argv[2], "r")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", argv[2]);
		exit(1);
	}

	int pagenum, recordnum; //, rmvpage, rmvrecord;
	int i, j;
	readPage(inputfp, tmpbuf, 0); //읽어온 헤드정보 저장
    Head head;
    memcpy(&head, tmpbuf, sizeof(Head));
    pagenum = head.pagenum; //전체 페이지 수 (헤더포함)
    recordnum = head.recordnum;  //전체 레코드 수
	recordmax = recordnum; //레코드 개수 저장

	//heap구축할 배열 메모리 할당
	heaparray = calloc(recordnum, sizeof(char *));
	for(i=0; i<recordnum; i++)
		heaparray[i] = calloc(RECORD_SIZE, sizeof(char));

	//heap 구축
	buildHeap(inputfp,heaparray);
	fclose(inputfp); //구축 완료 후 입력파일 닫기
	//heap 구축 완료 & 출력 파일 오픈
	if ((outputfp = fopen(argv[3], "w")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", argv[3]);
		exit(1);
	}
	readPage(inputfp, tmpbuf, 0); //읽어온 헤드정보 저장
	writePage(outputfp, tmpbuf, 0); //헤더페이지는 그대로 복사해주기 (삭제레코드없으니까)
	makeSortedFile(outputfp, heaparray); //나머지 heaparray의 내용도 파일에 작성
	fclose(outputfp); //작성 후 출력파일 닫기

	return 0;
}