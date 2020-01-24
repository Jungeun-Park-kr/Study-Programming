#pragma once
#pragma warning (disable : 4996)

enum {FALSE, TRUE};
typedef struct _node Node;				/* ����ü ��� ���������� */
struct  _node {							/* ��� ����ü (�ڱ����� ����ü ���) */
	int data; 							/* ������ ���� : int�� ������ ���� */
	Node *next;							/* ������ ���� */
};
typedef  struct  _list { 				/* ���� ����Ʈ ���� ����ü */
	Node *head;							/* head pointer (head node ����Ŵ) */
	Node *tail; 						/* tail pointer (tail node ����Ŵ) */
	int size;							/* ���� ����Ʈ�� ũ�� - ���� data node�� ���� */
}List;

int createList(List *lp);					/* ���� ����Ʈ �ʱ�ȭ */
int addFirst(List *lp, int data);			/* head node �ڿ� node �߰�(���� ����) */
int addLast(List *lp, int data);			/* tail node �տ� node �߰�(���� ����) */
void displayList(List *lp);					/* ����Ʈ ���� ��� ������ ��� */
int removeNode(List *lp, int data);		/* data ��� ���� */
Node * searchNode(List *lp, int data);		/* data�� ��ġ�ϴ� node �˻� */
void sortList(List *lp);					/* ��� ���� - �������� */
void destroyList(List *lp);					/* ����Ʈ ���� ��� ��带 ���� */
