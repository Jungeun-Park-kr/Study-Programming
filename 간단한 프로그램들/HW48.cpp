#include <stdio.h>
#pragma warning(disable:4996)

int main() {
	int ary[] = { 2,8,15,1,8,10,5,19,19,3,5,6,6,2,8,2,12,16,3,8,17,
				 12,5,3,14,13,3,2,17,19,16,8,7,12,19,10,13,8,20,
				 16,15,4,12,3,14,14,5,2,12,14,9,8,5,3,18,18,20,4 };
	int num[20] = { 0, };
	int i, j;
	int size = sizeof(ary) / sizeof(ary[0]);
	for (i = 0; i < 20; i++) { //num배열에 개수 카운트하는 for loop
		for (j = 0; j < size; j++) { //맞는 숫자 있는지 확인하는 for loop
			if (i+1 == ary[j]) {
				num[i]++;
			}

		}
	}
	for (i = 0; i < 20; i++) {
		printf("%2d - %2d 개\n", i + 1, num[i]);
	}

	return 0;
}
