#include<stdio.h>
#pragma warning(disable:4996)

int input(const char *);
int totalPay(int);
int totalTax(int);
void output(int,int);

int input(const char *msg) {
	int time;
	printf(msg);
	scanf("%d", &time);
	return time;
}
int totalPay(int time) {
	int total_pay = 0;
	int overtime = 0;
	if (time <=  40) {
		total_pay = 3000 * time;
		return total_pay;
	}
	else { //�ʰ��ٹ� �� ���
		total_pay = 40 * 3000; //�⺻��
		overtime = time - 40;
		total_pay += overtime * 3000 * 1.5;
		return total_pay;
	}
}

int totalTax(int pay) {
	int tax = 0, over_pay = 0;
	if (pay < 100000) {
		tax = 0.15 * pay;
		return tax;
	}
	else {
		tax = 0.15 * 100000; //�� ���� ���� 15%
		over_pay = pay - 100000;
		tax += 0.25 * over_pay; //�ʰ� �ݾ� 25%
		return tax;
	}

}

void output(int pay, int tax) {
	printf("# �Ѽ��� : %7d��\n", pay);
	printf("# ��  �� : %7d��\n", tax);
	printf("# �Ǽ��� : %7d��", pay - tax);
	return;
}

int main() {
	int time, total_pay = 0,tax =0;
	time = input("*1���ϰ��� �ٹ��ð��� �Է��Ͻÿ� : ");
	total_pay = totalPay(time);
	tax = totalTax(total_pay);
	output(total_pay, tax);
	
	return 0;
}