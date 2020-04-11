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
	printf("�ٹ��� : %d�� / �� ��ȭ �� %d ��", day, coin);
}

int main() {
	int day;

	while (1) {
		printf("* ����� �ٹ��ϼ��� �Է��Ͻÿ� : ");
		scanf("%d", &day);
		while (getchar() != '\n') { //�������Է�
			myflush();
			printf("* ����� �ٹ��ϼ��� �Է��Ͻÿ� : ");
			scanf("%d", &day);
		}
		if (day < 0 ) { //�������Է�
			printf("* ����� �ٹ��ϼ��� �Է��Ͻÿ� : ");
			scanf("%d", &day);
		}
		else {
			break;
		}
	}
	
	output(day);

	return 0;
}