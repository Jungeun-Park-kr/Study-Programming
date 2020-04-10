#include <stdio.h>
#pragma warning(disable:4996)

void myflush();
void output(int);

void myflush() {
	while (getchar() != '\n') {
		;
	}
}

void output(int day) {
	int coin=0;
	int i, j, repeat=0;
	for (i = 1; i <= day; i++) {
		for (j = 1; j <= i; j++) {
			coin += i;
			++repeat;
			if (repeat == day) {
				break;
			}
			
		}
		if (repeat == day) {
			break;
		}
	}
	printf("근무일 : %d일 / 총 금화 수 %d 개", day, coin);
}

int main() {
	int day;

	while (1) {
		printf("* 기사의 근무일수를 입력하시오 : ");
		scanf("%d", &day);
		while (getchar() != '\n') { //문자재입력
			myflush();
			printf("* 기사의 근무일수를 입력하시오 : ");
			scanf("%d", &day);
		}
		if (day < 0 ) { //음수재입력
			printf("* 기사의 근무일수를 입력하시오 : ");
			scanf("%d", &day);
		}
		else {
			break;
		}
	}
	
	output(day);

	return 0;
}