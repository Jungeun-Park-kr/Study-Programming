#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "person.h"

#define RECORDS_PER_PAGE (int)(PAGE_SIZE/RECORD_SIZE)
#define true 1
#define false 0

char fname[RECORD_SIZE];

//헤더 페이지 구조체
struct Head{
	int pagenum;
	int recordnum;
	int rmvpage;
	int rmvrecord;
};
typedef struct Head Head;

//삭제에 사용할 구조체
typedef struct _node Node;
struct _node{
	int rmvpage;
	int rmvrecord;
	Node *next;
};

typedef struct _list {
	Node *head;
	Node *tail;
}List;

List list;

typedef struct _del {
	char deli;
	int rmvpage;
	int rmvrecord;
}Del;

void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET);
	fread((void *)pagebuf, PAGE_SIZE, 1, fp);
	//printf("readPage : %s\n", pagebuf);
}


// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 위치에 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
void writePage(FILE *fp, const char *pagebuf, int pagenum) {
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET);
	fwrite((void *)pagebuf, PAGE_SIZE, 1, fp);
}

void pack(char *recordbuf, const Person *p)
{
	int len;
	sprintf(recordbuf, "%s#%s#%s#%s#%s#%s", p->sn, p->name, p->age, p->addr, p->phone, p->email);
	len = strlen(recordbuf);
	recordbuf[len] = '#';
	//문자열로 저장되는데..  널문자 들어가면 안되나??
	
}

void unpack(const char *recordbuf, Person *p)
{
	char tmpbuf[RECORD_SIZE];
	strcpy(tmpbuf, recordbuf);
	char *pp;
	pp = strtok(tmpbuf, "#");
	memcpy(p->sn, pp, 14);
	pp = strtok(NULL, "#");
	memcpy(p->name, pp, 18);
	pp = strtok(NULL, "#");
	memcpy(p->age, pp, 4);
	pp = strtok(NULL, "#");
	memcpy(p->addr, pp, 22);
	pp = strtok(NULL, "#");
	memcpy(p->phone, pp, 16);
	pp = strtok(NULL, "#");
	memcpy(p->email, pp, 26);
}


void updateHeader(char *pagebuf, int pagenum, int recordnum, int rmvpage, int rmvrecord) {
	Head head;
	head.pagenum = pagenum;
	head.recordnum = recordnum;
	head.rmvpage = rmvpage;
	head.rmvrecord = rmvrecord;
	//printf("updateHeader함수) pagenum:%d, recordnum:%d, rmvpage:%d, rmvrecord:%d\n", head.pagenum, head.recordnum, head.rmvpage, head.rmvrecord);
	memcpy(pagebuf, &head, sizeof(Head));
	//writePage(fp, headbuf, 0); //헤더 페이지 갱신
}

void removeNode(List *lp, Del *del) {
	Node *delp;
	Node *nxtp;

	delp = lp->head->next;
	nxtp = delp->next;
	lp -> head -> next = nxtp;

	del->rmvpage = nxtp->rmvpage;
	del->rmvrecord = nxtp->rmvrecord;
}

