#include<stdio.h>

int main() {
	unsigned time = 54321;
	int left_time = 0;

	int hour = time / (60 * 60);
	left_time = time - hour*(60*60);
	
	int minute = left_time / 60;
	left_time -= minute * 60;

	int second = left_time;

	printf("%u�ʴ� %d�ð� %d�� %d���Դϴ�.\n", time, hour, minute, second);
	return 0;
}