#include<stdio.h>
#pragma warning(disable:4996)

int menu();
void openFan(unsigned char *);
void offFan(unsigned char *);
void reverseFan(unsigned char *);
void displayFan(unsigned char *);
void myflush();
unsigned char char2binary(unsigned char, unsigned char);

unsigned char char2binary(unsigned char num, unsigned char binary) {
	switch (num) {
	case '1' :
		binary = 0x80;
		break;
	case '2' :
		binary = 0x40;
		break;
	case '3' :
		binary = 0x20;
		break;
	case '4' :
		binary = 0x10;
	case '5' :
		binary = 0x08;
		break;
	case '6' :
		binary = 0x04;
		break;
	case '7' :
		binary = 0x02;
		break;
	case '8' :
		binary = 0x01;
		break;
	dafault :
		break;
	}
	return binary;
}

void openFan(unsigned char *fan) {
	unsigned char num = 0x00;
	unsigned char check = 0x00;
	char line[] = "--------------------------------------------------------------\n";
	printf(line);
	printf("\t\tFan ���� �۾� ���� ȭ��\t\t\n");
	printf(line);

	while (1) {
		printf("* OPEN�� FAN ��ȣ�� �Է��Ͻÿ�(1-8) : ");
		scanf("%c", &num);
		if (49 <= num && num <= 56) {
			break;
		}
		else {
			;
		}
	}
	//printf("�Է��� �� ��ȣ %%#x : %#x \t %%d : %d\n", num, num);

	check = char2binary(num, check);
	*fan = *fan | check;
	displayFan(fan);
}
void offFan(unsigned char *fan) {
	unsigned char num = 0x00;
	unsigned char check = 0x00;
	char line[] = "--------------------------------------------------------------\n";
	printf(line);
	printf("\t\tFan �ݱ� �۾� ���� ȭ��\t\t\n");
	printf(line);

	while (1) {
		printf("* CLOSE�� FAN ��ȣ�� �Է��Ͻÿ�(1-8) : ");
		scanf("%c", &num);
		if (49 <= num && num <= 56) {
			break;
		}
		else {
			;
		}
	}
	//printf("�Է��� �� ��ȣ %%#x : %#x \t %%d : %d\n", num, num);

	check = char2binary(num, check);
	check = ~check;
	*fan = *fan & check;
	displayFan(fan);
}
void reverseFan(unsigned char *fan) {
	unsigned char num = 0x00;
	unsigned char check = 0x00;
	char line[] = "--------------------------------------------------------------\n";
	printf(line);
	printf("\t\tFan ��ü ��ȯ �۾� ���� ȭ��\t\t\n");
	printf(line);
	printf("��ü FAN�� ���°� ��ȯ�Ǿ����ϴ�. (ON, OFF ���� �ڹٲ�)\n");
	*fan = ~*fan;
	displayFan(fan);
}

void displayFan(unsigned char *fan) {
	unsigned char check = 0x80, res=0;
	char on[] = "ON";
	char off[] = "OFF";
	printf("--------------------------------------------------------------\n");
	for (int i = 0; i < 8; i++) {
		printf("%d��FAN  ", i + 1);
	}
	printf("\n");
	for (int i = 0; i < 8; i++,check/=2) {
		res = *fan & check;
		if (res == 0) {
			printf("%5s   ",off);
		}
		else {
			printf("%5s   ",on);
		}
	}
	printf("\n");
}

int menu() {
	int choice;
	char menu[] = "1. ȯǳ�� ���� / 2. ȯǳ�� �ݱ� / 3. ȯǳ�� ��ü ��ȯ / 4. ���� : ";

	printf(menu);
	scanf("%d", &choice);
	while (1) {
		while (getchar() != '\n') {
			myflush();
			printf(menu);
			scanf("%d", &choice);
		}
		if (choice<1||choice>4) {
			printf(menu);
			scanf("%d", &choice);
		}
		else {
			break;
		}
	}

	return choice;
}


void myflush() {
	while (getchar() != '\n') {
		;
	}
}

int main() {

	int choice;
	unsigned char fan = 0;
	char line[] = "--------------------------------------------------------------\n";

	while (1) {
		choice = menu();
		switch (choice) {
		case 1: //ȯǳ�� ����
			openFan(&fan);
			printf(line);
			break;
		case 2: //ȯǳ�� �ݱ�
			offFan(&fan);
			printf(line);
			break;
		case 3: //ȯǳ�� ��ü ��ȯ
			reverseFan(&fan);
			printf(line);
			break;
		case 4: //����
			return 0;
		}
	}

	return 0;
}