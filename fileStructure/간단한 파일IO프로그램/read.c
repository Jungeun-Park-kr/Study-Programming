#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){
    off_t fd1;
    off_t buf_size;
    off_t off_len;
    int len;
    char buf[BUFFER_SIZE];
    int offset;

    if(argc != 4) { //파일 실행시 옵션 인자개수 부족시 에러메시지 출력 후 종료
        fprintf(stderr, "Usage : %s file\n", argv[0]);
        exit(1);
    }

    if((fd1 = open(argv[1], O_RDONLY)) < 0) { //읽기 플래그로 파일 오픈
        fprintf(stderr, "open error for %s\n", argv[1]); 
        exit(1);
    }

    off_len = atol(argv[2]); //입력받은 오프셋 저장

    if((lseek(fd1, off_len, SEEK_CUR)) < 0){ //해당 오프셋으로 오프셋 이동
        fprintf(stderr, "lseek error\n");
        exit(1);
    }

   buf_size = atol(argv[3]); //입력받은 읽기 바이트 수 저장
   
    if((len = read(fd1, buf, buf_size)) > 0) { //입력받은 데이터 읽어 buf에 저장후 리턴받은 데이터 바이트수 len 변수에 저장
        write(1, buf, len); //저장한 데이터(buf)를 표준출력(파일디스크립터1)에 쓰기 => 화면에 출력
    }

   exit(0); 

}
