#include <stdio.h>
#pragma warning(disable:4996)

int transNumber(int num);
int inputUInt(int *first, int *last);
void output(int *first, int *last, int gozip);
void myflush();


int main() {
	int P1 = 0, P2 = 0, gozip_number = 0;
	int *first = &P1;
	int *last = &P2;
	int *gozip = &gozip_number;

	gozip_number = inputUInt(first, last);
	output(first, last, gozip_number);

	return 0;
}

int inputUInt(int *first, int *last) {
	int num, num2, gozip_num;
	printf("���� ��(P1) : ");
	scanf("%d", &num);
	while (1) {
		while (getchar() != '\n') {//���� �� ���� �˻�
			myflush();
			printf("���� ��(P1) : ");
			scanf("%d", &num);
		}
		if (num < 100 || num >10000) { //���� ���� �˻�
			printf("���� ��(P1) : ");
			scanf("%d", &num);
		}
		else {
			*first = num;
			break;
		}
		*first = num;
	}

	printf("�� ��(P2) : ");
	scanf("%d", &num2);
	while (1) {
		while (getchar() != '\n') {//���� �� ���� �˻�
			myflush();
			printf("�� ��(P2) : ");
			scanf("%d", &num2);
		}
		if (num2 < num || num2>10000) { //���� ���� �˻�
			printf("�� ��(P2) : ");
			scanf("%d", &num2);
		}
		else {
			*last = num2;
			break;
		}
		*last = num2;
	}

	printf("������(N) : ");
	scanf("%d", &gozip_num);
	while (1) {
		while (getchar() != '\n') {//���� �� ���� �˻�
			myflush();
			printf("������(N) : ");
			scanf("%d", &gozip_num);
		}
		if (gozip_num < 1 || gozip_num>10) { //���� ���� �˻�
			printf("������(N) : ");
			scanf("%d", &gozip_num);
		}
		else {
			break;
		}
	}

	return gozip_num;
}


void output(int *P1, int *P2, int gozip) {
	int cal_gozip = 0, count, number;
	int first = *P1;
	int last = *P2;
	printf("�������� %d�� ���� ���\n", gozip);

	while (first <= last) {
		count = 0;
		number = first;
		while (number > 10) {
			number = transNumber(number); //number : ���� ��� ���� �Լ�
			++count;
		}
		if (count == gozip) { //��� �� �������� ���� �Է��� �������� ���� ��
			++cal_gozip;
			printf("%d\n", first);
			++first;
		}
		else {
			++first;
		}
	}
	printf("�� ���� : %d��", cal_gozip);

}


int transNumber(int num) {
	int number = num;
	int d1, d2, d3, d4; //���� 1, 10, 100, 1000�� �ڸ� ��


	if (number < 100) { //2�ڸ� �� (��� ���߿� �ʿ�)
		d2 = number / 10;
		int tmp = number - d2 * 10;
		d1 = tmp;

		number = d2 * d1;
	}
	else if (100 <= number && number < 1000) { //3�ڸ� ��
		d3 = number / 100;
		int tmp = number - d3 * 100;
		d2 = tmp / 10;
		tmp = tmp - d2 * 10;
		d1 = tmp;

		number = d3 * d2 * d1;

	}
	else if (1000 <= number && number < 10000) { //4�ڸ� ��
		d4 = number / 1000;
		int tmp = number - d4 * 1000;
		d3 = tmp / 100;
		tmp = tmp - d3 * 100;
		d2 = tmp / 10;
		tmp = tmp - d2 * 10;
		d1 = tmp;

		number = d4 * d3 * d2 * d1;
	}
	else if (number == 10000) { //(10000�ۿ� ����)
		return 0; //1*0*0*0=0
	}
	else { ; }

	return number;
}


void myflush() {
	while (getchar() != '\n') {
		;
	}
}