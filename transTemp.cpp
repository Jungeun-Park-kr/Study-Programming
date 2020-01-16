/*
 * 작성 날짜 : 20년 1월 1일
 * 프로그램 이름 : calcElecFee(전기요금 계산하기)
 * 프로그램 설명 :  전기 사용량은 kw단위로 입력한 후 사용 요금 계산해주는 프로그램
 * 프로그램 조건 
	- 기본요금 : 660원
	- kw당 사용요금 : 88.5원
	- 전체요금 = 기본요금 (사용량 * kw당 사용요금)
	- 세금 : 전체 요금의 9%
	- 최종 사용 요금 = 전체 요금 + 세금
*/
#include <stdio.h>
#pragma warning(disable:4996)

int inputTemp();
void result(double);

int main() {
	int fahrenheit = inputTemp();
	double celsius = (double)5 / 9 * (fahrenheit - 32);
	result(celsius);

	return 0;
}

int inputTemp() {
	int fahrenheit;
	printf("화씨 온도를 입력하세요 : ");
	scanf("%d", &fahrenheit);
	return fahrenheit;
}

void result(double celsius) {
	printf("섭씨 온도는 %.1lf 도 입니다.\n", celsius);
	return;
}