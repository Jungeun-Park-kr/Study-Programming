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

int inputUsage();
void result(double);

int main() {
	int usage = inputUsage();

	int basic_rate = 660;
	double rate = 88.5;
	double total = basic_rate + (usage*rate);
	double tax = total * 0.09;
	total += tax;

	result(total);

	return 0;
}

int inputUsage() {
	int usage;
	printf("���� ��뷮�� �Է��ϼ���(kw) : ");
	scanf("%d", &usage);
	return usage;
}

void result(double total) {
	printf("���� ������� %lf �� �Դϴ�.\n", total);
	return;
}