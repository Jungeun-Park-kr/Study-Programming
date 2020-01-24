/*
 * 작성 날짜 : 20년 1월 1일
 * 프로그램 이름 : secondsToTime(초 수를 시, 분, 초 단위로 환산하는 프로그램)
 * 프로그램 설명 : 54321초를 시, 분, 초 단위로 환산하여 출력하는 프로그램

*/
#include<stdio.h>

int main() {
	unsigned time = 54321;
	int left_time = 0;

	int hour = time / (60 * 60);
	left_time = time - hour*(60*60);
	
	int minute = left_time / 60;
	left_time -= minute * 60;

	int second = left_time;

	printf("%u초는 %d시간 %d분 %d초입니다.\n", time, hour, minute, second);
	return 0;
}