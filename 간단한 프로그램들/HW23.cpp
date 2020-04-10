#include<stdio.h>
#include<string.h>
#pragma warning(disable:4996)

int main() {
	int num1 = 0, num2 = 0, number = 0;
	
	while (1) {
		printf("# 두 개의 정수를 입력하세요 : ");
		number = scanf("%d %d", &num1, &num2);
		
		char c = getchar();
		while (c != '\n' && c != '\t' && c!=' ') { //입력 문자 중 문자가 있을 때 종료
			return 0;
		}

		if (1) {
			if (num1 > num2) {
				printf("%d - %d = %d\n", num1, num2, num1 - num2);
			}
			else {
				printf("%d - %d = %d\n", num2, num1, num2 - num1);
			}
		}
		else {
			break;
		}
	}

	return 0;
}