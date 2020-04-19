#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
    int fd1, fd2;
    off_t buflen;
    int len, offs;
    char buf[BUFFER_SIZE];

    if(argc != 4){
        fprintf(stderr, "Usage %s file\n", argv[0]);
        exit(1);
    }

    if((fd1 = open(argv[1], O_RDWR | O_CREAT, 0644)) < 0) { //파일 오픈, 없을시 생성
        fprintf(stderr, "open error for %s\n", argv[1]);
        exit(1);
    }
    if((fd2 = open(argv[1], O_RDWR | O_CREAT, 0644)) < 0) { //파일 오픈, 없을시 생성
        fprintf(stderr, "open error for %s\n", argv[1]);
        exit(1);
    }

    offs = atol(argv[2]); //입력받은 오프셋 저장
    len = strlen(argv[3]); //입력받은 데이터의 길이 저장
    
    lseek(fd1, (off_t)offs , SEEK_CUR); //주어진 오프셋으로 fd1의 오프셋 이동
    
    lseek(fd2, (off_t)offs, SEEK_CUR); //fd2를 데이터가 입력될 오프셋으로 이동

    if((buflen = read(fd2, buf, BUFFER_SIZE)) < 0){ //끼워넣기 될 위치 이후에 위치한 기존 데이터 복사해두기
        fprintf(stderr, "read error\n");
        exit(1);
    }
    
    lseek(fd2, (off_t)0, SEEK_SET);  
    lseek(fd2, (off_t)(offs + len), SEEK_CUR); //삽입될 데이터 뒤로 fd2의  오프셋 이동


    if(write(fd1, argv[3], len) > 0){ //입력한 데이터의 길이만큼 끼워넣기
        if(write(fd2, buf, buflen) > 0) { //복사해둔 기존 데이터의 내용 마저 쓰기
	   printf("file insert Succeed!\n");
        }
    }

    exit(0);
}
