#include <stdio.h>
#pragma warning(disable:4996)

int input(void);
void myflush();


void myflush() {
	while (getchar() != '\n') {
		;
	}
}

int input() {
	int weight;
	printf("# 계란의 무게를 입력하세요(단위 : g) : ");
	scanf("%d", &weight);

	while (getchar() != '\n') {//숫자 외 문자 검사
		myflush();
		printf("# 숫자를 다시 입력하시오 : ");
		scanf("%d", &weight);
	}

	return weight;
}
int main() {
	int weight, packing=0;
	
	while (packing < 10) {
		weight = input();

		if (weight < 150) {
			printf("* 메추리알 가지고 장난하지 마시오~ ^^\n");
		}
		else if (weight > 500) {
			printf("* 타조알 가지고 장난하지 마시오~ ^^\n");
		}
		else {
			printf("* 현재 달걀의 수 : %d\n", ++packing);
		}
	}
	printf("\n\n*** 달걀 포장이 끝났습니다.");

	return 0;
}