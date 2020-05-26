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

void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET);
	fread((void *)pagebuf, PAGE_SIZE, 1, fp);
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
	recordbuf[len] = '#'; //문자열로 저장되는데..  널문자 들어가면 안되나?? (안되게 함!!)
	
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
	memcpy(pagebuf, &head, sizeof(Head));
}

void insert(FILE *fp, const Person *p)
{
    char pagebuf[PAGE_SIZE], headbuf[PAGE_SIZE];
    char recordbuf[RECORD_SIZE], tmpbuf[RECORD_SIZE];
    char tmprecord[RECORD_SIZE], initial_record[RECORD_SIZE];
    int curpage; //레코드 저장할 페이지 번호
	int pagenum, recordnum, rmvpage, rmvrecord;
	int i, jump;
    //페이지가 있는(생성된)경우 : 헤더 정보 먼저 가져오기
	readPage(fp, headbuf, 0); //읽어온 헤드정보 저장
    Head head;
    memcpy(&head, headbuf, sizeof(Head));
    pagenum = head.pagenum;
    recordnum = head.recordnum;
    rmvpage = head.rmvpage; //삭제 페이지
    rmvrecord = head.rmvrecord; //삭제 레코드
	memset(pagebuf, (char)0xFF, PAGE_SIZE);
    if (rmvpage != -1) { //삭제된 레코드 있는 경우
		memset(pagebuf, (char)0xFF, PAGE_SIZE);
		memset(recordbuf, (char)0xFF, RECORD_SIZE);
		readPage(fp, pagebuf, rmvpage); //헤더에 저장된 가장 최근에 삭제된 페이지 가져옴
    	memcpy(recordbuf, pagebuf+(rmvrecord*RECORD_SIZE), RECORD_SIZE); //삭제 레코드 가져옴
		int pnum, rnum;
		char c;
		memcpy(&c, recordbuf, sizeof(char));
		memcpy(&pnum, recordbuf+1, sizeof(int));
		memcpy(&rnum, recordbuf+5, sizeof(int));

		if (c == '*') {
			memset(recordbuf, (char)0xFF, RECORD_SIZE);
			pack(recordbuf, p);
			memcpy(pagebuf+(rmvrecord*RECORD_SIZE), recordbuf, RECORD_SIZE); //내용 쓰기
			writePage(fp, pagebuf, rmvpage); //가장 최근에 삭제한 곳에 다시 데이터 저장
			if(pnum == -1 && rnum == -1) {//삭제 데이터가 이거 하나 뿐일때(마지막/유일)
				//여기에 데이터 써야함
				//헤더 페이지 -1 -1로 갱신 해 줘야함
				rmvpage = -1;
				rmvrecord = -1;
			}
			else { //두개 이상 삭제 된 레코드 있을 때
				//내가 가진 이 정보를 헤더페이지에 갱신 해 줘야함
				//여기 데이터 써야함
				rmvpage = pnum;
				rmvrecord = rnum;
			}
		}
		else {
			fprintf(stderr, "delete error! 헤더와 파일 정보 불일치\n");
			exit(1);
		}
		updateHeader(pagebuf, pagenum, recordnum, rmvpage, rmvrecord); //헤더 업데이트
		writePage(fp, pagebuf, 0); //변경된 헤더 적용
    }
    else { //삭제된 레코드 없어서 append 하는 경우
		int record_nums = pagenum * RECORDS_PER_PAGE - RECORDS_PER_PAGE;
        if (record_nums > recordnum) { //페이지 추가하지 않아도 되는경우
            curpage = pagenum-1; //다음 레코드 바로 저장 (pagenum-1)페이지에
            updateHeader(pagebuf, pagenum, ++recordnum, rmvpage, rmvrecord);
            writePage(fp, pagebuf, 0); //헤더 업데이트
        }
        else { //새 페이지 할당 후 레코드 저장해야 하는 경우
            writePage(fp, pagebuf, pagenum); //새 페이지 할당
            curpage = pagenum++; //저장할 페이지 저장후, pagenum++
            //헤더 업데이트
            memset(pagebuf, (char)0xFF, PAGE_SIZE);
            updateHeader(pagebuf, pagenum, ++recordnum, rmvpage, rmvrecord);
            writePage(fp, pagebuf, 0);
        }
		//curpage에서 가져올거임
		//이제 페이지 내에서 저장할 레코드 번호 찾아야함
		memset(initial_record, (char)0xFF, RECORD_SIZE);
		//pagebuf 내에 충분한 공간 있는지 확인 후 해당 위치에 recordbuf내용 저장해주기
		readPage(fp, pagebuf, curpage);
		for (i = 0; i < RECORDS_PER_PAGE; i++) {
			jump = i*RECORD_SIZE; 
			memcpy(recordbuf, pagebuf+jump, RECORD_SIZE); //tmprecord = pagebuf+jump해서 RECORD_SIZE 만큼 strncpy()
			if (!memcmp(recordbuf, initial_record, RECORD_SIZE)) { //tmprecord가 0xff만 있는 경우
				
				pack(recordbuf, p); //입력받은 person 구조체를  recordbuf에 저장
				memcpy(pagebuf+jump, recordbuf, RECORD_SIZE); //tmprecord주소부터 RECORD_SIZE만큼 pagebuf에 저장하기
				break;
			}
		}
		writePage(fp, pagebuf, curpage); //새로운 레코드 페이지에 저장
	}	
}

