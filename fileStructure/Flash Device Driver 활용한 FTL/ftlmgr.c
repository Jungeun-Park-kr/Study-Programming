#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "flash.h"

FILE *flashfp;	// fdevicedriver.c에서 사용

#define true 1
#define false 0

int cOption = false;
int wOption = false;
int rOption = false;
int eOption = false;

int dd_read(int ppn, char *pagebuf);
int dd_write(int ppn, char *pagebuf);
int dd_erase(int pbn);

int isEmpty(char *pagebuf); //페이지에 의미있는 데이터가 있는지 확인하는 함수
int isEmptyBlock(int pbn); //블록에 의미있는 데이터가 있는지 확인하는 함수

int main(int argc, char *argv[])
{
	char sectorbuf[SECTOR_SIZE];
	char pagebuf[PAGE_SIZE];
	char *blockbuf;
	char initialbuf[SECTOR_SIZE];
	char tmp[PAGE_SIZE];
	int pages, ppn, pbn;
	int i, j;
	memset(initialbuf, (char)0xFF, SECTOR_SIZE); //0xFF로만 이루어진 initial buf 배열 초기화

	//프로그램 실행에 사용된 옵션 확인
	if (argc < 3) { //실행시 인자 부족한 경우 오류메시지 출력 후 종료
		fprintf(stderr, "usage : %s <OPTION> <OPTION_COMMAND>\n", argv[0]);
		exit(1);
	}
	else {
		if (!strcmp(argv[1], "c")) //<flashfile>생성 옵션
			cOption = true;
		else if (!strcmp(argv[1], "w")) //페이지 쓰기 옵션
			wOption = true;
		else if (!strcmp(argv[1], "r")) //페이지 읽기 옵션
			rOption = true;
		else if (!strcmp(argv[1], "e")) //블록 삭제 옵션
			eOption = true;
		else { //지정된 옵션 외의 다른 옵션 사용시 에러메시지 출력 후 종료
			fprintf(stderr, "usage : %s <OPTION> <OPTION_COMMAND>\n", argv[0]);
			exit(1);
		}
	}

	//flash memory emulator
	if (cOption) { //c옵션으로 실행시 <flashfile> 파일 생성
		if (argc != 4) {
			fprintf(stderr, "usage : %s c <flashfile> <#blocks>\n",argv[0]);
			exit(1);
		}
		if (atoi(argv[3]) < 16) { //블락 수가 너무 작을경우 오류메시지 출력 후 종료
			fprintf(stderr, "block number is too small!\n");
			exit(1);
		}

		memset(pagebuf, (char)0xFF, PAGE_SIZE); //최초 파일 생성시 pagebuf버퍼를 0xFF로 채움
		pages = atoi(argv[3]) * PAGE_NUM; //총 page 개수 = block 개수 * block당 page수(4)

		if ((flashfp = fopen(argv[2], "w+")) == NULL) { //파일 생성
			fprintf(stderr, "fopen error for %s\n", argv[2]);
			exit(1);
		}
		for (i = 0; i < pages; i++) { //0xFF로 채워진 pagebuf의 내용을 써서 파일의 모든 내용 초기화
			if (dd_write(i, pagebuf) == -1) { //쓰기 실패시 에러메시지 출력 후 종료
				fprintf(stderr, "fwrite error\n");
				exit(1);
			}
		}
		fclose(flashfp);
		return 0;
	}


	// 페이지 쓰기: pagebuf의 섹터와 스페어에 각각 입력된 데이터를 정확히 저장하고 난 후 해당 인터페이스를 호출한다
	else if (wOption) {
		if (argc != 6) {
			fprintf(stderr, "usage : %s w <flashfile> <ppn> Msectordata> <sparedata>\n", argv[0]);
			exit(1);
		}
		if ((flashfp = fopen(argv[2], "r+")) == NULL) {
			fprintf(stderr, "open error for %s\n", argv[2]);
			exit(1);
		}
        ppn = atoi(argv[3]);
        /* 구현해야 할 것 : out-of-place update
         * 입력받은 ppn의 내용을 읽어와 pagebuf에 저장 후 isEmpty()로 데이터가 있는지 확인 ->
         * isEmpty() - True : 입력받은 내용 바로 쓰기
         * isEmpty() - False : 해당 ppg에서 제일 가까운 순서대로 비어있는 블록을 찾음
         *                      -> 비어있는 블록에 ppn이 속한 블록(ppn제외) 복사
         *                      -> erase()로 원래 블록 내용 삭제
         *                      -> 복사해둔 데이터 다시 옮기기
         *                      -> 입력받은 ppn에 입력받은 데이터 쓰기
         *                      -> erase()복사해뒀던 데이터 블록의 내용 삭제*/
        
        //pagebuf에 입력받은 데이터 저장
		strcpy(sectorbuf, argv[4]); //입력받은 sectordata 복사
		strncat(sectorbuf, initialbuf, (SECTOR_SIZE - strlen(sectorbuf))); //공간이 남은경우 0xFF로 채움 
		strcpy(pagebuf, sectorbuf); //pagebuf에 sectordata 붙이기
		strcat(pagebuf + SECTOR_SIZE, argv[5]); //pagebuf에 입력받은 sparedata붙이기
		strncat(pagebuf, initialbuf, (PAGE_SIZE - SECTOR_SIZE + strlen(argv[5]))); //공간 남은경우 0xFF로 채움

        memset(tmp, 0, PAGE_SIZE);
        dd_read(ppn, tmp);

        if (isEmpty(tmp)) { //페이지에 내용이 없는경우
		    if (dd_write(ppn, pagebuf) == -1) { //페이지에 해당내용 쓰기
			    fprintf(stderr, "write error\n");
			    exit(1);
		    }
		    fclose(flashfp);
		    return 0;
        }

        else { //기존 데이터가 존재하는 경우 - out-of-place update
            pbn = ppn/4 + 1; //현재블락 다음부터 비어있는 블록을 찾음 (데이터 임시 복사를 위해)
            while(1){
                if(isEmptyBlock(pbn)) //비어있는 블록 찾음
                    break;
                else
                    pbn += 1;
            }
            //pbn : 비어있는 블록, ppn : 내가 데이터 쓸 페이지
            int curpbn = ppn/4; //데이터를 쓸 블록
            int curppn = curpbn*4; //데이터를 쓸 블록의 첫 페이지
            int tmppn = pbn*4; //데이터를 복사해둘 페이지(해당 블록의 첫 페이지)
            //pbn에 데이터 옮기기
            for (i = curppn,j = tmppn; i < curppn+4; i++,j++) {
                if (i==ppn) //내가 데이터를 써야할 페이지데이터는 복사하면 안됨
                    continue;
                else {
                    memset(tmp, 0, PAGE_SIZE);
                    dd_read(i, tmp);
                    if(!isEmpty(tmp)) //데이터가 있는 페이지만 복사
                        dd_write(j,tmp); //내용복사
                }
            }
           //erase()로 원래 블록의 데이터 삭제하기
            dd_erase(curpbn);
            //복사해둔 데이터 옮기기
            for (i = curppn,j=tmppn ; i < curppn+4; i++,j++) {
                if (i==ppn)
                    continue;
                else {
                    memset(tmp, 0, PAGE_SIZE);
                    dd_read(j, tmp);
                    if(isEmpty(tmp))
                        dd_write(i, tmp);
                }
            }
            //입력받은 내용 write
		    if (dd_write(ppn, pagebuf) == -1) { //페이지에 해당내용 쓰기
			    fprintf(stderr, "write error\n");
			    exit(1);
		    }
            //카피했던 데이터 블록의 내용 삭제
            dd_erase(pbn);

		    fclose(flashfp);
		    return 0;
        }
	}

	//페이지 읽기: pagebuf를 인자로 사용하여 해당 인터페이스를 호출하여 페이지를 읽어 온 후 여기서 섹터 데이터와스페어 데이터를 분리해 낸다
	else if (rOption) {
		ppn = atoi(argv[3]);
		if (argc != 4) { //옵션의 가변인자 부족시 에러메시지 출력 후 종료
			fprintf(stderr, "usage : %s r <flashfile> <ppn>\n", argv[0]);
			exit(1);
		}
		if ((flashfp = fopen(argv[2], "r")) == NULL) { //파일 오픈 실패시 에러메시지 출력 후 종료
				fprintf(stderr, "fopen error for %s\n", argv[2]);
				exit(1);
		}
		memset(tmp, 0, PAGE_SIZE);
		memset(pagebuf, 0, PAGE_SIZE);
		dd_read(ppn, pagebuf); //dd_read()로 읽어온 데이터 pagebuf에 저장

		if (isEmpty(pagebuf)) { //의미있는 데이터가 없는경우 출력하지 않고 바로 종료
			return 0;
		}
		//sector data와 spare data 분리해 tmp배열에 출력할 내용 저장
		j = 0;
		for (i = 0; i < SECTOR_SIZE; i++) {
			if(isprint(pagebuf[i]))
				tmp[j++] = pagebuf[i];
			else
				break;
		}
		tmp[j++] = ' ';
		for (i = SECTOR_SIZE; i < SECTOR_SIZE + SPARE_SIZE; i++) {
			if(isprint(pagebuf[i]))
				tmp[j++] = pagebuf[i];
			else
				break;
		}

		tmp[j] = '\0'; //sector data와 spare data 저장 후, 의미있는 데이터만 출력되도록 널문자 추가
		printf("%s\n", tmp); //데이터 출력
		
		fclose(flashfp);
		return 0;
	}

	//블록 삭제 - 블록 단위로 삭제함에 주의!
	else if (eOption) {
		if (argc != 4) { //옵션 인자개수 부족시 에러메시지 출력 후 종료
			fprintf(stderr, "usage : %s e <flashfile> <pbn>\n", argv[0]);
			exit(1);
		}
		
		if ((flashfp = fopen(argv[2], "r+")) == NULL) { //블록 삭제할 파일 열기. 실패시 에러메시지 출력 후 종료
			fprintf(stderr, "open error for %s\n", argv[2]);
			exit(1);
		}

		if (dd_erase(atoi(argv[3])) == -1) { //블록삭제 실패시 에러메시지 출력 후 종료
			fprintf(stderr, "erase error\n");
			exit(1);
		}
		else
			return 0;
	}

	return 0;
}

int isEmpty(char *pagebuf) { //pagebuf 문자열에 의미있는 데이터가 있는지 확인하는 함수
	//반환값 : 의미있는 데이터가 없는경우 true 리턴, 의미있는 데이터가 있는경우 false리턴
	int i;
	for (i = 0; i < PAGE_SIZE; i++) {
		if (isprint(pagebuf[i]))
			return false;
	}
	return true;
}

int isEmptyBlock(int pbn) {
    //파라미터로 받은 bpn에 있는 모든 page가 비어있는지 확인
    //반환값 : 비어있는 경우 -> true, 데이터가 있는경우 - False 리턴
    int ppn = pbn * 4;
    char tmpbuf[PAGE_SIZE];
    for(int i = ppn; i < ppn+4; i++) {
        memset(tmpbuf, 0, PAGE_SIZE);
        dd_read(i, tmpbuf);
        if(!isEmpty(tmpbuf))
            return false;
    }
    return true;
}
