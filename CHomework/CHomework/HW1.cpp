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