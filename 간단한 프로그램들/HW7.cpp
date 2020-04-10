#include <stdio.h>
#pragma warning(disable:4996)

int main() {
	double distance, speed, sec=0, time=0;
	int hour=0, min=0;
	printf("* 거리를 입력하시오(km단위) : ");
	scanf("%lf", &distance);
	printf("* 시속을 입력하싱로(km/h단위) : ");
	scanf("%lf", &speed);
	time = distance / speed;
	hour = (int)time;
	double tmp = (time - (double)hour)*60;
	min = (int)tmp;
	tmp -= (double)min;
	sec = tmp * 60;

	printf("%.2lf km => %d시간 %d분 %.3lf 초 소요됨", distance, hour, min, sec);
	return 0;
}