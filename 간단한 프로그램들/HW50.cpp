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
		if (index != -1) { //���� �˻� �� �ٷ� ����
			return index;
		}
	}
	//�������� ���� ��� index �ʱⰪ -1 ����
	return index;
}
int input(char *str, char *ch) {
	printf("# ���ڿ��� �Է��Ͻÿ� : ");
	scanf("%s", str);
	if (strcmp(str, "end") == 0) {
		return -1;
	}
	printf("# ���ڸ� �Է��Ͻÿ� : ");
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
			printf("\"%s\"���ڿ� �ȿ� '%c' ���ڴ� �������� �ʽ��ϴ�.\n", str, ch);
		}
		else {
			printf("\"%s\"���ڿ� �ȿ� '%c' ���ڴ� %d�� ��ġ�� �����մϴ�.\n", str, ch, index);
		}
	}
	return 0;
}