void insert(FILE *fp, const Person *p)
{
    char pagebuf[PAGE_SIZE], headbuf[PAGE_SIZE];
    char recordbuf[PAGE_SIZE];
    int curpage; //레코드 저장할 페이지 번호
	int pagenum, recordnum, rmvpage, rmvrecord;
    //페이지가 있는(생성된)경우 : 헤더 정보 먼저 가져오기
	readPage(fp, headbuf, 0); //읽어온 헤드정보 저장
    Head head;
    memcpy(&head, headbuf, sizeof(Head));
    pagenum = head.pagenum;
    recordnum = head.recordnum;
    rmvpage = head.rmvrecord;
    rmvrecord = head.rmvrecord;
	printf("pagenum : %d, recordnum : %d, rmvpate : %d, rmvrecord : %d\n", pagenum, recordnum, rmvpage, rmvrecord); 
    memset(pagebuf, (char)0xFF, PAGE_SIZE);
    if (rmvpage != -1) { //삭제된 레코드 있는 경우
		curpage = rmvrecord;  //curpage = 가장 최근에 삭제된 레코드의 페이지 번호
		memset(pagebuf, (char)0xFF, PAGE_SIZE);
		writePage(fp, pagebuf, curpage); //새로운 데이터 쓰기 전 초기화
		Del del;
		removeNode(&list, &del);   //삭제 레코드 관리하는 linked list 수정하기
		rmvpage = del.rmvpage;
		rmvrecord = del.rmvrecord;
		updateHeader(pagebuf, pagenum, recordnum, rmvpage, rmvrecord); //헤더 업데이트
		writePage(fp, pagebuf, 0); //변경된 헤더 적용
    }
    else { //삭제된 레코드 없어서 append 하는 경우
        printf("삭제 레코드 x\n");
		int record_nums = pagenum * RECORDS_PER_PAGE - RECORDS_PER_PAGE;
        if (record_nums > recordnum) { //페이지 추가하지 않아도 되는경우
			printf("페이지 추가 안해도 됨\n");
            curpage = pagenum-1; //다음 레코드 바로 저장 (pagenum-1)페이지에
            updateHeader(pagebuf, pagenum, ++recordnum, rmvpage, rmvrecord);
            writePage(fp, pagebuf, 0); //헤더 업데이트
        }
        else { //새 페이지 할당 후 레코드 저장해야 하는 경우
			printf("페이지 새로 할당\n");
            writePage(fp, pagebuf, pagenum); //새 페이지 할당
            curpage = pagenum++; //저장할 페이지 저장후, pagenum++
            //헤더 업데이트
            memset(pagebuf, (char)0xFF, PAGE_SIZE);
            updateHeader(pagebuf, pagenum, ++recordnum, rmvpage, rmvrecord);
            writePage(fp, pagebuf, 0);
        }
    }
	printf("<<업데이트 후 헤더정보>>\n");
    //페이지가 있는(생성된)경우 : 헤더 정보 먼저 가져오기
	readPage(fp, pagebuf, 0); //읽어온 헤드정보 저장
	memcpy(&head, pagebuf, sizeof(Head));
	pagenum = head.pagenum;
    recordnum = head.recordnum;
    rmvpage = head.rmvrecord;
    rmvrecord = head.rmvrecord;
	printf("pagenum : %d, recordnum : %d, rmvpate : %d, rmvrecord : %d\n", pagenum, recordnum, rmvpage, rmvrecord); 
    
    //이제 페이지 내에서 저장할 레코드 번호 찾아야함
    char tmprecord[RECORD_SIZE], initial_record[RECORD_SIZE];
	memset(initial_record, (char)0xFF, RECORD_SIZE);
    int i, jump;
    //pagebuf 내에 충분한 공간 있는지 확인 후 해당 위치에 recordbuf내용 저장해주기
    readPage(fp, pagebuf, curpage);
	printf("curpage : %d, buf : %s\n", curpage,pagebuf);
    for (i = 0; i < RECORDS_PER_PAGE; i++) {
        jump = i*RECORD_SIZE; 
        memcpy(recordbuf, pagebuf+jump, RECORD_SIZE); //tmprecord = pagebuf+jump해서 RECORD_SIZE 만큼 strncpy()
        if (!memcmp(recordbuf, initial_record, RECORD_SIZE)) { //tmprecord가 0xff만 있는 경우
            
			printf("빈 레코드 칸 찾음 i : %d\n",i);
			pack(recordbuf, p); //입력받은 person 구조체를  recordbuf에 저장
            memcpy(pagebuf+jump, recordbuf, RECORD_SIZE); //tmprecord주소부터 RECORD_SIZE만큼 pagebuf에 저장하기
            break;
        }
    }
    //pack(recordbuf, p); //입력받은 person 구조체를 recordbuf에 저장
    writePage(fp, pagebuf, curpage); //새로운 레코드 페이지에 저장
}

