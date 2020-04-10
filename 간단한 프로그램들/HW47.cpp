#include<stdio.h>
#pragma warning(disable:4996)

void input(double *);
double my_max(double *);
double my_min(double *);
void output(double, double);
void myflush();

void myflush() {
	while (getchar() != '\n')
		;
}
double my_max(double *ary) {
	double max = -9999.9999;
	for (int i = 0; i < 5; i++) {
		if (max < ary[i]) {
			max = ary[i];
		}
	}
	return max;
}

double my_min(double *ary) {
	double min = 999999.9999;
	for (int i = 0; i < 5; i++) {
		if (ary[i] < min) {
			min = ary[i];
		}
	}
	return min;
}

void output(double max, double min) {
	printf("가장 큰 값 : %.2lf\n", max);
	printf("가장 작은 값 : %.2lf", min);

}
void input(double *ary) {
	int i;
	for (i = 0; i < 5; i++) {
		printf("%d번 방 값 : ",i);
		scanf("%lf", &ary[i]);

		while (getchar() != '\n') {
			myflush();
			printf("%d번 방 값 : ", i);
			scanf("%lf", &ary[i]);
		}
	}
	
	return;
}
int main() {
	double  max, min;
	//double *ary = input();
	double ary[5];
	input(ary);

	max = my_max(ary);
	min = my_min(ary);
	output(max, min);

	return 0;
}