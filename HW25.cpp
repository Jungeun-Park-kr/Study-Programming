#include <stdio.h>
#include<time.h>
#include<stdlib.h>
#pragma warning(disable:4996)

int input();
void output(int, int, int*, int*);
void myflush();

void myflush() {
	while (getchar() != '\n') {
		;
	}
}
int input() {
	int num, repeat=1;
	printf("# ���ڸ� �Է��Ͻÿ� : ");
	scanf("%d", &num);
	
	while (repeat) {
		while (getchar() != '\n') {//���� �� ���� �˻�
			myflush();
			printf("# ���ڸ� �ٽ� �Է��Ͻÿ� : ");
			scanf("%d", &num);
		}
		if (num <= 0 || num > 100) { //���� ���� �˻�
			printf("# ���ڸ� �ٽ� �Է��Ͻÿ� : ");
			scanf("%d", &num);
		}
		else {
			break;
		}
	}


	return num;
}
void output(int user, int rand, int *min, int *max) {
	int result = 0;
	if (user < rand) { 
		if (user > *min) { //�Է� ���� ���� min ���� ū ��츸 ����
			*min = user; 
		}
		printf("%d ���ٴ� ũ�� %d ���ٴ� �۽��ϴ�.\n", *min, *max);
	}
	else {
		if (user < *max) { //�Է��� ���� ���� max���� �������� ����
			*max = user;
		}
		printf("%d ���ٴ� ũ�� %d ���ٴ� �۽��ϴ�.\n", *min, *max);
	}
	
}

int main() {
	int user_num, rand_num, count=0;
	int minimum = 0, maximum = 100;
	int *min = &minimum;
	int *max = &maximum;
	srand((unsigned int)time(NULL));
	rand_num = rand() % 100 + 1;

	while (1) {
		user_num = input();
		count++;
		if (user_num == rand_num) {
			printf("�������� %d��° ���� ���߼̽��ϴ�.", count);
			break;
		}
		output(user_num,rand_num, min, max);
	}

	return 0;
}