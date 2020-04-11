#include <stdio.h>
#include<time.h>
#include<stdlib.h>
#pragma warning(disable:4996)

void randAry(int (*)[4]);
void output(int(*)[4]);

void output(int(*ary)[4]) {
	int column[4] = { 0 }, row[3] = { 0 };
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 4; j++) {
			row[i] += ary[i][j];
		}
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			column[i] += ary[j][i];
		}
	}

	for (int i = 0; i < 3; i++) {
		printf("%d행     :", i);
		for (int j = 0; j < 4; j++) {
			printf("%5d", ary[i][j]);
		}
		printf("    %d행의 합 : %d\n", i,row[i]);
	}
	printf("열의 합 :");
	for (int i = 0; i < 4; i++) {
		printf("%5d", column[i]);
	}

}

void randAry(int (*ary)[4]) {
	int num=0;
	
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 4; j++) {
			ary[i][j] = (rand() % 9) + 1;
		}
	}

}


int main() {
	int ary[3][4];
	srand((unsigned int)time(NULL));
	randAry(ary);
	output(ary);

	return 0;
}