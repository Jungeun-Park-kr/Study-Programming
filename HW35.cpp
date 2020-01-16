#include <stdio.h>
#pragma warning(disable:4996)

int main() {
	int num, sum=0;
	int i;
	for (i = 0; i < 5; i++) {
		printf("0보다 큰 수를 입력하시오(%d 번째) : ", i + 1);
		scanf("%d", &num);
		if (num <= 0) {
			i--;
		}
		else {
			sum += num;
		}
	}
	printf("입력된 값의 총 합 : %d", sum);

	return 0;
}