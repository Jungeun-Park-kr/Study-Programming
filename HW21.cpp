#include<stdio.h>
#pragma warning(disable:4996)


int main() {
	//double height[5];
	double sum = 0;
	double height=0;

	int i;
	/*for (i = 0; i < 5; i++) {
		printf("%d번 학생의 키는? ",i+1);
		scanf("%lf", &height[i]);
		sum += height[i];
	}*/
	for (i = 1; i < 6; i++) {
		printf("%d번 학생의 키는? ", i);
		scanf("%lf", &height);
		sum += height;
	}

	printf("다섯 명의 평균 키는 %.1lfcm 입니다.", sum / 5.0);
	return 0;
}