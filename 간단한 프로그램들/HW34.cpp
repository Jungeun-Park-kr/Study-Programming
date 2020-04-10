#include <stdio.h>
#pragma warning(disable:4996)

void input(int *, int *);
void myflush();
int calcFee(int *, int *);
void output(int*, int *, int);

void output(int *code, int *usage, int charge) {
	if (*code == 1) {
		printf("\n# ������ڵ� : %d(������)\n", *code);
	}
	else if (*code == 2) {
		printf("\n# ������ڵ� : %d(�����)\n", *code);
	}
	else {
		printf("\n# ������ڵ� : %d(������)\n", *code);
	}

	printf("# ��뷮 : %d ton\n", *usage);
	printf("# �Ѽ������ : %d��\n", charge);
}

int calcFee(int *code, int *usage) {
	int charge = 0, tax=0;
	if (*code == 1) { //������(��� 50��)
		charge = 50 * *usage;
		tax = charge * 0.05;
		charge += tax;
	}
	else if (*code == 2) { //����� (��� 45��)
		charge = 45 * *usage;
		tax = charge * 0.05;
		charge += tax;
	}
	else { //������ (��� 30��)
		charge = 30 * *usage;
		tax = charge * 0.05;
		charge += tax;
	}

	return charge;
}

void myflush() {
	while (getchar() != '\n') {
		;
	}
}

void input(int *code, int *usage) {

	while (1) {
		printf("* ����� �ڵ带 �Է��Ͻÿ�(1:������/2:�����/3:������) : ");
		scanf("%d", code);
		while (getchar() != '\n') {
			myflush();
			printf("* ����� �ڵ带 �Է��Ͻÿ�(1:������/2:�����/3:������) : ");
			scanf("%d", code);
		}
		if (*code < 0 || *code>3) {
			printf("* ����� �ڵ带 �Է��Ͻÿ�(1:������/2:�����/3:������) : ");
			scanf("%d", code);
		}
		else {
			break;
		}
	}
	while (1) {
		printf("* ��뷮�� �Է��Ͻÿ�(ton����) : ");
		scanf("%d", usage);
		while (getchar() != '\n') { //�������Է�
			myflush();
			printf("* ��뷮�� �Է��Ͻÿ�(ton����) : ");
			scanf("%d", usage);
		}
		if (*code < 0 ) { //�������Է�
			printf("* ��뷮�� �Է��Ͻÿ�(ton����) : ");
			scanf("%d", usage);
		}
		else {
			break;
		}
	}

}

int main() {
	int code, usage, charge=0;
	input(&code, &usage);
	charge = calcFee(&code, &usage);
	output(&code, &usage, charge);
	return 0;
}