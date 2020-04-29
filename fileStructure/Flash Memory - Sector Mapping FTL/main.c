#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "sectormap.h"

FILE *flashfp;


void ftl_open();
void ftl_read(int lsn, char *sectorbuf);
void ftl_write(int lsn, char *sectorbuf);
SpareData set_spare(int lsn, int is_invalid, char *dummy);
void spare_to_str(char *dest, SpareData spare);
void erase_garbage_block();
int search_empty_psn();
void ftl_print();

int main(void) {

    char *fname = "newflash";
    char pagebuf[PAGE_SIZE];
    char sectorbuf[SECTOR_SIZE];
    char sparebuf[SPARE_SIZE];
    int i, pageNum;
 
    //flash memory file 생성 및 초기화
    if((flashfp = fopen(fname, "w+")) == NULL) {
        fprintf(stderr, "fopen error for %s\n",fname);
        exit(1);
    }
    
    memset(pagebuf, (char)0xFF, PAGE_SIZE);
    pageNum = BLOCKS_PER_DEVICE * PAGES_PER_BLOCK;
    for (i = 0; i < pageNum; i++) {
        if (dd_write(i, pagebuf) == -1) {
            fprintf(stderr, "fwrite error\n");
            exit(1);
        }
    }
    fclose(flashfp);

    if ((flashfp = fopen(fname, "r+")) == NULL) {
        fprintf(stderr, "fopen error for %s\n", fname);
        exit(1);
    }
    memset(sectorbuf, 0, SECTOR_SIZE);
    ftl_open();
    ftl_print();
   /* 
    sprintf(sectorbuf, "first sector data!");
    ftl_write(3, sectorbuf);
    ftl_print();
    ftl_read(3, sectorbuf);
    printf("lsn3: %s\n", sectorbuf);


    sprintf(sectorbuf, "second sector data~");
    ftl_write(5, sectorbuf);
    ftl_print();
    ftl_read(5, sectorbuf);
    printf("lsn5 : %s\n", sectorbuf);

    sprintf(sectorbuf, "third sector data:)");
    ftl_write(7, sectorbuf);
    ftl_print();
    ftl_read(7, sectorbuf);
    printf("lsn7 : %s\n", sectorbuf);

    sprintf(sectorbuf, "fourth sector data on lpn3!!@@$%");
    ftl_write(3, sectorbuf); 
    ftl_print();
    ftl_read(3, sectorbuf);
    printf("lsn3 : %s\n", sectorbuf);
*/
    ftl_write(2, "first data");
    ftl_write(4, "second data");
    ftl_write(8, "third data");
    ftl_write(2, "again 2, foutrh data");
    ftl_write(7, "fifth data");
    ftl_write(8, "sixth data");
    ftl_write(4, "asdfaef");
    ftl_write(1, "asefczvxc");
    ftl_write(5, "klj;lkj");
    ftl_write(8, "agaitnawe ife");
    ftl_write(7, "aelskfjlc");
    ftl_write(3, "latiesjalf");
    
    ftl_print();
    ftl_write(4, "finally!!");
    ftl_print();
    

} 
