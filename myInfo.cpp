/*
 * 작성 날짜 : 19년 12월 29일
 * 프로그램 이름 : myInfo(나에 대한 정보 출력)
 * 프로그램 설명 : 이름은 main()에서 출력, 나이와 키는 각각 출력하는 함수를 만듦

*/

#include<stdio.h>
void printAge(int);
void printHeight(double);

void printAge(int age) {
	printf("나이 : %d세\n",age);
}
void printHeight(double height) {
	printf("신장 : %lfcm\n",height);
}

int main(void) {
	char name[20] = "박정은";
	int age = 21;
	double height = 163.8;

	printf("성명 : %s\n", name);
	printAge(age);
	printHeight(height);

	return 0;
}