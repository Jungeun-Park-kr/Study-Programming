#include <stdio.h>
#pragma warning(disable:4996)

int input();
int calculate(int);
void output(int,int);

int input() {
	int depth;
	printf("* 우물의 깊이를 입력하시오(cm단위) : ");
	scanf("%d", &depth);
	return depth;
}
int calculate(int depth) {
	int day=0;
	if (depth <= 50) {
		return day;
	}
	else { 
		day++;
		while (1) {
			depth -= 50;
			if (depth <= 0) {
				break;
			}
			depth += 20;
			day++;
		}
		return day;
		
	}
}

void output(int depth, int day) {
	double output_depth = depth*0.01;
	printf("%.2lf 미터 깊이의 우물을 탈출하기 위해서는 %d일이 걸립니다.", output_depth, day);
	return;
}

int main() {
	int depth, day;
	depth = input();
	day= calculate(depth);
	output(depth, day);

	return 0;
}