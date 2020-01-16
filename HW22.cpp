#include <stdio.h>
#pragma warning(disable:4996)

void myflush();
int input(const char *);

void myflush() {
	while (getchar() != '\n') {
		;
	}
	return;
}

int input(const char *msg) {
	int num;
	printf(msg);
	scanf("%d", &num);
	while (getchar() != '\n') {
		myflush();
		printf(msg);
		scanf("%d", &num);
	}
	return num;
}

int main() {
	int num;
	num = input("# 정수값을 입력하세요 : ");
	
	int i;
	for (i = 1; i <= num; i++) {
		printf("*");
		if (i % 5 == 0) {
			printf("\n");
		}
	}

	return 0;
}