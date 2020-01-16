#include <stdio.h>
#pragma warning(disable:4996)

int input(const char*, int *);
void sort(int *, int);
void output(int *, int);

void output(int *ary, int size) {
	printf("# 소트 후 데이터 : ");
	for (int i = 0; i < size-1; i++) {
		printf("%d ", ary[i]);
	}
	printf("\n");
}

void sort(int *ary, int size) {
	int tmp=0;
	int i, j;
	for (i = 0; i < size-1; i++) { //0빼고 정렬
		for (j = i+1; j < size-1; j++) {
			if (ary[i] > ary[j]) {
				tmp = ary[i];
				ary[i] = ary[j];
				ary[j] = tmp;
			}
		}
	}
}

int input(const char *msg, int *ary){
	int count = 0;
	printf(msg);
	int i = 0;
	while (1) {
		scanf("%d", &ary[i]);
		++count;
		if (ary[i] == 0)
			break;
		++i;
	}
	return count;
}

int main() {
	int count=0, size=0;
	int ary1[101], ary2[101], ary3[101];

	count=input("# 소트 전 데이터(1차) :", ary1);
	size = sizeof(ary1) / sizeof(ary1[0]);
	sort(ary1, count);
	output(ary1, count);

	count = input("# 소트 전 데이터(2차) :", ary2);
	size = sizeof(ary2) / sizeof(ary2[0]);
	sort(ary2, count);
	output(ary2, count);

	count = input("# 소트 전 데이터(3차) :", ary3);
	size = sizeof(ary3) / sizeof(ary3[0]);
	sort(ary3, count);
	output(ary3, count);

	return 0;
}