#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

int main(int argc, char *argv[]){
    int fd1, fd2;
    char character;

    if(argc != 3){ //입력받은 실행 옵션 인자 부족할 시 에러메시지 출력 후 종료
        fprintf(stderr, "Usage : %s file\n", argv[0]);
        exit(1);
    }

    if((fd1 = open(argv[1], O_RDWR)) < 0) { //병합할 파일 읽기쓰기 모두 가능한 플래그로 오픈
        fprintf(stderr, "open error for %s\n", argv[1]);
        exit(1);
    }

    if((fd2 = open(argv[2], O_RDONLY)) < 0){ //병합될 파일 읽기만 가능한 플래그로 오픈
        fprintf(stderr, "open error for %s\n", argv[2]);
        exit(1);
    }
    
    lseek(fd1, (off_t)0, SEEK_END); //병합할 파일의 끝으로 오프셋 이동
    lseek(fd2, (off_t)0, SEEK_SET);

    while(read(fd2, &character, 1) > 0){ //병합될 파일의 끝까지 병합할 파일로 데이터 읽고 쓰기
        write(fd1, &character, 1);
    }

    printf("Merge <%s> and <%s> are Succeed!\n",argv[1],argv[2]);

    exit(0);
}
