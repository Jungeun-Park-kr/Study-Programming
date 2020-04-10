#include <stdio.h>
#pragma warning(disable:4996)

int inputAge();
int inputNumber();
void printResult(int,int,int);

int inputAge() {
	int age = 0;
	printf("입장객의 나이를 입력하시오 : ");
	scanf("%d", &age);
	return age;
}
int inputNumber() {
	int number = 0;
	printf("입장객의 수를 입력하시오 : ");
	scanf("%d", &number);
	return number;
}
void printResult(int cost, int discount, int total) {
	printf("입장료 => %d원\n",cost);
	printf("할인금액 => %d원\n",discount);
	printf("결제금액 => %d원\n",total);
}

int main() {

	int age = inputAge();
	int number = inputNumber();
	//printf("%d, %d", age, number);
	int cost = 0;
	int discount = 0;
	int total = 0;
	//5명 이상 : 할인
	if (number >= 5) {
		if (age < 14) { //14세 미만
			if (age <= 7) { //7세 이하 : 500원
				discount = (500 * number)*0.1;
				cost = 500 * number;
			}
			else { //8세 ~ 13세 : 700원
				discount = 700 *number *0.1;
				cost = 700 * number;
			}
		}
		else { //14세 이상
			if (age <= 19) { //14세 ~ 19세 : 1000원
				discount = 1000 * number *0.1;
				cost = 1000 * number;
			}
			else if (age <= 55) { //20세 ~ 55세 : 1500원
				discount = 1500 * number *0.1;
				cost = 1500 * number;
			}
			else { //56세 이상 : 500원
				discount = 500 * number *0.1;
				cost = 500 * number;
			}
		}
		total = cost - discount;
	}
	else { //5명 미만 : 할인X
		if (age < 14) { //14세 미만
			if (age <= 7) {
				cost = 500 * number;
			}
			else { //8세 ~ 13세
				cost = 700 * number;
			}
		}
		else { //14세 이상
			if (age <= 19) { //14세 ~ 19세
				cost = 1000 * number;
			}
			else if (age <= 55) { //20세 ~ 55세
				cost = 1500 * number;
			}
			else { //56세 이상
				cost = 500 * number;
			}
		}
		total = cost;
	}
	printResult(cost, discount, total);

	return 0;
}