int createList(List *lp) {
	if (lp == NULL) 
		return false;

	lp->head = (Node *)malloc(sizeof(Node));
	if (lp->head == NULL) {
		return false;
	}
	lp->tail = (Node *)malloc(sizeof(Node));
	if (lp->tail == NULL) {
		return false;
	}
	lp->head->next = lp->tail;
	lp->tail->next = lp->tail;
}

void addNode(List *lp, Del del) {
	Node *new;
	new = (Node *)malloc(sizeof(Node));
	
	//삭제 레코드 정보 저장
	new->rmvpage = del.rmvpage;
	new->rmvrecord = del.rmvrecord;
	//노드추가
	lp->head->next = new;
}

void delete(FILE *fp, const char *sn) 
{
	char pagebuf[PAGE_SIZE], recbuf[RECORD_SIZE], tmpbuf[PAGE_SIZE];
	Person person;
	int i, j, jump, find = false;
	int vicpage, vicrec;
	int pagenum, recordnum, rmvpage, rmvrecord;

    //페이지가 있는(생성된)경우 : 헤더 정보 먼저 가져오기
	readPage(fp, pagebuf, 0); //읽어온 헤드정보 저장
    Head head;
    memcpy(&head, pagebuf, sizeof(Head));
    pagenum = head.pagenum;
    recordnum = head.recordnum;
    rmvpage = head.rmvrecord;
    rmvrecord = head.rmvrecord;
	Del del;
	del.deli = '*';
	if (rmvpage == -1 && rmvrecord == -1) { //처음 삭제되는 경우
		createList(&list); //리스트 생성
	}
	for(i = 1; i < pagenum; i++) {
		readPage(fp, pagebuf, i); //페이지 읽어오기
		for(j=0; j<RECORDS_PER_PAGE; j++) {
			jump = j*RECORD_SIZE;
			memcpy(recbuf, pagebuf+jump, RECORD_SIZE); //레코드 읽어오기
			unpack(recbuf, &person);
			if (!strcmp(sn, person.sn)) { //삭제할 레코드 찾은경우
				//삭제할 레코드 앞에 메타 데이터 추가
				del.rmvpage = i; //삭제할 페이지 번호
				del.rmvrecord = j; //삭제할 레코드 번호
				
				printf("before :%s, len : %ld\n", pagebuf+jump, strlen(pagebuf+jump));
				*(pagebuf+jump) = '*';
				
				memset(pagebuf+jump+1, i, sizeof(int));

				memset(pagebuf+jump+5, j, sizeof(int));
				printf("pagebuf+jump : %s, len : %ld\n", pagebuf+jump,strlen(pagebuf+jump));
					
				printf("after :%s, len : %ld\n", pagebuf,strlen(pagebuf));
				writePage(fp, pagebuf, i);
				rmvpage =i;
				rmvrecord = j;
				find = true;
				break;
			}
		}
		if (find)
			break;
	}

	//<링크드 리스트 수정>
	addNode(&list, del); //1. 노드생성 후 추가
	//rmvpage = vicpage;	//헤더 데이터 갱신
	//rmvrecord = vicrec;
	memset(pagebuf, (char)0xFF, PAGE_SIZE);
	updateHeader(pagebuf, pagenum, recordnum, rmvpage, rmvrecord);
	writePage(fp, pagebuf,0); //헤더 페이지 갱신

	readPage(fp, pagebuf, rmvpage);
	printf("변경된data : %s\n",pagebuf);

}


