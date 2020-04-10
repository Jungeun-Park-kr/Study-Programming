#include<stdio.h>
#pragma warning(disable:4996)


int inputYMD(int*, int*, int*);
int yearCheck(int);
int isError(int*, int*, int*);
int calcDays(int*, int*, int*);
void output(int *, int*, int*, int);

int isError(int *year, int *month, int *day) {
	int result = 0;
	if (*year < 1900) {
		return -1;
	}
	if (*month < 1 || *month >12) {
		return -1;
	}
	if (*month == 2) {
		if (*day == 29 && !yearCheck(*year)) {
			return -1;
		}
		else { ; }
	}

	return result;
}

void output(int *year, int *month, int *day, int days) {
	printf("  %d년 %d월 %d일은 ", *year, *month, *day);
	switch (days % 7) {
		case 0:
			printf("일요일");
			break;
		case 1:
			printf("월요일");
			break;
		case 2:
			printf("화요일");
			break;
		case 3:
			printf("수요일");
			break;
		case 4:
			printf("목요일");
			break;
		case 5:
			printf("금요일");
			break;
		case 6:
			printf("토요일");
			break;
		default:
			break;
	}
	printf("입니다.\n");
	return;
}

int calcDays(int *year, int *month, int *day) {
	int days = 0;
	for (int i = 1990; i < *year; i++) {
		if (yearCheck(i))//윤년이면 1
			days += 366;
		else //평년
			days += 365;
	}
	for (int i = 1; i < *month; i++) {
		if (i == 1 || i == 3 || i == 5 || i == 7 || i == 8 || i == 10 || i == 12) {
			days += 31;
		}
		else if (i == 2) {
			if (yearCheck(*year)) {
				days += 29;
			}
			else {
				days += 28;
			}
		}
		else {
			days += 30;
		}
	}
	days += (*day);


	return days;
}


int inputYMD(int *year, int *month, int *day) {
	int check=0, repeat=0;

	while (1) {
		printf("* 년 월 일을 입력하시오 : ");
		scanf("%d %d %d", year, month, day);


		while (getchar() != '\n') { //문자입력시 -1리턴 후 종료
			return -1;
		}

		check = isError(year, month, day);
		if (check == 0) {
		//정상입력
			break;
		}
		else { //비정상입력
			;
		}
	}

	return 0;
}
int yearCheck(int year) {
	int result; //윤년 : 1, 평년 : 0 리턴

	if (year % 4 == 0) {
		if (year % 100 == 0) {
			if (year % 400 == 0) {
				return 1;
			}
			else {
				return 0;
			}
		}
		else {
			return 1;
		}
	}
	else {
		return 0;
	}


	return result;
}



int main() {
	int year, month, day, check, days;
	check = inputYMD(&year, &month, &day);

	if (check == -1) //문자 입력시 종료
		return 0;

	days = calcDays(&year, &month, &day);
	output(&year, &month, &day, days);

	return 0;
}