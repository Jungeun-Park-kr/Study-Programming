#include <stdio.h>
#pragma warning(disable:4996)

void input(int *, int *);
void myflush();
int calcFee(int *, int *);
void output(int*, int *, int);

void output(int *code, int *usage, int charge) {
	if (*code == 1) {
		printf("\n# 사용자코드 : %d(가정용)\n", *code);
	}
	else if (*code == 2) {
		printf("\n# 사용자코드 : %d(상업용)\n", *code);
	}
	else {
		printf("\n# 사용자코드 : %d(공업용)\n", *code);
	}

	printf("# 사용량 : %d ton\n", *usage);
	printf("# 총수도요금 : %d원\n", charge);
}

int calcFee(int *code, int *usage) {
	int charge = 0, tax=0;
	if (*code == 1) { //가정용(톤당 50원)
		charge = 50 * *usage;
		tax = charge * 0.05;
		charge += tax;
	}
	else if (*code == 2) { //상업용 (톤당 45원)
		charge = 45 * *usage;
		tax = charge * 0.05;
		charge += tax;
	}
	else { //공업용 (톤당 30원)
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
		printf("* 사용자 코드를 입력하시오(1:가정용/2:상업용/3:공업용) : ");
		scanf("%d", code);
		while (getchar() != '\n') {
			myflush();
			printf("* 사용자 코드를 입력하시오(1:가정용/2:상업용/3:공업용) : ");
			scanf("%d", code);
		}
		if (*code < 0 || *code>3) {
			printf("* 사용자 코드를 입력하시오(1:가정용/2:상업용/3:공업용) : ");
			scanf("%d", code);
		}
		else {
			break;
		}
	}
	while (1) {
		printf("* 사용량을 입력하시오(ton단위) : ");
		scanf("%d", usage);
		while (getchar() != '\n') { //문자재입력
			myflush();
			printf("* 사용량을 입력하시오(ton단위) : ");
			scanf("%d", usage);
		}
		if (*code < 0 ) { //음수재입력
			printf("* 사용량을 입력하시오(ton단위) : ");
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