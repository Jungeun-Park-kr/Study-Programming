#include<stdio.h>
#pragma warning(disable:4996)


int main() {
	//double height[5];
	double sum = 0;
	double height=0;

	int i;
	/*for (i = 0; i < 5; i++) {
		printf("%d�� �л��� Ű��? ",i+1);
		scanf("%lf", &height[i]);
		sum += height[i];
	}*/
	for (i = 1; i < 6; i++) {
		printf("%d�� �л��� Ű��? ", i);
		scanf("%lf", &height);
		sum += height;
	}

	printf("�ټ� ���� ��� Ű�� %.1lfcm �Դϴ�.", sum / 5.0);
	return 0;
}