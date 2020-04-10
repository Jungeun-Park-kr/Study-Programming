#include<stdio.h>
#include<string.h>
#pragma warning(disable:4996)

int input(char *);
int calcNum(char *);
void output(char *,int);

int input(char *ary) {
	printf(" # 문장을 입력하시오 : ");
	scanf("%s", ary);
	if (strcmp(ary, "end") == 0) {
		return -1;
	}
	else {
		return 0;
	}
}


void output(char *ary, int result) {
	printf("\"%s\" 내의 총 숫자는 [%d]입니다.\n\n", ary, result);
}


int calcNum(char *ary) {
	int result = 0, tmp = 0;
	int size = strlen(ary);

	for (int i = 0; i < size; i++) {
		if ('0' <= ary[i] && ary[i] <= '9') {
			tmp = tmp * 10 + ary[i] - '0';
		}
		else {
			if (tmp > 0) {
				result += tmp;
				tmp = 0;
			}
		}
	}
	if (tmp > 0) {
		result += tmp;
	}
	
	return result;
}
int main() {
	char ary[100];
	int num = 0, repeat;
	while (1) {
		repeat=input(ary);
		if (repeat == -1) {
			break;
		}
		num = calcNum(ary);
		output(ary, num);
	}
	return 0;
}