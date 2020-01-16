#include<stdio.h>

int main() {
	double day = 365.2422;
	int iday, hour, min;
	double sec;
	iday = (int)day;
	double tmp = day - (double)iday;
	//double tmp_hour = tmp;
	hour = tmp * 24;
	double tmp_hour = tmp * 24;
	tmp = tmp_hour - (double)hour; 
	
	min = tmp * 60;
	double tmp_min = tmp * 60;
	tmp = tmp_min - (double)min;

	sec = tmp * 60;

	
	printf("%.4lf일은 %d일 %d시간 %d분 %.2lf초입니다.", day, iday, hour, min, sec);

	return 0;
}