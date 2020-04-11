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
	//대문자일지 1 리턴
	(65 <= ch && ch <= 90) ? result = 1 : result = 0;
	return result;
}

void output(char *word,int result) {
	if (result==1) { //1 : 회문
		printf("\"%s\" : 회문입니다!\n", word);
	}
	else {
		printf("\"%s\" : 회문이 아닙니다!\n", word);
	}
}
int palindrome(char *word, int size) {

	for (int i = 0; i < size/2; i++) {
		if (word[i] != word[size - i-1]) {
			return -1; //회문 아닐시 -1리턴
		}
		else {
			return 1; //회문 : 1 리턴
		}
	}
}

int input(char *word, char*word_copy) {
	int size = 0, result = 0;

	printf("# 단어 입력 : ");
	scanf("%s", word);
	if (strcmp(word, "end")==0) {
		return -1;
	}
	else {
		//모두 소문자로 통일 후 리턴
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
			//"end" 입력시까지 반복
			return 0;
		}
		else {
			ispalindrome = palindrome(word_copy, size); //회문이면 1리턴
		}
		output(word, ispalindrome);
}

	return 0;
}