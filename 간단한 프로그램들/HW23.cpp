#include<stdio.h>
#include<string.h>
#pragma warning(disable:4996)

int main() {
	int num1 = 0, num2 = 0, number = 0;
	
	while (1) {
		printf("# �� ���� ������ �Է��ϼ��� : ");
		number = scanf("%d %d", &num1, &num2);
		
		char c = getchar();
		while (c != '\n' && c != '\t' && c!=' ') { //�Է� ���� �� ���ڰ� ���� �� ����
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