#include <stdio.h>
#pragma warning(disable:4996)

int inputUInt(const char *);
double inputDouble(const char *);
int ipow(int, int);
double fpow(double, int);
void myflush();

int ipow(int base, int exponent) {
	int result = 1;

	for (int i = 0; i < exponent; i++) {
		result *= base;
	}

	return result;
}

double fpow(double base, int exponent) {
	double result = 1;

	for (int i = 0; i < exponent; i++) {
		result *= base;
	}

	return result;
}


void myflush() {
	while (getchar() != '\n') {
		;
	}
}

int inputUInt(const char *msg) {
	int num;
	printf(msg);
	scanf("%d", &num);

	while (1) {
		while (getchar() != '\n') {
			myflush();
			printf(msg);
			scanf("%d", &num);
		}
		if (num < 0) {
			printf(msg);
			scanf("%d", &num);
		}
		else {
			break;
		}
	}
	return num;
}

double inputDouble(const char *msg) {
	double num;
	printf(msg);
	scanf("%lf", &num);

	while (1) {
		while (getchar() != '\n') {
			myflush();
			printf(msg);
			scanf("%lf", &num);
		}
		if (num < 0) {
			printf(msg);
			scanf("%lf", &num);
		}
		else {
			break;
		}
	}
	return num;
}


int main() {

	int int_base, int_exponent, int_result, double_exponent;
	double double_base, double_result;
	int_base = inputUInt("* 양의 정수 밑을 입력 하시오 : ");
	int_exponent = inputUInt("* 양의 승을 입력 하시오 : ");
	int_result = ipow(int_base, int_exponent);
	printf("%d의 %d승은 %d입니다.\n\n", int_base, int_exponent, int_result);

	double_base = inputDouble("* 양의 실수 밑을 입력 하시오 :");
	double_exponent = inputUInt("* 양의 승을 입력 하시오 : ");
	double_result = fpow(double_base, double_exponent);
	printf("%.2lf의 %d승은 %.3lf입니다.\n", double_base, double_exponent, double_result);


	return 0;
}