#include <stdio.h>
#pragma warning(disable:4996)

int inputAge();
int inputNumber();
void printResult(int,int,int);

int inputAge() {
	int age = 0;
	printf("���尴�� ���̸� �Է��Ͻÿ� : ");
	scanf("%d", &age);
	return age;
}
int inputNumber() {
	int number = 0;
	printf("���尴�� ���� �Է��Ͻÿ� : ");
	scanf("%d", &number);
	return number;
}
void printResult(int cost, int discount, int total) {
	printf("����� => %d��\n",cost);
	printf("���αݾ� => %d��\n",discount);
	printf("�����ݾ� => %d��\n",total);
}

int main() {

	int age = inputAge();
	int number = inputNumber();
	//printf("%d, %d", age, number);
	int cost = 0;
	int discount = 0;
	int total = 0;
	//5�� �̻� : ����
	if (number >= 5) {
		if (age < 14) { //14�� �̸�
			if (age <= 7) { //7�� ���� : 500��
				discount = (500 * number)*0.1;
				cost = 500 * number;
			}
			else { //8�� ~ 13�� : 700��
				discount = 700 *number *0.1;
				cost = 700 * number;
			}
		}
		else { //14�� �̻�
			if (age <= 19) { //14�� ~ 19�� : 1000��
				discount = 1000 * number *0.1;
				cost = 1000 * number;
			}
			else if (age <= 55) { //20�� ~ 55�� : 1500��
				discount = 1500 * number *0.1;
				cost = 1500 * number;
			}
			else { //56�� �̻� : 500��
				discount = 500 * number *0.1;
				cost = 500 * number;
			}
		}
		total = cost - discount;
	}
	else { //5�� �̸� : ����X
		if (age < 14) { //14�� �̸�
			if (age <= 7) {
				cost = 500 * number;
			}
			else { //8�� ~ 13��
				cost = 700 * number;
			}
		}
		else { //14�� �̻�
			if (age <= 19) { //14�� ~ 19��
				cost = 1000 * number;
			}
			else if (age <= 55) { //20�� ~ 55��
				cost = 1500 * number;
			}
			else { //56�� �̻�
				cost = 500 * number;
			}
		}
		total = cost;
	}
	printResult(cost, discount, total);

	return 0;
}