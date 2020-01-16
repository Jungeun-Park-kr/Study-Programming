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