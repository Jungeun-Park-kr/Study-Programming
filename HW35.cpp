#include <stdio.h>
#pragma warning(disable:4996)

int main() {
	int num, sum=0;
	int i;
	for (i = 0; i < 5; i++) {
		printf("0���� ū ���� �Է��Ͻÿ�(%d ��°) : ", i + 1);
		scanf("%d", &num);
		if (num <= 0) {
			i--;
		}
		else {
			sum += num;
		}
	}
	printf("�Էµ� ���� �� �� : %d", sum);

	return 0;
}