/*
 * 작성 날짜 : 20년 1월 1일
 * 프로그램 이름 : yearToTime(1 년을 일, 시, 분, 초로 환산하는 프로그램)
 * 프로그램 설명 : 1년이 365.2422일 일 때, 몇 일, 몇 시간, 몇 분, 몇 초인지 출력
			단, 초수는 소수점 이하 둘째 자리까지 출력

*/
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