#include "singlyLinkedlist.h"
#include <stdio.h>  // printf(), scanf()
#include <stdlib.h>  // malloc(), free()

/*----------------------------------------------------------------------------------
Function name	: createList - ���� ����Ʈ ���� �� �ʱ�ȭ
Parameters		: lp - ����Ʈ ���� ����ü�� �ּ�
Returns			: ���� - TRUE / ���� - FALSE
----------------------------------------------------------------------------------*/
int createList(List *lp)
{
	if (lp == NULL) { /* lp������ NULL check */
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
Function name	: addFirst - head node �ڿ� node �߰�(���� ����)
Parameters		: lp - ����Ʈ ���� ����ü�� �ּ�
				  data - �߰��� ������
Returns			: ���� - TRUE / ���� - FALSE
----------------------------------------------------------------------------------*/
int addFirst(List *lp, int data)
{
	Node *newp;
	if (lp == NULL) { /* lp������ NULL check */
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

	return TRUE; // return ���� �������ּ���.
}
/*----------------------------------------------------------------------------------
Function name	: addLast - tail node �տ� �� node �߰�(���� ����)
Parameters		: lp - ����Ʈ ���� ����ü�� �ּ�
				  data - �߰��� ������
Returns			: ���� - TRUE / ���� - FALSE
----------------------------------------------------------------------------------*/
int addLast(List *lp, int data)
{
	Node *newp;	/* �� ��� �ּ������ ������ */
	Node *btp;	/* tail node �ٷ� �� ��带 ����Ű�� ������ */
	Node *curp;

	if (lp == NULL) { /* lp������ NULL check */
		return FALSE;
	}
	newp = (Node *)malloc(sizeof(Node));
	if (newp == NULL) {
		return FALSE;
	}
	newp->data = data;
	//printf("�Է��� data : %d\t", newp->data);

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
	//printf("�߰��� �� size : %d\n", lp->size);
	return TRUE; 
}

/*----------------------------------------------------------------------------------
Function name	: displayList - ����Ʈ ���� ��� ������ ���
Parameters		: lp - ����Ʈ ���� ����ü�� �ּ�
Returns			: ����
----------------------------------------------------------------------------------*/
void displayList(List *lp)
{
	Node *curp;

	if (lp == NULL) { /* lp������ NULL check */
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
Function name	: searchNode - data�� ��ġ�ϴ� ù ��° node �˻�
Parameters		: lp - ����Ʈ ���� ����ü�� �ּ�
				  data - �˻��� ������
Returns			: ���� - �˻��� ����� �ּ� / ���� - NULL pointer
----------------------------------------------------------------------------------*/
Node * searchNode(List *lp, int data)
{
	Node *curp;
	
	if (lp == NULL) { /* lp������ NULL check */
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
Function name	: removeNode - data�� ��ġ�ϴ� ù ��° ��� ����
Parameters		: lp - ����Ʈ ���� ����ü�� �ּ�
data - ������ ������
Returns			: ���� - TRUE / ���� - FALSE
----------------------------------------------------------------------------------*/
int removeNode(List *lp, int data)
{
	Node *delp;
	Node *curp;

	if (lp == NULL) { /* lp������ NULL check */
		return FALSE;
	}

	delp = lp->head->next;
	curp = lp->head;
	//printf("ù ��° ��� : %d\n", lp->head->data);
	//printf("size : %d\n", lp->size);
	if (delp->data == data) {
		lp->head->next = delp->next;
		lp->size = lp->size - 1;
		//printf("ù ��° ��� : %d\n", lp->head->data);
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
Function name	: sortList - ��� ����(��������)
Parameters		: lp - ����Ʈ ���� ����ü�� �ּ�
Returns			: ����
----------------------------------------------------------------------------------*/
void sortList(List *lp)
{
	Node *curp;
	Node *nextp;
	int temp;
	if (lp == NULL) { /* lp������ NULL check */
		return;
	}
	if (lp->size <= 1) {
		printf("������ data�� 1�� �����Դϴ�.\n");
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
Function name	: destroyList - ����Ʈ ���� ��� ���(head, tail ��� ����)�� ����
Parameters		: lp - ����Ʈ ���� ����ü�� �ּ�
Returns			: ����
----------------------------------------------------------------------------------*/
void destroyList(List *lp)
{
	Node *curp;
	Node *nextp;
	if (lp == NULL) { /* lp������ NULL check */
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

