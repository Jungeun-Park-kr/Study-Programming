#include<stdio.h>
#include <time.h>
#include<stdlib.h>
#pragma warning(disable:4996)

#define ROCK 1
#define SCISSORS 2
#define PAPER 3

int input(void);
void myflush(void);
int random(int num);
int output(int user, int com, int *win, int *draw);

int output(int user, int com, int *win, int *draw) {
	/* 바위(1) 가위(2) 보(3)
	 * 질 때까지 진행 계속 진행
	 * 종료 전 게임 결과(n무,n승) 출력 후 종료 (main)
	 */
	if (user == ROCK) { 
		if (com==ROCK) {
			printf("당신은 바위 선택, 컴퓨터는 바위 선택 : 비겼습니다.\n");
			++*draw;
			return 1;
		}
		else if (com ==SCISSORS) {
			printf("당신은 바위 선택, 컴퓨터는 가위 선택 : 이겼습니다.\n");
			++*win;
			return 1;
		}
		else {
			printf("당신은 바위 선택, 컴퓨터는 보 선택 : 졌습니다.\n");
			return 0;
		}
	}
	else if (user ==SCISSORS) {
		if (com == ROCK) {
			printf("당신은 가위 선택, 컴퓨터는 바위 선택 : 졌습니다.\n");
			return 0;
		}
		else if (com == SCISSORS) {
			printf("당신은 가위 선택, 컴퓨터는 가위 선택 : 비겼습니다.\n");
			++*draw;
			return 1;
		}
		else {
			printf("당신은 가위 선택, 컴퓨터는 보 선택 : 이겼습니다.\n");
			++*win;
			return 1;
		}
	}
	else { 
		if (com == ROCK) {
			printf("당신은 보 선택, 컴퓨터는 바위 선택 : 이겼습니다.\n");
			++*win;
			return 1;
		}
		else if (com == SCISSORS) {
			printf("당신은 보 선택, 컴퓨터는 가위 선택 : 졌습니다.\n");
			return 0;
		}
		else {
			printf("당신은 보 선택, 컴퓨터는 보 선택 : 비겼습니다.\n");
			++*draw;
			return 1;
		}
	}

}

int random(int num) {
	int res;
	res = rand() % num;
	return res;
}

void myflush() {
	while (getchar() != '\n') {
		;
	}
}

int input() {
	int user;
	printf("# 바위는 1, 가위는 2, 보는 3 중에서 선택하세요 : ");
	scanf("%d", &user);

	while (1) {
		while (getchar() != '\n') {//숫자 외 문자 검사
			myflush();
			printf("# 바위는 1, 가위는 2, 보는 3 중에서 다시 선택하세요 : ");
			scanf("%d", &user);
		}
		if (user<1 || user>3) { //숫자 범위 검사
			printf("# 바위는 1, 가위는 2, 보는 3 중에서 다시 선택하세요 : ");
			scanf("%d", &user);
		}
		else {
			break;
		}
	}
	return user;
}


int main() {
	int user, com, win_time=0, draw_time=0;
	int *win = &win_time;
	int *draw = &draw_time;
	srand((unsigned int)time(NULL));
	
	int repeat = 1;
	while (repeat) {
		user = input();
		com = random(3) + 1; //0~2까지의 난수 +1
		repeat = output(user, com, win, draw); //이기거나 비기면 1 리턴, 지면 0리턴 -> 반복 종료
	}
	printf("게임결과 : %d승 %d무", *win, *draw);

	return 0;
}