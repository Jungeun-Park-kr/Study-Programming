#include<stdio.h>
#pragma warning(disable:4996)

int input();

int input() {
	int num;
	printf("정수값을 입력하세요 : ");
	scanf("%d", &num);
	return num;
}

int main() {
	int number;
	number = input();
	
	printf("입력된 값은 8 진수로 %#o 입니다.\n", number);
	printf("입력된 값은 16 진수로 %#x 입니다.", number);

	return 0;
}