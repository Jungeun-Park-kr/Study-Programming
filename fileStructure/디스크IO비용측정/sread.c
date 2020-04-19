#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
//필요하면 header file 추가 가능

//
// argv[1]: 레코드 파일명
//

#define BUFFER_SIZE 100

int main(int argc, char **argv)
{
    int fd;
    off_t fsize;
    int records;
    int timecost;
    int buf[BUFFER_SIZE];
    int repeat;
    struct timeval start_time;
    struct timeval end_time;


    if(argc != 2) {
        fprintf(stderr, "Usage : %s file\n", argv[0]);
        exit(1);
    }

    if ((fd = open(argv[1], O_RDONLY)) < 0) { //표준입력을 받은 레코드파일 열기
        fprintf(stderr, "open error for %s\n", argv[1]);
        exit(1);
    }

    if ((fsize = lseek(fd, (off_t)0, SEEK_END)) < 0) { //파일 크기 확인
        fprintf(stderr, "lseek error\n");
        exit(1);
    }
    lseek(fd, (off_t)0, SEEK_SET);

    records = (fsize / 100); //파일전체크기/100B(레코드 하나의 크기) = 레코드 개수
    repeat = records;
    

    gettimeofday(&start_time, NULL); //레코드 읽기 전 시간 저장
    while(--repeat) { //레코드개수만큼 순차적 읽기 시작
        if(read(fd, buf, 100) < 0) {
	   fprintf(stderr,"read error\n");
	   exit(1);
        }
    }
    gettimeofday(&end_time, NULL);
    
    timecost = ((end_time.tv_sec - start_time.tv_sec)*1000000) + ((end_time.tv_usec - start_time.tv_usec));

    
    printf("#records: %d timecost: %d us\n",records, timecost);
    


	return 0;
}
