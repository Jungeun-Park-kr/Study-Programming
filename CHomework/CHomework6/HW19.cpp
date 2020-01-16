#include<stdio.h>
#pragma warning(disable:4996)

int input(const char *);
int yearCheck(int year);
void output(int year, int result);

int input(const char *msg) {
	int year=0;
	printf(msg);
	scanf("%d", &year);

	return year;
}

int yearCheck(int year) {
	int result; //���� : 1, ��� : 0 ����
	
	if (year % 4 == 0) {
		if (year % 100 == 0) {
			if (year % 400 == 0) {
				return 1;
			}
			else {
				return 0;
			}
		}
		else {
			return 1;
		}
	}
	else {
		return 0;
	}


	/*switch (year % 4) {
	case 0 :
		switch(year % 100) {
		case 0 :
			switch (year % 400) {
			case 0:
				return 1;
				break;
			default:
				return 0;
				break;
			}
		default :
			return 1;
			break;
		}
	default:
		return 0;
	}*/


	return result;
}

void output(int year, int result) {
	switch (result) {
	case 0 : //���
		printf("%d���� ���(Common year) �Դϴ�.", year);
		break;
	case 1: //����
		printf("%d���� ����(Leap year) �Դϴ�.", year);
		break;
	}
	
	return;
}

int main() {
	int year, result=0;
	year = input("�⵵�� �Է��Ͻÿ� : ");
	result = yearCheck(year);
	output(year, result);

	return 0;
}