#include <stdio.h>
#pragma warning(disable:4996)

int main() {
	double distance, speed, sec=0, time=0;
	int hour=0, min=0;
	printf("* �Ÿ��� �Է��Ͻÿ�(km����) : ");
	scanf("%lf", &distance);
	printf("* �ü��� �Է��Ͻ̷�(km/h����) : ");
	scanf("%lf", &speed);
	time = distance / speed;
	hour = (int)time;
	double tmp = (time - (double)hour)*60;
	min = (int)tmp;
	tmp -= (double)min;
	sec = tmp * 60;

	printf("%.2lf km => %d�ð� %d�� %.3lf �� �ҿ��", distance, hour, min, sec);
	return 0;
}