#include <stdio.h>

int main() {
	int i, j;
	char three[] = "*";
	char five[] = "#";

	for (i = j=1; i <= 100; i++,j++) {
		if (i % 3 == 0 && i % 5 == 0) {
			printf("%4d", i);
		}
		else if (i % 3 == 0) {
			printf("%4s", three);
		}
		else if (i % 5 == 0) {
			printf("%4s", five);
		}
		else {
			printf("%4d", i);
		}

		if (i % 10 == 0) {
			printf("\n");
		}
	}

	return 0;
}