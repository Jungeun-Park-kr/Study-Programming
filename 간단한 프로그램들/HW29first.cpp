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
//	printf("���� ��(P1) : ");
//	scanf("%d", &num);
//	while (1) {
//		while (getchar() != '\n') {//���� �� ���� �˻�
//			myflush();
//			printf("���� ��(P1) : ");
//			scanf("%d", num);
//		}
//		if (num< 100 || num >10000) { //���� ���� �˻�
//			printf("���� ��(P1) : ");
//			scanf("%d", num);
//		}
//		else {
//			*first = num;
//			break;
//		}
//		*first = num;
//	}
//	
//	printf("�� ��(P2) : ");
//	scanf("%d", &num2);
//	while (1) {
//		while (getchar() != '\n') {//���� �� ���� �˻�
//			myflush();
//			printf("�� ��(P2) : ");
//			scanf("%d", num2);
//		}
//		if (num2 < num || num2>10000) { //���� ���� �˻�
//			printf("�� ��(P2) : ");
//			scanf("%d", num2);
//		}
//		else {
//			*last = num2;
//			break;
//		}
//		*last = num2;
//	}
//
//	printf("������(N) : ");
//	scanf("%d", &gozip_num);
//	while (1) {
//		while (getchar() != '\n') {//���� �� ���� �˻�
//			myflush();
//			printf("������(N) : ");
//			scanf("%d", gozip_num);
//		}
//		if (gozip_num < 1 || gozip_num>10) { //���� ���� �˻�
//			printf("������(N) : ");
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
//	int d1, d2, d3, d4; //���� 1, 10, 100, 1000�� �ڸ� ��
//	//���� �� ��� �� ���� (�ּ� 3�ڸ�(100), �ִ� 5�ڸ�(10000)
//	/* ������ ��� : �� �ڸ� �� �����Ͽ� �� ����
//	 * �� �ڸ��� ���ϸ� ī��Ʈ ���� �� 1�� �ڸ� �� �� ���� �ݺ�
//	 * 10�̸��̸� ī��Ʈ�� ����
//	 */
//	int count = 0;
//	while (number > 10) {
//		//printf("���� number : %d\n", number);
//		if (number < 100) { //2�ڸ� �� (��� ���߿� �ʿ�)
//			d2 = number / 10;
//			int tmp = number - d2 * 10;
//			d1 = tmp;
//
//			number = d2 * d1;
//			//printf("2�ڸ��� num:%d :d1:%d d2:%d\n", number, d1, d2);
//			++count;
//		}
//		else if (100 <= number && number < 1000) { //3�ڸ� ��
//			d3 = number / 100;
//			int tmp = number - d3 * 100;
//			d2 = tmp / 10;
//			tmp = tmp - d2 * 10;
//			d1 = tmp;
//
//			number = d3 * d2 * d1;
//			//printf(" 3�ڸ��� num:%d d1:%d d2:%d d3:%d count:%d\n", number, d1, d2, d3,count);
//			++count;
//		}
//		else if (1000 <= number && number < 10000) { //4�ڸ� ��
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
//			//printf(" 4�ڸ��� num:%d d1:%d d2:%d d3:%d d4:%dcount:%d\n", number, d1, d2, d3,d4, count);
//
//		}
//		else if(number==10000){ //(10000�ۿ� ����)
//			//*result = num;
//			return 1; //count��1
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
//	printf("�������� %d�� ���� ���\n", *gozip);
//
//	//printf("while����----P1 : %d P2 : %d ---\n\n", P1, P2);
//
//	while (P1 <= P2) { //�ݺ�Ƚ��
//		//cal_gozip = transNumber(P1, result);
//		cal_gozip = transNumber(P1);
//		//printf("----P1 : %d P2 : %d, result : %d---\n\n", P1, P2, cal_gozip);
//		if (cal_gozip == gozip_number) { //��� �� �������� ���� �Է��� �������� ���� ��
//			//printf("%d\n", *result);
//			printf("%d\n", P1);
//			++P1;
//			++count;
//		}
//		else {
//			++P1;
//		}
//	}
//	printf("�� ���� : %d��", count);
//
//
//	return 0;
//}