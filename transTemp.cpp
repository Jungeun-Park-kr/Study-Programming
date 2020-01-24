/*
 * 작성 날짜 : 20년 1월 1일
 * 프로그램 이름 : transTemp (온도 변환하기)
 * 프로그램 설명 : 키보드로 화씨 온도 입력받고 섭씨 온도를 계산하여 출력
 * 프로그램 조건 
	- 섭씨온도는 소수점 이하 첫째 자리까지 출력
	- 식 : C=5/9(F-32) (C:섭씨, F:화씨)
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