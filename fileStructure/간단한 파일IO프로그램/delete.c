#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]){

    int fd1, fd2;
    off_t fsize, fchange, fdel;
    off_t mv_offset, del_byte, tmp;
    char character;
    char buf[BUFFER_SIZE];
    int len;
    int i;

    if(argc != 4) { //실행시 옵션 인자 개수 부족시 에러메시지 출력 후 종료
        fprintf(stderr, "Usage : %s\n", argv[0]);
        exit(1);
    }

    if((fd1 = open(argv[1], O_RDWR | 0644)) < 0){ //삭제후에 기존 파일을 앞으로 옮겨 쓰기위해 읽기 모두 가능하도록 파일 오픈
        fprintf(stderr, "open error for %s\n", argv[1]);
        exit(1);
    }
    
    if((fd2 = open(argv[1], O_RDONLY | 0644)) < 0){ //삭제할 바이트 이후에 있는 데이터를 읽기 위해 읽기 플래그로 파일 오픈
        fprintf(stderr, "open error for %s\n", argv[1]);
        exit(1);
    }

    mv_offset = (off_t)atol(argv[2]); //삭제될 위치 오프셋 저장
    del_byte = (off_t)atol(argv[3]); //삭제할 바이트 수 저장

    fsize = lseek(fd1, (off_t)0, SEEK_END); //파일 크기를 저장하기 위해 lseek()
    lseek(fd1, (off_t)0, SEEK_SET); //fd1의 오프셋 파일 처음으로 초기화

    lseek(fd1, mv_offset, SEEK_CUR); //삭제될 위치의 오른쪽으로 fd1의 오프셋 이동
    lseek(fd2, (off_t)(mv_offset + del_byte), SEEK_CUR); //fd2의 오프셋은 삭제 이후의 기존 데이터중 첫번째로 가리킴


    while(1){    
        if(read(fd2, &character, 1) > 0) { //아직 옮기지 못한 기존 데이터가 남아있을 때 까지 fd2로  읽기
	   write(fd1, &character, 1); //읽은 데이터를 주어진 오프셋 오른쪽부터 한 글자씩 다시 옮기기(쓰기)
        }else{ //더이상 옮길 내용 없으면 반복문 종료
	   break;
        }
    }

   
    if(del_byte > (fsize - mv_offset)){ //삭제한 바이트 수 만큼의 데이터가 존재하지 않는 경우
        if(ftruncate(fd1, mv_offset) < 0){ //파일의 마지막 바이트 까지 삭제한후의 파일 길이로 조절
	   fprintf(stderr, "ftruncate error\n");
	   exit(1);
        }
        else{
	   exit(0);
        } 
    }
    
    if(ftruncate(fd1, fsize-del_byte) < 0){ //삭제한 후의 파일 사이즈로 길이 조절
        fprintf(stderr, "ftruncate error\n");
        exit(1);
    }


    exit(0);
}


