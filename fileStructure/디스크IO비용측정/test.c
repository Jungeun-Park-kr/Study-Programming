#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
int main(){

    int i;
    char *filename = "testfile2.txt";
    char *openfile = "words.txt";
    int fd;
    int fd2;
    char buf[101];

    if((fd = open(filename, O_RDWR|O_CREAT)) < 0){
        fprintf(stderr, "open error for %s\n", filename);
        exit(1);
    }
    if ((fd2 = open(openfile, O_RDONLY)) < 0) {
        fprintf(stderr, "open error for %s\n", openfile);
        exit(1);
    }

    int len = read(fd2, buf, 100);
    printf("len : %d", len);

    for(i=0; i<3000; ++i){
        write(fd, buf, 100);
    }

    return 0;
}
