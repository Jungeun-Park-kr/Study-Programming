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
	printf("# 숫자를 입력하시오 : ");
	scanf("%d", &num);
	
	while (repeat) {
		while (getchar() != '\n') {//숫자 외 문자 검사
			myflush();
			printf("# 숫자를 다시 입력하시오 : ");
			scanf("%d", &num);
		}
		if (num <= 0 || num > 100) { //숫자 범위 검사
			printf("# 숫자를 다시 입력하시오 : ");
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
		if (user > *min) { //입력 값이 기존 min 보다 큰 경우만 갱신
			*min = user; 
		}
		printf("%d 보다는 크고 %d 보다는 작습니다.\n", *min, *max);
	}
	else {
		if (user < *max) { //입력한 값이 기존 max보다 작을때만 갱신
			*max = user;
		}
		printf("%d 보다는 크고 %d 보다는 작습니다.\n", *min, *max);
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
			printf("ㅊㅋㅊㅋ %d번째 만에 맞추셨습니다.", count);
			break;
		}
		output(user_num,rand_num, min, max);
	}

	return 0;
}