void delete(FILE *fp, const char *sn) 
{
	char pagebuf[PAGE_SIZE], recbuf[RECORD_SIZE], tmpbuf[PAGE_SIZE];
	Person person;
	int i, j, jump, find = false, isfirst = false;
	int vicpage, vicrec;
	int pagenum, recordnum, rmvpage, rmvrecord;

    //페이지가 있는(생성된)경우 : 헤더 정보 먼저 가져오기
	readPage(fp, tmpbuf, 0); //읽어온 헤드정보 저장
    Head head;
    memcpy(&head, tmpbuf, sizeof(Head));
    pagenum = head.pagenum;
    recordnum = head.recordnum;
    rmvpage = head.rmvpage;
    rmvrecord = head.rmvrecord;
	if (rmvpage == -1&& rmvrecord == -1)
		isfirst = true;

	for(i = 1; i < pagenum; i++) { //사용자가 입력한 주민번호 찾기
		readPage(fp, pagebuf, i); //페이지 읽어오기
		for(j=0; j<RECORDS_PER_PAGE; j++) {
			jump = j*RECORD_SIZE;
			memcpy(recbuf, pagebuf+jump, RECORD_SIZE); //레코드 읽어오기
			unpack(recbuf, &person);
			if (!strcmp(sn, person.sn)) { //삭제할 레코드 찾은경우
				rmvpage = i;
				rmvrecord = j;
				find = true;
				break;
			}
		}
		if (find)
			break;
	}
	if (find == false) {
		fprintf(stderr, "존재하지 않는 filed value 입니다.\n");
		exit(1);
	}

	if (isfirst) { //처음 삭제되는 경우
		//헤더에 내 정보 저장, 나한테 *-1-1해줌
		*(pagebuf+jump) = '*';
		int min = -1;
		memcpy(pagebuf+jump+1, &min, sizeof(int));
		memcpy(pagebuf+jump+5, &min, sizeof(int));
		writePage(fp, pagebuf, i);	
	}
	else { //이미 삭제된 레코드 있는 경우
		//이전 삭제된 레코드를 나한테 저장 -> 헤더에 내 정보 저장
		*(pagebuf+jump) = '*';
		memcpy(pagebuf+jump+1, &head.rmvpage, sizeof(int));
		memcpy(pagebuf+jump+5, &head.rmvrecord, sizeof(int));
		writePage(fp, pagebuf, i);
		
		//내 정보 헤더에 넣기 -> 헤더갱신
	}
	//헤더 갱신
	memset(tmpbuf, (char)0xFF, PAGE_SIZE);
	updateHeader(tmpbuf, pagenum, recordnum, rmvpage, rmvrecord);
	writePage(fp, tmpbuf,0); //헤더 페이지 갱신
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
	fclose(fp);
	return 0;
}

