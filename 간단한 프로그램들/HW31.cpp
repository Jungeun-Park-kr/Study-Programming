#include <stdio.h>
#pragma warning(disable:4996)

int menu();
int inputInt(const char *);
void myflush();
void deposit(int *);
void withdraw(int *);
void myflush() {
	while (getchar() != '\n') {
		;
	}
}
int inputInt(const char * msg) {
	int num;
	printf(msg);
	scanf("%d", &num);
	while (1) {
		while (getchar() != '\n') {//���� �� ���� �˻�
			myflush();
			printf("* �߸� �Է��ϼ̽��ϴ�. �ٽ� �Է��Ͻʽÿ� : ");
			scanf("%d", &num);
		}
		if (num<0) { //���� ���� �˻�
			printf("* �߸� �Է��ϼ̽��ϴ�. �ٽ� �Է��Ͻʽÿ� : ");
			scanf("%d", &num);
		}
		else {
			break;
		}
	}
	
	return num;
}

int menu() {
	char select;
	int result = -1;
	printf("\n\n# �޴��� �����Ͻÿ�(i-�Ա�, o-���, q-����) : ");
	scanf("%c", &select);
	myflush();
	switch (select) {
	case 'i' : //�Ա�
		result = 0;
		break;
	case 'o' : //���
		result= 1;
		break;
	case 'q' : //����
		result= -1;
		break;
	default : //�߸��Է�
		printf("* �߸� �Է��ϼ̽��ϴ�.\n");
		result = 2;
	}
	return result;
}

void deposit(int *money) {
	int deposit = inputInt("# �Աݾ��� �Է��ϼ��� : ");
	*money += deposit;
	printf("* ���� �ܾ��� %d�� �Դϴ�.\n", *money);
}
void withdraw(int *money) {
	int withdraw = inputInt("# ��ݾ��� �Է��ϼ��� : ");
	if (withdraw > *money) {
		printf("* �ܾ��� �����մϴ�.\n");
		printf("* ���� �ܾ��� %d�� �Դϴ�.\n", *money);
		return;
	}
	else {
		*money -= withdraw;
		printf("*���� �ܾ��� %d�� �Դϴ�.\n", *money);
	}
	
}

int main() {
	int money = 0, select=0;
	printf("* ���� �ܾ��� %d�� �Դϴ�.\n", money);
	//select = menu();
	while (1) {
		select = menu();
		if (select == 0) {
			deposit(&money);
		}
		else if (select == 1) {
			withdraw(&money);
		}
		else if(select==-1) { //-1 ���� ��ư
			break;
		}
		else { //2 : �߸� ����
			;
		}
	}

	return 0;
}