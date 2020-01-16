#include<stdio.h>
#pragma warning(disable:4996)
void swap(int *, int);


void swap(int *ary,int size) {
	int tmp, i, j;
	for(i=0;i<size;i++){
		for (j = i; j < size; j++) {
			if (ary[i] < ary[j]) {
				tmp = ary[i];
				ary[i] = ary[j];
				ary[j] = tmp;
			}
		}
	}
	printf("바뀐 배열에 저장된 값 :");
	for (int i = 0; i < size; i++) {
		printf("%5d",ary[i]);
	}

	return ;
}

int main() {
	int ary[]= { 1,2,3,4,5};
	int size = sizeof(ary)/sizeof(ary[0]);

	printf("처음 배열에 저장된 값 :");
	for (int i = 0; i < size; i++) {
		printf("%5d",ary[i]);
	}
	printf("\n");

	swap(ary,size);


	return 0;
}