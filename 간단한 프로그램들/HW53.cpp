#include<stdio.h>
#include<string.h>
#include<ctype.h>
#pragma warning(disable:4996)

int palindrome(char *, int);
int input(char*);
void output(char *, int);
int isUpper(char);

int isUpper(char ch) {
	int result = 0;
	//�빮������ 1 ����
	(65 <= ch && ch <= 90) ? result = 1 : result = 0;
	return result;
}

void output(char *word,int result) {
	if (result==1) { //1 : ȸ��
		printf("\"%s\" : ȸ���Դϴ�!\n", word);
	}
	else {
		printf("\"%s\" : ȸ���� �ƴմϴ�!\n", word);
	}
}
int palindrome(char *word, int size) {

	for (int i = 0; i < size/2; i++) {
		if (word[i] != word[size - i-1]) {
			return -1; //ȸ�� �ƴҽ� -1����
		}
		else {
			return 1; //ȸ�� : 1 ����
		}
	}
}

int input(char *word, char*word_copy) {
	int size = 0, result = 0;

	printf("# �ܾ� �Է� : ");
	scanf("%s", word);
	if (strcmp(word, "end")==0) {
		return -1;
	}
	else {
		//��� �ҹ��ڷ� ���� �� ����
		strcpy(word_copy, word);
		size=strlen(word);
		for (int i = 0; i < size; i++) {
			if (isUpper(word[i])) {
				word_copy[i] = tolower(word[i]);
			}
		}
		return size;
	}
}
int main() {
	char word[30], word_copy[30];;
	int result = 0, size=0, ispalindrome=0;

	while (1) {
		size = input(word,word_copy);

		if (size == -1) {
			//"end" �Է½ñ��� �ݺ�
			return 0;
		}
		else {
			ispalindrome = palindrome(word_copy, size); //ȸ���̸� 1����
		}
		output(word, ispalindrome);
}

	return 0;
}