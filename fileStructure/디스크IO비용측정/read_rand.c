#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>

#define SUFFLE_NUM	10000
#define BUFFER_SIZE 101
void GenRecordSequence(int *list, int n);
void swap(int *a, int *b);

int main(int argc, char **argv)
{
    int *read_order_list;
    int num_of_records;
    int fd;
    off_t fsize;
    int i;
    int cur;
    char buf[BUFFER_SIZE];
    int timecost;

    struct timeval start_time;
    struct timeval end_time;
    
    if (argc != 2) { //프로그램 실행시 인자 부족할 경우 에러메시지 출력 후 종료
        fprintf(stderr, "Usage : %s file\n", argv[0]);
        exit(1);
    }

    if ((fd = open(argv[1], O_RDONLY)) < 0) { //인자로 입력한 파일 오픈
        fprintf(stderr, "open error for %s\n", argv[1]);
        exit(1);
    }
    
    if ((fsize = lseek(fd, (off_t)0, SEEK_END)) < 0) { //file size 저장
        fprintf(stderr, "lseek error\n");
        exit(1);
    }

    num_of_records = (fsize/100); //레코드 수 : 파일크기 / 100(레코드 하나의 크기)
    
    read_order_list = (int *)malloc(sizeof(int)*num_of_records); //random레코드 번호를 저장할 배열 동적할당
        
    GenRecordSequence(read_order_list, num_of_records); //랜덤으로 레코드 읽을 순서를 정하는 함수 호출


    gettimeofday(&start_time, NULL); //읽기 전 시간 저장
    for(i = 0; i < num_of_records; ++i) { //랜덤한 순서로 레코드를 하나씩 읽기를 레코드 수 만큼 반복
        cur = *(read_order_list+i); //랜덤 레코드 번호
        
        if (lseek(fd, (off_t)cur*100, SEEK_SET) < 0) { //랜덤한 파일 레코드의 해당 위치로 이동
	   fprintf(stderr, "lseek error\n");
	   exit(1);
        }
        if (read(fd, buf, 100) < 0) { //도착한 위치에서 한 개의 레코드를 읽음
	   fprintf(stderr, "read error\n");
	   exit(1);
        }
        
    }
    gettimeofday(&end_time, NULL); //데이터 모두 읽은 후 시간
        
    timecost = ((end_time.tv_sec - start_time.tv_sec)*1000000) + (end_time.tv_usec - start_time.tv_usec); //timecost계산
    
    printf("#records: %d timecost: %d us\n",num_of_records, timecost); //레코드 개수와 timecost출력 후 종료

    return 0;
}

void GenRecordSequence(int *list, int n) //랜덤한 레코드 번호를 저장할 리스트와 레코드 개수 파라미터
{       //랜덤으로 생성된 번호를 충분한 만큼 스왑해서 레코드 값이 무작위 배열되도록 하는 함수
	int i, j, k;

	srand((unsigned int)time(0));

	for(i=0; i<n; i++)
	{
		list[i] = i;
	}
	
	for(i=0; i<SUFFLE_NUM; i++)
	{
		j = rand() % n;
		k = rand() % n;
		swap(&list[j], &list[k]);
	}

	return;
}

void swap(int *a, int *b)
{
	int tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;

	return;
}
