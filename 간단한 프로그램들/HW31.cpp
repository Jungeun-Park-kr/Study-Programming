#include <stdio.h>
#pragma warning(disable:4996)

int menu();
int inputInt(const char *);
void myflush();
void deposit(int *);
void withdraw(int *);
void myflush() {
	while (getchar() != '\n') {
		;
	}
}
int inputInt(const char * msg) {
	int num;
	printf(msg);
	scanf("%d", &num);
	while (1) {
		while (getchar() != '\n') {//숫자 외 문자 검사
			myflush();
			printf("* 잘못 입력하셨습니다. 다시 입력하십시오 : ");
			scanf("%d", &num);
		}
		if (num<0) { //숫자 범위 검사
			printf("* 잘못 입력하셨습니다. 다시 입력하십시오 : ");
			scanf("%d", &num);
		}
		else {
			break;
		}
	}
	
	return num;
}

int menu() {
	char select;
	int result = -1;
	printf("\n\n# 메뉴를 선택하시오(i-입금, o-출금, q-종료) : ");
	scanf("%c", &select);
	myflush();
	switch (select) {
	case 'i' : //입금
		result = 0;
		break;
	case 'o' : //출금
		result= 1;
		break;
	case 'q' : //종료
		result= -1;
		break;
	default : //잘못입력
		printf("* 잘못 입력하셨습니다.\n");
		result = 2;
	}
	return result;
}

void deposit(int *money) {
	int deposit = inputInt("# 입금액을 입력하세요 : ");
	*money += deposit;
	printf("* 현재 잔액은 %d원 입니다.\n", *money);
}
void withdraw(int *money) {
	int withdraw = inputInt("# 출금액을 입력하세요 : ");
	if (withdraw > *money) {
		printf("* 잔액이 부족합니다.\n");
		printf("* 현재 잔액은 %d원 입니다.\n", *money);
		return;
	}
	else {
		*money -= withdraw;
		printf("*현재 잔액은 %d원 입니다.\n", *money);
	}
	
}

int main() {
	int money = 0, select=0;
	printf("* 현재 잔액은 %d원 입니다.\n", money);
	//select = menu();
	while (1) {
		select = menu();
		if (select == 0) {
			deposit(&money);
		}
		else if (select == 1) {
			withdraw(&money);
		}
		else if(select==-1) { //-1 종료 버튼
			break;
		}
		else { //2 : 잘못 누름
			;
		}
	}

	return 0;
}