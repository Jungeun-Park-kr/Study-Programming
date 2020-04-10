#include <stdio.h>
#include <string.h>
#include "singlyLinkedlist.h"

int menu(const char **mList, size_t menuCnt);
void mInput(List *lp);		/* 입력메뉴 처리 함수 */
void mOutput(List *lp);		/* 출력메뉴 처리 함수 */
void mSearch(List *lp);		/* 검색메뉴 처리 함수 */
void mDelete(List *lp);		/* 삭제메뉴 처리 함수 */
void mSort(List *lp);		/* 정렬메뉴 처리 함수 */
void myflush();				/* 입력 버퍼 flush 함수 */
/*----------------------------------------------------------------------------------
  Function name : main
----------------------------------------------------------------------------------*/
int main()
{
	const char *menuList[] = { "입력하기","출력하기","검색하기","삭제하기", "정렬하기", "종  료" };
	int menuNum;	/* 메뉴번호 저장 변수 */
	int menuCnt;	/* 메뉴개수 저장 변수 */
	List list;		/* 리스트관리 구조체 변수 */
	int bres;

	menuCnt = sizeof(menuList) / sizeof(menuList[0]);

	bres = createList(&list);		/* 비어있는 리스트 초기화 */
	if (bres == TRUE){
		printf("@ list 생성 성공!\n");
	}
	else{
		printf("@ list 생성 실패!\n");
		return 0;
	}
	while (1)
	{
		menuNum = menu(menuList, menuCnt);	/* 메뉴화면을 띄우고 메뉴번호를 입력 받음 */
		if (menuNum == menuCnt) { break; }
		switch (menuNum)
		{
		case 1: mInput(&list); break;		/* 입력메뉴 실행 */
		case 2: mOutput(&list); break;		/* 출력메뉴 실행 */
		case 3: mSearch(&list); break;		/* 검색메뉴 실행 */
		case 4: mDelete(&list); break;		/* 삭제메뉴 실행 */
		case 5: mSort(&list); break;		/* 정렬메뉴 실행 */
		}
	}
	printf("list내의 데이터 노드의 개수 : %d\n", list.size);

	destroyList(&list);	/* 리스트내의 모든 데이터 삭제 */

	return 0;
}
/*----------------------------------------------------------------------------------
Function name	: menu
Parameters		: mList - 메뉴 목록 배열
				  menuCnt - 메뉴 개수
Returns			: 사용자 선택한 메뉴번호
----------------------------------------------------------------------------------*/
int menu(const char **mList, size_t menuCnt)
{
	size_t menuNum = 0;	/* 존재하지 않는 메뉴 번호 저장 */
	size_t i;

	printf("\n\n");
	for (i = 0; i < menuCnt; i++) {	/* 메뉴 출력 */
		printf("%d. %s\n", i + 1, mList[i]);
	}

	while (menuNum<1 || menuNum>menuCnt) {	/* 메뉴번호가 유효하지 않을 동안 반복 */
		printf("# 메뉴 선택 : ");
		scanf("%d", &menuNum);	/* 메뉴 번호 입력 */
	}
	return menuNum;
}
/*----------------------------------------------------------------------------------
Function name	: mInput - 입력 메뉴 처리 함수
Parameters		: lp - 리스트 관리 구조체의 주소
Returns			: 없음
----------------------------------------------------------------------------------*/
void mInput(List *lp)
{
	int inData;
	int res;  /* scanf()함수의 리턴 값 저장 */
	int bres;

	printf("\n[ 입력하기 메뉴 ]\n\n");

	while (1) {
		printf("# 정수를 입력하세요(문자 입력시 종료) : ");
		res = scanf("%d", &inData);	/* scanf()함수의 리턴 값 : 정수 입력 시 1, 문자 입력 시 0리턴 됨*/
		if (res == 0) {	/* 문자 입력 시 종료 */
			myflush();
			break;
		}
		//bres = addFirst(lp, inData);	/* tail 노드 앞에 데이터 추가  */
		bres = addLast(lp, inData);
		if (bres == TRUE)
			printf("@ 데이터 추가 성공!\n");
		else
			printf("@ 데이터 추가 실패!\n");
	}
	return;
}
/*----------------------------------------------------------------------------------
Function name	: mOutput - 출력 메뉴 처리 함수
Parameters		: lp - 리스트 관리 구조체의 주소
Returns			: 없음
----------------------------------------------------------------------------------*/
void mOutput(List *lp)
{
	displayList(lp);
}
/*----------------------------------------------------------------------------------
Function name	: mSearch - 검색 메뉴 처리 함수
Parameters		: lp - 리스트 관리 구조체의 주소
Returns			: 없음
----------------------------------------------------------------------------------*/
void mSearch(List * lp)
{
	int sData;
	Node *resp;		/* 검색된 노드의 시작주소 저장 */
	int res;		/* scanf()함수의 리턴 값 저장 */

	printf("\n[ 검색하기 메뉴 ]\n\n");
	while (1) {
		printf("# 찾을 데이터를 입력하세요(문자 입력 시 종료) : ");
		res = scanf("%d", &sData);	/* scanf()함수의 리턴 값 : 정수 입력 시 1, 문자 입력 시 0리턴 됨*/
		if (res == 0) {		/* 문자 입력 시 종료 */
			myflush();
			break;
		}
		else {
			;
		}
		resp = searchNode(lp, sData);
		if (resp != NULL) {	/* 데이터를 찾았으면 */
			printf("@ 검색 데이터 존재!\n");
		}
		else {				/* 데이터를 못찾았으면 */
			printf("@ 검색 데이터 존재하지 않음!\n");
		}
	}
	return;
}
/*----------------------------------------------------------------------------------
Function name	: mDelete - 삭제 메뉴 처리 함수
Parameters		: lp - 리스트 관리 구조체의 주소
Returns			: 없음
----------------------------------------------------------------------------------*/
void mDelete(List * lp)
{
	int delData;
	int res;		/* scanf()함수의 리턴 값 저장 */
	int bres;

	printf("\n[ 삭제하기 메뉴 ]\n\n");
	while (1) {
		printf("# 삭제할 데이터를 입력하세요(문자 입력 시 종료) : ");
		res = scanf("%d", &delData);	/* scanf()함수의 리턴 값 : 정수 입력 시 1, 문자 입력 시 0리턴 됨*/
		if (res == 0) {		/* 문자 입력 시 종료 */
			myflush();
			break;
		}
		else {
			;
		}
		bres = removeNode(lp, delData);
		if (bres == TRUE)
			printf("@ 삭제 성공!\n");
		else
			printf("@ 삭제 실패!\n");
	}
	return;
}
/*----------------------------------------------------------------------------------
Function name	: mSort - 정렬 메뉴 처리 함수
Parameters		: lp - 리스트 관리 구조체의 주소
Returns			: 없음
----------------------------------------------------------------------------------*/
void mSort(List *lp)
{
	sortList(lp);
}
/*----------------------------------------------------------------------------------
Function name	: myflush - 입력 버퍼 내의 모든 데이터 지우는 함수
Parameters		: 없음
Returns			: 없음
----------------------------------------------------------------------------------*/
void myflush()
{
	while (getchar() != '\n') {
		;
	}
}
