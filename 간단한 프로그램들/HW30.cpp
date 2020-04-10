#include <stdio.h>
#pragma warning(disable:4996)

void myflush();
void printResult(int);

void myflush() {
	while (getchar() != '\n') {
		;
	}
}
void printResult(int num) {
	if (num == 0) {
		printf("0");
	}
	else {
		printf("1");
	}
}

int main() {
	int num, binary = 0;
	printf("* 10���� ������ �Է��Ͻÿ� : ");
	scanf("%d", &num);
	while (getchar() != '\n') {//���� �� ���� �˻�
		myflush();
		printf("* 10���� ������ �ٽ� �Է��Ͻÿ� : ");
		scanf("%x", &num);
	}

	binary = 0 | num;
	int res = 0;

	printf("%d(10) = ", num);
	int i;

	unsigned int j = 0x80000000;
	for (int i = 0; i < 32; i++,j/=2) {
		res = num & j;
		printResult(res);
	}
	printf("(2)");
	return 0;
}