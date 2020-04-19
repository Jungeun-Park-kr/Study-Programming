#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

int main (int argc, char *argv[]){

    int fd;
    int len, offset;

    if(argc != 4){
        fprintf(stderr, "Usage : %sfile\n",argv[0]);
        exit(1);
    }

    if((fd = open(argv[1], O_RDWR)) < 0 ){
        fprintf(stderr, "open error for %s\n", argv[1]);
        exit(1);
    }

    len = strlen(argv[3]); //입력받은 데이터의 길이
    offset = atoi(argv[2]); //입력받은 오프셋 저장

    lseek(fd, offset, SEEK_CUR); //입력받은 만큼 오프셋 이동

    if(write(fd, argv[3], strlen(argv[3])) > 0){ //데이터 파일에 쓰기
        printf("overwrite Succeed!\n");
    }


    exit(0);
}

