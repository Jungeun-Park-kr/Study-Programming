#include<stdio.h>
#pragma warning(disable:4996)


int inputYMD(int*, int*, int*);
int yearCheck(int);
int isError(int*, int*, int*);
int calcDays(int*, int*, int*);
void output(int *, int*, int*, int);

int isError(int *year, int *month, int *day) {
	int result = 0;
	if (*year < 1900) {
		return -1;
	}
	if (*month < 1 || *month >12) {
		return -1;
	}
	if (*month == 2) {
		if (*day == 29 && !yearCheck(*year)) {
			return -1;
		}
		else { ; }
	}

	return result;
}

void output(int *year, int *month, int *day, int days) {
	printf("  %d�� %d�� %d���� ", *year, *month, *day);
	switch (days % 7) {
		case 0:
			printf("�Ͽ���");
			break;
		case 1:
			printf("������");
			break;
		case 2:
			printf("ȭ����");
			break;
		case 3:
			printf("������");
			break;
		case 4:
			printf("�����");
			break;
		case 5:
			printf("�ݿ���");
			break;
		case 6:
			printf("�����");
			break;
		default:
			break;
	}
	printf("�Դϴ�.\n");
	return;
}

int calcDays(int *year, int *month, int *day) {
	int days = 0;
	for (int i = 1990; i < *year; i++) {
		if (yearCheck(i))//�����̸� 1
			days += 366;
		else //���
			days += 365;
	}
	for (int i = 1; i < *month; i++) {
		if (i == 1 || i == 3 || i == 5 || i == 7 || i == 8 || i == 10 || i == 12) {
			days += 31;
		}
		else if (i == 2) {
			if (yearCheck(*year)) {
				days += 29;
			}
			else {
				days += 28;
			}
		}
		else {
			days += 30;
		}
	}
	days += (*day);


	return days;
}


int inputYMD(int *year, int *month, int *day) {
	int check=0, repeat=0;

	while (1) {
		printf("* �� �� ���� �Է��Ͻÿ� : ");
		scanf("%d %d %d", year, month, day);


		while (getchar() != '\n') { //�����Է½� -1���� �� ����
			return -1;
		}

		check = isError(year, month, day);
		if (check == 0) {
		//�����Է�
			break;
		}
		else { //�������Է�
			;
		}
	}

	return 0;
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


	return result;
}



int main() {
	int year, month, day, check, days;
	check = inputYMD(&year, &month, &day);

	if (check == -1) //���� �Է½� ����
		return 0;

	days = calcDays(&year, &month, &day);
	output(&year, &month, &day, days);

	return 0;
}