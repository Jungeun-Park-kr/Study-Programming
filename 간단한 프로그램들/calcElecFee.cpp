<<<<<<< HEAD
//#include <stdio.h>
//#pragma warning(disable:4996)
//
//int inputUsage();
//void result(double);
//
//int main() {
//	int usage = inputUsage();
//
//	int basic_rate = 660;
//	double rate = 88.5;
//	double total = basic_rate + (usage*rate);
//	double tax = total * 0.09;
//	total += tax;
//
//	result(total);
//
//	return 0;
//}
//
//int inputUsage() {
//	int usage;
//	printf("전기 사용량을 입력하세요(kw) : ");
//	scanf("%d", &usage);
//	return usage;
//}
//
//void result(double total) {
//	printf("전기 사용요금은 %lf 원 입니다.\n", total);
//	return;
//}
=======
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

int inputUsage();
void result(double);

int main() {
	int usage = inputUsage();

	int basic_rate = 660;
	double rate = 88.5;
	double total = basic_rate + (usage*rate);
	double tax = total * 0.09;
	total += tax;

	result(total);

	return 0;
}

int inputUsage() {
	int usage;
	printf("전기 사용량을 입력하세요(kw) : ");
	scanf("%d", &usage);
	return usage;
}

void result(double total) {
	printf("전기 사용요금은 %lf 원 입니다.\n", total);
	return;
}
>>>>>>> d4bdd6f0e471933bf3d6458054c5f18e222802e5
