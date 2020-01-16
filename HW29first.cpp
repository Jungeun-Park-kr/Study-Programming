//#include <stdio.h>
//#pragma warning(disable:4996)
//
//int transNumber(int num);
//void inputUInt(int *first, int *last, int *gozip);
//void output();
//void myflush();
//
//void myflush() {
//	while (getchar() != '\n') {
//		;
//	}
//}
//
//void inputUInt(int *first, int *last, int *gozip) {
//	int num, num2, gozip_num;
//	printf("시작 값(P1) : ");
//	scanf("%d", &num);
//	while (1) {
//		while (getchar() != '\n') {//숫자 외 문자 검사
//			myflush();
//			printf("시작 값(P1) : ");
//			scanf("%d", num);
//		}
//		if (num< 100 || num >10000) { //숫자 범위 검사
//			printf("시작 값(P1) : ");
//			scanf("%d", num);
//		}
//		else {
//			*first = num;
//			break;
//		}
//		*first = num;
//	}
//	
//	printf("끝 값(P2) : ");
//	scanf("%d", &num2);
//	while (1) {
//		while (getchar() != '\n') {//숫자 외 문자 검사
//			myflush();
//			printf("끝 값(P2) : ");
//			scanf("%d", num2);
//		}
//		if (num2 < num || num2>10000) { //숫자 범위 검사
//			printf("끝 값(P2) : ");
//			scanf("%d", num2);
//		}
//		else {
//			*last = num2;
//			break;
//		}
//		*last = num2;
//	}
//
//	printf("고집수(N) : ");
//	scanf("%d", &gozip_num);
//	while (1) {
//		while (getchar() != '\n') {//숫자 외 문자 검사
//			myflush();
//			printf("고집수(N) : ");
//			scanf("%d", gozip_num);
//		}
//		if (gozip_num < 1 || gozip_num>10) { //숫자 범위 검사
//			printf("고집수(N) : ");
//			scanf("%d", gozip_num);
//		}
//		else {
//			*gozip = gozip_num;
//			break;
//		}
//		*gozip = gozip_num;
//	}
//
//	return;
//}
//
////int transNumber(int num, int*result) {
//int transNumber(int num) {
//	int number = num;
//	int d1, d2, d3, d4; //각각 1, 10, 100, 1000의 자리 수
//	//고집 수 계산 후 리턴 (최소 3자리(100), 최대 5자리(10000)
//	/* 고집수 계산 : 각 자리 수 분할하여 값 저장
//	 * 각 자리수 곱하면 카운트 증가 후 1의 자리 될 때 까지 반복
//	 * 10미만이면 카운트를 리턴
//	 */
//	int count = 0;
//	while (number > 10) {
//		//printf("현재 number : %d\n", number);
//		if (number < 100) { //2자리 수 (계산 도중에 필요)
//			d2 = number / 10;
//			int tmp = number - d2 * 10;
//			d1 = tmp;
//
//			number = d2 * d1;
//			//printf("2자리수 num:%d :d1:%d d2:%d\n", number, d1, d2);
//			++count;
//		}
//		else if (100 <= number && number < 1000) { //3자리 수
//			d3 = number / 100;
//			int tmp = number - d3 * 100;
//			d2 = tmp / 10;
//			tmp = tmp - d2 * 10;
//			d1 = tmp;
//
//			number = d3 * d2 * d1;
//			//printf(" 3자리수 num:%d d1:%d d2:%d d3:%d count:%d\n", number, d1, d2, d3,count);
//			++count;
//		}
//		else if (1000 <= number && number < 10000) { //4자리 수
//			d4 = number / 1000;
//			int tmp = number - d4 * 1000;
//			d3 = tmp / 100;
//			tmp = tmp - d3 * 100;
//			d2 = tmp / 10;
//			tmp = tmp - d2 * 10;
//			d1 = tmp;
//
//			number = d4 * d3 * d2 * d1;
//			++count;
//			//printf(" 4자리수 num:%d d1:%d d2:%d d3:%d d4:%dcount:%d\n", number, d1, d2, d3,d4, count);
//
//		}
//		else if(number==10000){ //(10000밖에 없음)
//			//*result = num;
//			return 1; //count가1
//		}
//		else { ; }
//	}
//	//*result = num;
//	return count;
//}
//
//
//int main() {
//	int P1 = 0, P2 = 0, gozip_number=0, cal_result=0;
//	int *first = &P1;
//	int *last = &P2;
//	int *gozip = &gozip_number;
//	int cal_gozip = 0, count=0;
//	int *result = &cal_result;
//
//	inputUInt(first, last, gozip);
//	printf("고집수가 %d인 숫자 출력\n", *gozip);
//
//	//printf("while문전----P1 : %d P2 : %d ---\n\n", P1, P2);
//
//	while (P1 <= P2) { //반복횟수
//		//cal_gozip = transNumber(P1, result);
//		cal_gozip = transNumber(P1);
//		//printf("----P1 : %d P2 : %d, result : %d---\n\n", P1, P2, cal_gozip);
//		if (cal_gozip == gozip_number) { //계산 한 고집수의 값이 입력한 고집수와 같을 때
//			//printf("%d\n", *result);
//			printf("%d\n", P1);
//			++P1;
//			++count;
//		}
//		else {
//			++P1;
//		}
//	}
//	printf("총 개수 : %d개", count);
//
//
//	return 0;
//}