#include<stdio.h>
#pragma warning(disable:4996)

int main() {
	int history, literature, art, total=0;
	printf("역사, 문학, 예능 점수를 입력하세요 : ");
	scanf("%d %d %d", &history, &literature, &art);
	total = history + literature + art;
	printf("총점은 %d 이고 평균은 %.2lf 입니다.", total, total / 3.0);

	return 0;
}