#include<stdio.h>
#pragma warning(disable:4996)

int main() {
	int history, literature, art, total=0;
	printf("����, ����, ���� ������ �Է��ϼ��� : ");
	scanf("%d %d %d", &history, &literature, &art);
	total = history + literature + art;
	printf("������ %d �̰� ����� %.2lf �Դϴ�.", total, total / 3.0);

	return 0;
}