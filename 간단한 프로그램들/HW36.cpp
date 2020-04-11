#include <stdio.h>
#pragma warning(disable:4996)

int fibonacci(int);
int input();
void myflush();

int fibonacci(int num) {
	int sum = 0, result=0;
	int i;
	if (num == 1 || num==2) {
		return 1;
	}
	else {
		return (fibonacci(num - 2) + fibonacci(num-1));
	}

}

int input() {
	int num;

	while (1) {
		printf("피보나치 수열의 항수를 입력하시오 : ");
		scanf("%d", &num);
		while (getchar() != '\n') { //문자재입력
			myflush();
			printf("피보나치 수열의 항수를 입력하시오 : ");
			scanf("%d", &num);
		}
		if (num < 0) { //음수재입력
			printf("피보나치 수열의 항수를 입력하시오 : ");
			scanf("%d", &num);
		}
		else {
			break;
		}
	}
	return num;

}

void myflush() {
	while (getchar() != '\n')
		;
}

int main() {
	int num, result, sum=0;

	num = input();

	for (int i = 1; i <= num; i++) {
		if (i == num) {
			result = fibonacci(i);
			printf(" %d", result);
			sum += result;
			break;
		}
		result = fibonacci(i);
		printf(" %d +", result);
		sum += result;
	}
	printf(" = %d", sum);
	return 0;
}