#include<stdio.h>
#pragma warning(disable:4996)

int input();

int input() {
	int num;
	printf("�������� �Է��ϼ��� : ");
	scanf("%d", &num);
	return num;
}

int main() {
	int number;
	number = input();
	
	printf("�Էµ� ���� 8 ������ %#o �Դϴ�.\n", number);
	printf("�Էµ� ���� 16 ������ %#x �Դϴ�.", number);

	return 0;
}