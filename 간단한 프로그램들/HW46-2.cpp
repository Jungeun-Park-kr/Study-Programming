//#include <stdio.h>
//#pragma warning(disable:4996)
//
//int yearCheck(int);
//int calcDays(int*, int*, int*);
//
//int calcDays(int *year, int *month, int *day) {
//	int days=0;
//	for (int i = 1990; i < *year; i++) {
//		if (yearCheck(i))//�����̸� 1
//			days += 366;
//		else //���
//			days += 365;
//	}
//	for (int i = 1; i < *month; i++) {
//		if (i == 1 || i == 3 || i == 5 || i == 7 || i == 8 || i == 10 || i == 12 ) {
//			days += 31;
//		}
//		else if (i == 2) {
//			if (yearCheck(*year)) {
//				days += 29;
//			}
//			else {
//				days += 28;
//			}
//		}
//		else {
//			days += 30;
//		}
//	}
//	days += (*day);
//
//
//	return days;
//}
//int yearCheck(int year) {
//	int result; //���� : 1, ��� : 0 ����
//
//	if (year % 4 == 0) {
//		if (year % 100 == 0) {
//			if (year % 400 == 0) {
//				return 1;
//			}
//			else {
//				return 0;
//			}
//		}
//		else {
//			return 1;
//		}
//	}
//	else {
//		return 0;
//	}
//
//
//	return result;
//}
//
//int main() {
//	int year, month, day, days;
//	printf("�� �� ���� �Է��ϼ��� : ");
//	scanf("%d %d %d", &year, &month, &day);
//	
//	days = calcDays(&year, &month, &day);
//	switch (days % 7) {
//	case 0:
//		printf("%d�� %d�� %d���� �Ͽ����Դϴ�.", year, month, day );
//		break;
//	case 1:
//		printf("%d�� %d�� %d���� �������Դϴ�.", year, month, day );
//		break;
//	case 2:
//		printf("%d�� %d�� %d���� ȭ�����Դϴ�.", year, month, day);
//		break;
//	case 3:
//		printf("%d�� %d�� %d���� �������Դϴ�.", year, month, day);
//		break;
//	case 4:
//		printf("%d�� %d�� %d���� ������Դϴ�.", year, month, day);
//		break;
//	case 5:
//		printf("%d�� %d�� %d���� �ݿ����Դϴ�.", year, month, day);
//		break;
//	case 6:
//		printf("%d�� %d�� %d���� ������Դϴ�.", year, month, day);
//		break;
//	}
//
//	return 0;
//}