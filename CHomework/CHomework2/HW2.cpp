#include <stdio.h>
void print_result(int, int, int, int, int, int, int, int, int);

void print_result(int money, int m50000, int m10000, int m5000, int m1000, int m500, int m100, int m50, int m10) {
	printf("<<ȯ���� �ݾ� : %d��>>\n", money);
	printf("50000�� �� => %d��\n", m50000);
	printf("10000�� �� => %d��\n", m10000);
	printf("5000�� �� => %d��\n", m5000);
	printf("1000�� �� => %d��\n", m1000);
	printf("500�� �� => %d��\n", m500);
	printf("100�� �� => %d��\n", m100);
	printf("50�� �� => %d��\n", m50);
	printf("10�� �� => %d��\n", m10);
}

int main() {
	int money = 278970;
	int left_money = 0;

	int m50000 = money / 50000;
	left_money = money - m50000*50000;

	int m10000 = left_money / 10000;
	left_money -= m10000*10000;

	int m5000 = left_money / 5000;
	left_money-=m5000*5000;

	int m1000 = left_money / 1000;
	left_money-=m1000*1000;

	int m500 = left_money / 500;
	left_money -= m500*500;

	int m100 = left_money / 100;
	left_money -= m100*100;

	int m50 = left_money / 50;
	left_money -=m50*50;

	int m10 = left_money / 10;
	

	print_result(money, m50000, m10000, m5000, m1000, m500, m100, m50, m10);
	
	return 0;
	
}