int main(int argc, char *argv[])
{

	FILE *fp;  // 레코드 파일의 파일 포인터
	int i;
	int iOption = false;
	int dOption = false;
    long fsize;
	Person person;
	char fname[RECORD_SIZE];

	if (argc < 4) {
		fprintf(stderr, "usage : %s <OPTION> <record file name> <filed value(list)>\n", argv[0]);
		exit(1);
	}
	else {
		if (!strcmp(argv[1], "i"))
			iOption = true;
		else if (!strcmp(argv[1], "d"))
			dOption = true;
		else {
			fprintf(stderr, "usage : %s <OPTION> <record file name> <filed value(list)>\n", argv[0]);
			exit(1);
		}
	}
	//이름저장
	strcpy(fname, argv[2]);
	if(iOption) { //삽입
		if (argc != 9) {
			fprintf(stderr, "usage : %s i <record file name> <filed value list>\n", argv[0]);
			exit(1);
		}
		if ((fp = fopen(argv[2], "a+")) == NULL) {
			fprintf(stderr, "fopen error for %s\n", argv[2]);
			exit(1);
		}

        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
		printf("fsize : %ld\n", fsize);

        fclose(fp);
		//입력받은 필드 데이터를 Person 구조체에 저장
		strcpy(person.sn, argv[3]);
		strcpy(person.name, argv[4]);
		strcpy(person.age, argv[5]);
		strcpy(person.addr, argv[6]);
		strcpy(person.phone, argv[7]);
		strcpy(person.email, argv[8]);
		
        ///사전작업
        char pagebuf[PAGE_SIZE];
        if (fsize == 0) { //첫 생성인 경우 -> 헤더 먼저 생성하기
			memset(pagebuf, (char)0xFF, PAGE_SIZE);
            fp = fopen(fname, "a");
            updateHeader(pagebuf, 2, 0, -1, -1); //첫 헤더 생성
            writePage(fp, pagebuf, 0); //헤더 페이지 작성
            memset(pagebuf, (char)0xFF, PAGE_SIZE);
            writePage(fp, pagebuf, 1); //다음 페이지 설정
            fclose(fp);
        }
        if ((fp = fopen(fname, "r+")) == NULL) {
			fprintf(stderr, "fopen error for %s\n", fname);
			exit(1);
		}
        insert(fp, &person);
		printf("insert 성공\n");
		fclose(fp);
	}
	else if(dOption) { //삭제
		if (argc != 4) {
			fprintf(stderr, "usage : %s d <record file name> <filed value>\n", argv[0]);
			exit(1);
		}
		if ((fp = fopen(argv[2], "r+")) == NULL) {
			fprintf(stderr, "fopen error for %s\n", argv[2]);
			exit(1);
		}
		char sn[14];
		strcpy(sn, argv[3]); //입력받은 주민번호 저장
		delete(fp, sn);

	}

	fp =fopen(fname, "r");
	printf("<<정보>>\n");
	int pagenum, recordnum, rmvpage, rmvrecord;
	int j, jump;
	char recbuf[RECORD_SIZE], pagebuf[PAGE_SIZE];
	//페이지가 있는(생성된)경우 : 헤더 정보 먼저 가져오기
	readPage(fp, pagebuf, 0); //읽어온 헤드정보 저장
	Head head;
	memcpy(&head, pagebuf, sizeof(Head));
	pagenum = head.pagenum;
	recordnum = head.recordnum;
	rmvpage = head.rmvrecord;
	rmvrecord = head.rmvrecord;
	printf("pagenum : %d, recordnum : %d, rmvpage : %d, rmvrecord : %d\n", pagenum, recordnum, rmvpage, rmvrecord); 
	
	for(i = 1; i < pagenum; i++) {
		readPage(fp, pagebuf, i); //페이지 읽어오기
		for(j=0; j<RECORDS_PER_PAGE; j++) {
			jump = j*RECORD_SIZE;
			memcpy(recbuf, pagebuf+jump, RECORD_SIZE); //레코드 읽어오기
			printf("page(%d)record(%d) : %s\n",i,j,recbuf);
		}
	}
	fclose(fp);

	return 0;
}

