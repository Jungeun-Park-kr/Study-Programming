/*
 * �ۼ� ��¥ : 20�� 1�� 1��
 * ���α׷� �̸� : transTemp (�µ� ��ȯ�ϱ�)
 * ���α׷� ���� : Ű����� ȭ�� �µ� �Է¹ް� ���� �µ��� ����Ͽ� ���
 * ���α׷� ���� 
	- �����µ��� �Ҽ��� ���� ù° �ڸ����� ���
	- �� : C=5/9(F-32) (C:����, F:ȭ��)
*/
#include <stdio.h>
#pragma warning(disable:4996)

int inputTemp();
void result(double);

int main() {
	int fahrenheit = inputTemp();
	double celsius = (double)5 / 9 * (fahrenheit - 32);
	result(celsius);

	return 0;
}

int inputTemp() {
	int fahrenheit;
	printf("ȭ�� �µ��� �Է��ϼ��� : ");
	scanf("%d", &fahrenheit);
	return fahrenheit;
}

void result(double celsius) {
	printf("���� �µ��� %.1lf �� �Դϴ�.\n", celsius);
	return;
}