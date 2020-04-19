#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 100
#define S_MODE 0644

int main(int argc, char* argv[]){

    char buf[BUFFER_SIZE];
    int fd1, fd2;
    int len;

    if(argc != 3){ //실행 옵션의 인자 부족시 에러메시지 출력 후 종료
        fprintf(stderr,"Usage : %s filein fileout\n", argv[0]);
        exit(1);
    }

    if((fd1 = open(argv[1], O_RDONLY)) < 0 ) { //데이터를 복사할 파일 오픈
        fprintf(stderr, "open error for %s\n",argv[1]); 
        exit(1);
    }

    if((fd2 = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, S_MODE)) < 0){ //복사한 내용을 붙여넣을 파일 생성,오픈(이미 내용 있을시 길이0으로하는 플래그)
        fprintf(stderr, "open error %s\n",argv[2]);
        exit(1);
    }

    while((len = read(fd1, buf, BUFFER_SIZE)) > 0){ //파일 데이터 복사
        write(fd2, buf, len); //복사한 데이터 붙여넣기
    }
    
    printf("File copy <%s> to <%s> is Succeed!\n",argv[1],argv[2]);

    exit(0);

}



