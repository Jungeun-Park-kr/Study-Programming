/*
 * �ۼ� ��¥ : 20�� 1�� 1��
 * ���α׷� �̸� : calcElecFee(������ ����ϱ�)
 * ���α׷� ���� :  ���� ��뷮�� kw������ �Է��� �� ��� ��� ������ִ� ���α׷�
 * ���α׷� ���� 
	- �⺻��� : 660��
	- kw�� ����� : 88.5��
	- ��ü��� = �⺻��� (��뷮 * kw�� �����)
	- ���� : ��ü ����� 9%
	- ���� ��� ��� = ��ü ��� + ����
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