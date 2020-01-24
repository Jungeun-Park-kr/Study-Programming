#include <stdio.h>
#include <string.h>
#include "singlyLinkedlist.h"

int menu(const char **mList, size_t menuCnt);
void mInput(List *lp);		/* �Է¸޴� ó�� �Լ� */
void mOutput(List *lp);		/* ��¸޴� ó�� �Լ� */
void mSearch(List *lp);		/* �˻��޴� ó�� �Լ� */
void mDelete(List *lp);		/* �����޴� ó�� �Լ� */
void mSort(List *lp);		/* ���ĸ޴� ó�� �Լ� */
void myflush();				/* �Է� ���� flush �Լ� */
/*----------------------------------------------------------------------------------
  Function name : main
----------------------------------------------------------------------------------*/
int main()
{
	const char *menuList[] = { "�Է��ϱ�","����ϱ�","�˻��ϱ�","�����ϱ�", "�����ϱ�", "��  ��" };
	int menuNum;	/* �޴���ȣ ���� ���� */
	int menuCnt;	/* �޴����� ���� ���� */
	List list;		/* ����Ʈ���� ����ü ���� */
	int bres;

	menuCnt = sizeof(menuList) / sizeof(menuList[0]);

	bres = createList(&list);		/* ����ִ� ����Ʈ �ʱ�ȭ */
	if (bres == TRUE){
		printf("@ list ���� ����!\n");
	}
	else{
		printf("@ list ���� ����!\n");
		return 0;
	}
	while (1)
	{
		menuNum = menu(menuList, menuCnt);	/* �޴�ȭ���� ���� �޴���ȣ�� �Է� ���� */
		if (menuNum == menuCnt) { break; }
		switch (menuNum)
		{
		case 1: mInput(&list); break;		/* �Է¸޴� ���� */
		case 2: mOutput(&list); break;		/* ��¸޴� ���� */
		case 3: mSearch(&list); break;		/* �˻��޴� ���� */
		case 4: mDelete(&list); break;		/* �����޴� ���� */
		case 5: mSort(&list); break;		/* ���ĸ޴� ���� */
		}
	}
	printf("list���� ������ ����� ���� : %d\n", list.size);

	destroyList(&list);	/* ����Ʈ���� ��� ������ ���� */

	return 0;
}
/*----------------------------------------------------------------------------------
Function name	: menu
Parameters		: mList - �޴� ��� �迭
				  menuCnt - �޴� ����
Returns			: ����� ������ �޴���ȣ
----------------------------------------------------------------------------------*/
int menu(const char **mList, size_t menuCnt)
{
	size_t menuNum = 0;	/* �������� �ʴ� �޴� ��ȣ ���� */
	size_t i;

	printf("\n\n");
	for (i = 0; i < menuCnt; i++) {	/* �޴� ��� */
		printf("%d. %s\n", i + 1, mList[i]);
	}

	while (menuNum<1 || menuNum>menuCnt) {	/* �޴���ȣ�� ��ȿ���� ���� ���� �ݺ� */
		printf("# �޴� ���� : ");
		scanf("%d", &menuNum);	/* �޴� ��ȣ �Է� */
	}
	return menuNum;
}
/*----------------------------------------------------------------------------------
Function name	: mInput - �Է� �޴� ó�� �Լ�
Parameters		: lp - ����Ʈ ���� ����ü�� �ּ�
Returns			: ����
----------------------------------------------------------------------------------*/
void mInput(List *lp)
{
	int inData;
	int res;  /* scanf()�Լ��� ���� �� ���� */
	int bres;

	printf("\n[ �Է��ϱ� �޴� ]\n\n");

	while (1) {
		printf("# ������ �Է��ϼ���(���� �Է½� ����) : ");
		res = scanf("%d", &inData);	/* scanf()�Լ��� ���� �� : ���� �Է� �� 1, ���� �Է� �� 0���� ��*/
		if (res == 0) {	/* ���� �Է� �� ���� */
			myflush();
			break;
		}
		//bres = addFirst(lp, inData);	/* tail ��� �տ� ������ �߰�  */
		bres = addLast(lp, inData);
		if (bres == TRUE)
			printf("@ ������ �߰� ����!\n");
		else
			printf("@ ������ �߰� ����!\n");
	}
	return;
}
/*----------------------------------------------------------------------------------
Function name	: mOutput - ��� �޴� ó�� �Լ�
Parameters		: lp - ����Ʈ ���� ����ü�� �ּ�
Returns			: ����
----------------------------------------------------------------------------------*/
void mOutput(List *lp)
{
	displayList(lp);
}
/*----------------------------------------------------------------------------------
Function name	: mSearch - �˻� �޴� ó�� �Լ�
Parameters		: lp - ����Ʈ ���� ����ü�� �ּ�
Returns			: ����
----------------------------------------------------------------------------------*/
void mSearch(List * lp)
{
	int sData;
	Node *resp;		/* �˻��� ����� �����ּ� ���� */
	int res;		/* scanf()�Լ��� ���� �� ���� */

	printf("\n[ �˻��ϱ� �޴� ]\n\n");
	while (1) {
		printf("# ã�� �����͸� �Է��ϼ���(���� �Է� �� ����) : ");
		res = scanf("%d", &sData);	/* scanf()�Լ��� ���� �� : ���� �Է� �� 1, ���� �Է� �� 0���� ��*/
		if (res == 0) {		/* ���� �Է� �� ���� */
			myflush();
			break;
		}
		else {
			;
		}
		resp = searchNode(lp, sData);
		if (resp != NULL) {	/* �����͸� ã������ */
			printf("@ �˻� ������ ����!\n");
		}
		else {				/* �����͸� ��ã������ */
			printf("@ �˻� ������ �������� ����!\n");
		}
	}
	return;
}
/*----------------------------------------------------------------------------------
Function name	: mDelete - ���� �޴� ó�� �Լ�
Parameters		: lp - ����Ʈ ���� ����ü�� �ּ�
Returns			: ����
----------------------------------------------------------------------------------*/
void mDelete(List * lp)
{
	int delData;
	int res;		/* scanf()�Լ��� ���� �� ���� */
	int bres;

	printf("\n[ �����ϱ� �޴� ]\n\n");
	while (1) {
		printf("# ������ �����͸� �Է��ϼ���(���� �Է� �� ����) : ");
		res = scanf("%d", &delData);	/* scanf()�Լ��� ���� �� : ���� �Է� �� 1, ���� �Է� �� 0���� ��*/
		if (res == 0) {		/* ���� �Է� �� ���� */
			myflush();
			break;
		}
		else {
			;
		}
		bres = removeNode(lp, delData);
		if (bres == TRUE)
			printf("@ ���� ����!\n");
		else
			printf("@ ���� ����!\n");
	}
	return;
}
/*----------------------------------------------------------------------------------
Function name	: mSort - ���� �޴� ó�� �Լ�
Parameters		: lp - ����Ʈ ���� ����ü�� �ּ�
Returns			: ����
----------------------------------------------------------------------------------*/
void mSort(List *lp)
{
	sortList(lp);
}
/*----------------------------------------------------------------------------------
Function name	: myflush - �Է� ���� ���� ��� ������ ����� �Լ�
Parameters		: ����
Returns			: ����
----------------------------------------------------------------------------------*/
void myflush()
{
	while (getchar() != '\n') {
		;
	}
}
