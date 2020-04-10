#include<stdio.h>
#include<string.h>
#pragma warning(disable:4996)

int input(char *, char*);
int strcheck(char *, char*);

int strcheck(char *str, char *ch) {
	int index=-1, size;
	size = sizeof(str)/sizeof(str[0]);
	for (int i = 0; i < size; i++) {
		if (str[i] == *ch) {
			index=i;
		}
		if (index != -1) { //단일 검색 후 바로 리턴
			return index;
		}
	}
	//존재하지 않을 경우 index 초기값 -1 리턴
	return index;
}
int input(char *str, char *ch) {
	printf("# 문자열을 입력하시오 : ");
	scanf("%s", str);
	if (strcmp(str, "end") == 0) {
		return -1;
	}
	printf("# 문자를 입력하시오 : ");
	scanf(" %c", ch);
	return 0;
}
int main() {
	char str[100], ch, index, repeat;

	while (1) {
		repeat = input(str, &ch);
		if (repeat == -1) {
			return 0;
		}
		index = strcheck(str, &ch);
		if (index == -1) {
			printf("\"%s\"문자열 안에 '%c' 문자는 존재하지 않습니다.\n", str, ch);
		}
		else {
			printf("\"%s\"문자열 안에 '%c' 문자는 %d번 위치에 존재합니다.\n", str, ch, index);
		}
	}
	return 0;
}