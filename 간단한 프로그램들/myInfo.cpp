/*
 * �ۼ� ��¥ : 19�� 12�� 29��
 * ���α׷� �̸� : myInfo(���� ���� ���� ���)
 * ���α׷� ���� : �̸��� main()���� ���, ���̿� Ű�� ���� ����ϴ� �Լ��� ����

*/

#include<stdio.h>
void printAge(int);
void printHeight(double);

void printAge(int age) {
	printf("���� : %d��\n",age);
}
void printHeight(double height) {
	printf("���� : %lfcm\n",height);
}

int main(void) {
	char name[20] = "������";
	int age = 21;
	double height = 163.8;

	printf("���� : %s\n", name);
	printAge(age);
	printHeight(height);

	return 0;
}