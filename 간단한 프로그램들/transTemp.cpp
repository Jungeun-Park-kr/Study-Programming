/*
 * �ۼ� ��¥ : 20�� 1�� 1��
<<<<<<< HEAD
 * ���α׷� �̸� : calcElecFee(������ ����ϱ�)
 * ���α׷� ���� :  ���� ��뷮�� kw������ �Է��� �� ��� ��� ������ִ� ���α׷�
 * ���α׷� ���� 
	- �⺻��� : 660��
	- kw�� ����� : 88.5��
	- ��ü��� = �⺻��� (��뷮 * kw�� �����)
	- ���� : ��ü ����� 9%
	- ���� ��� ��� = ��ü ��� + ����
=======
 * ���α׷� �̸� : transTemp (�µ� ��ȯ�ϱ�)
 * ���α׷� ���� : Ű����� ȭ�� �µ� �Է¹ް� ���� �µ��� ����Ͽ� ���
 * ���α׷� ���� 
	- �����µ��� �Ҽ��� ���� ù° �ڸ����� ���
	- �� : C=5/9(F-32) (C:����, F:ȭ��)
>>>>>>> d4bdd6f0e471933bf3d6458054c5f18e222802e5
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