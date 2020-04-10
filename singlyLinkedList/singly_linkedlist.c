#include "singlyLinkedlist.h"
#include <stdio.h>  // printf(), scanf()
#include <stdlib.h>  // malloc(), free()

/*----------------------------------------------------------------------------------
Function name	: createList - 연결 리스트 생성 및 초기화
Parameters		: lp - 리스트 관리 구조체의 주소
Returns			: 성공 - TRUE / 실패 - FALSE
----------------------------------------------------------------------------------*/
int createList(List *lp)
{
	if (lp == NULL) { /* lp포인터 NULL check */
		return FALSE;
	}

	lp->head = (Node *)malloc(sizeof(Node));
	if (lp->head == NULL) {
		return FALSE;
	}
	lp->tail = (Node*)malloc(sizeof(Node));
	if (lp->tail == NULL) {
		return FALSE;
	}

	lp->head->next = lp->tail;
	lp->tail->next = lp->tail;
	lp->size = 0;

	return TRUE; 
}

/*----------------------------------------------------------------------------------
Function name	: addFirst - head node 뒤에 node 추가(역순 저장)
Parameters		: lp - 리스트 관리 구조체의 주소
				  data - 추가할 데이터
Returns			: 성공 - TRUE / 실패 - FALSE
----------------------------------------------------------------------------------*/
int addFirst(List *lp, int data)
{
	Node *newp;
	if (lp == NULL) { /* lp포인터 NULL check */
		return FALSE;
	}

	newp = (Node*)malloc(sizeof(Node));
	if (newp == NULL) {
		return FALSE;
	}
	newp->data = data;
	newp->next = lp->head->next;
	lp->head->next = newp;
	lp->size++;

	return TRUE; // return 값은 수정해주세요.
}
/*----------------------------------------------------------------------------------
Function name	: addLast - tail node 앞에 새 node 추가(정순 저장)
Parameters		: lp - 리스트 관리 구조체의 주소
				  data - 추가할 데이터
Returns			: 성공 - TRUE / 실패 - FALSE
----------------------------------------------------------------------------------*/
int addLast(List *lp, int data)
{
	Node *newp;	/* 새 노드 주소저장용 포인터 */
	Node *btp;	/* tail node 바로 앞 노드를 가리키는 포인터 */
	Node *curp;

	if (lp == NULL) { /* lp포인터 NULL check */
		return FALSE;
	}
	newp = (Node *)malloc(sizeof(Node));
	if (newp == NULL) {
		return FALSE;
	}
	newp->data = data;
	//printf("입력한 data : %d\t", newp->data);

	if (lp->size == 0) {
		lp->head->next = newp;
		newp->next = lp->tail;
		lp->size++;
		return TRUE;
	}
	
	curp = lp->head->next;
	btp = lp->head;
	while (curp != lp->tail) {
		curp = curp->next;
		btp = btp->next;
	}
	btp->next = newp;

	newp->next = lp->tail;
	//printf("newp : %d \tlp->tail : %d\t ",newp,lp->tail);
	lp->size = lp->size++;
	//printf("추가한 후 size : %d\n", lp->size);
	return TRUE; 
}

/*----------------------------------------------------------------------------------
Function name	: displayList - 리스트 내의 모든 데이터 출력
Parameters		: lp - 리스트 관리 구조체의 주소
Returns			: 없음
----------------------------------------------------------------------------------*/
void displayList(List *lp)
{
	Node *curp;

	if (lp == NULL) { /* lp포인터 NULL check */
		return;
	}
	printf("displayList()\tsize:%d\n",lp->size);
	curp = lp->head->next;
	while (curp != lp->tail) {
		printf("%5d", curp->data);
		curp = curp->next;
		//printf("\nsize : %d \n", lp->size);
	}

	return;
}

/*----------------------------------------------------------------------------------
Function name	: searchNode - data와 일치하는 첫 번째 node 검색
Parameters		: lp - 리스트 관리 구조체의 주소
				  data - 검색할 데이터
Returns			: 성공 - 검색된 노드의 주소 / 실패 - NULL pointer
----------------------------------------------------------------------------------*/
Node * searchNode(List *lp, int data)
{
	Node *curp;
	
	if (lp == NULL) { /* lp포인터 NULL check */
		return NULL;
	}

	curp = lp->head->next;
	while (curp != lp->tail) {
		if (curp->data == data) {
			return curp;
		}
		curp = curp->next;
	}

	return NULL;  
}
/*----------------------------------------------------------------------------------
Function name	: removeNode - data와 일치하는 첫 번째 노드 삭제
Parameters		: lp - 리스트 관리 구조체의 주소
data - 삭제할 데이터
Returns			: 성공 - TRUE / 실패 - FALSE
----------------------------------------------------------------------------------*/
int removeNode(List *lp, int data)
{
	Node *delp;
	Node *curp;

	if (lp == NULL) { /* lp포인터 NULL check */
		return FALSE;
	}

	delp = lp->head->next;
	curp = lp->head;
	//printf("첫 번째 노드 : %d\n", lp->head->data);
	//printf("size : %d\n", lp->size);
	if (delp->data == data) {
		lp->head->next = delp->next;
		lp->size = lp->size - 1;
		//printf("첫 번째 노드 : %d\n", lp->head->data);
		//printf("size : %d\n", lp->size);
		return TRUE;
	}

	while (delp != lp->tail) {
		if (delp->data == data){
			curp->next = delp->next;
			lp->size = lp->size - 1;
			return TRUE;
		}
		delp = delp->next;
		curp = curp->next;
	}

	return FALSE;
}
/*----------------------------------------------------------------------------------
Function name	: sortList - 노드 정렬(오름차순)
Parameters		: lp - 리스트 관리 구조체의 주소
Returns			: 없음
----------------------------------------------------------------------------------*/
void sortList(List *lp)
{
	Node *curp;
	Node *nextp;
	int temp;
	if (lp == NULL) { /* lp포인터 NULL check */
		return;
	}
	if (lp->size <= 1) {
		printf("정렬할 data가 1개 이하입니다.\n");
		return FALSE;
	}
	curp = lp->head->next;

	while (curp!=lp->tail) {
		nextp = curp->next;
		while (nextp != lp->tail) {
			if (curp->data > nextp->data) {
				temp = curp->data;
				curp->data = nextp->data;
				nextp->data = temp;
			}
			nextp=nextp->next;
		}
		curp = curp->next;
	}

	return;
}
/*----------------------------------------------------------------------------------
Function name	: destroyList - 리스트 내의 모든 노드(head, tail 노드 포함)를 삭제
Parameters		: lp - 리스트 관리 구조체의 주소
Returns			: 없음
----------------------------------------------------------------------------------*/
void destroyList(List *lp)
{
	Node *curp;
	Node *nextp;
	if (lp == NULL) { /* lp포인터 NULL check */
		return;
	}

	curp = lp->head;
	nextp= lp->head->next;
	while (nextp != lp->tail) {
		curp = nextp;
		nextp = nextp->next;
		free(curp);		
	}
	free(lp->head);
	free(lp->tail);

	return;
}

