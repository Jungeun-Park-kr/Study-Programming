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
	/* ����(1) ����(2) ��(3)
	 * �� ������ ���� ��� ����
	 * ���� �� ���� ���(n��,n��) ��� �� ���� (main)
	 */
	if (user == ROCK) { 
		if (com==ROCK) {
			printf("����� ���� ����, ��ǻ�ʹ� ���� ���� : �����ϴ�.\n");
			++*draw;
			return 1;
		}
		else if (com ==SCISSORS) {
			printf("����� ���� ����, ��ǻ�ʹ� ���� ���� : �̰���ϴ�.\n");
			++*win;
			return 1;
		}
		else {
			printf("����� ���� ����, ��ǻ�ʹ� �� ���� : �����ϴ�.\n");
			return 0;
		}
	}
	else if (user ==SCISSORS) {
		if (com == ROCK) {
			printf("����� ���� ����, ��ǻ�ʹ� ���� ���� : �����ϴ�.\n");
			return 0;
		}
		else if (com == SCISSORS) {
			printf("����� ���� ����, ��ǻ�ʹ� ���� ���� : �����ϴ�.\n");
			++*draw;
			return 1;
		}
		else {
			printf("����� ���� ����, ��ǻ�ʹ� �� ���� : �̰���ϴ�.\n");
			++*win;
			return 1;
		}
	}
	else { 
		if (com == ROCK) {
			printf("����� �� ����, ��ǻ�ʹ� ���� ���� : �̰���ϴ�.\n");
			++*win;
			return 1;
		}
		else if (com == SCISSORS) {
			printf("����� �� ����, ��ǻ�ʹ� ���� ���� : �����ϴ�.\n");
			return 0;
		}
		else {
			printf("����� �� ����, ��ǻ�ʹ� �� ���� : �����ϴ�.\n");
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
	printf("# ������ 1, ������ 2, ���� 3 �߿��� �����ϼ��� : ");
	scanf("%d", &user);

	while (1) {
		while (getchar() != '\n') {//���� �� ���� �˻�
			myflush();
			printf("# ������ 1, ������ 2, ���� 3 �߿��� �ٽ� �����ϼ��� : ");
			scanf("%d", &user);
		}
		if (user<1 || user>3) { //���� ���� �˻�
			printf("# ������ 1, ������ 2, ���� 3 �߿��� �ٽ� �����ϼ��� : ");
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
		com = random(3) + 1; //0~2������ ���� +1
		repeat = output(user, com, win, draw); //�̱�ų� ���� 1 ����, ���� 0���� -> �ݺ� ����
	}
	printf("���Ӱ�� : %d�� %d��", *win, *draw);

	return 0;
}