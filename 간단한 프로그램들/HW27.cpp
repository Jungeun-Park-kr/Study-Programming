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
	printf("# ����� ���Ը� �Է��ϼ���(���� : g) : ");
	scanf("%d", &weight);

	while (getchar() != '\n') {//���� �� ���� �˻�
		myflush();
		printf("# ���ڸ� �ٽ� �Է��Ͻÿ� : ");
		scanf("%d", &weight);
	}

	return weight;
}
int main() {
	int weight, packing=0;
	
	while (packing < 10) {
		weight = input();

		if (weight < 150) {
			printf("* ���߸��� ������ �峭���� ���ÿ�~ ^^\n");
		}
		else if (weight > 500) {
			printf("* Ÿ���� ������ �峭���� ���ÿ�~ ^^\n");
		}
		else {
			printf("* ���� �ް��� �� : %d\n", ++packing);
		}
	}
	printf("\n\n*** �ް� ������ �������ϴ�.");

	return 0;
}