#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "sectormap.h"

FILE *flashfp;

int main(void) {

    char *fname = "flashMemory";
    char pagebuf[PAGE_SIZE];
    char sectorbuf[SECTOR_SIZE];
    char sparebuf[SPARE_SIZSE];
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
    //fclose(flashfp);

    ftl_open();
    sprintf(sectorbuf, "first sector data!");
    ftl_write(3, sectorbuf);
    sprintf(sectorbuf, "second sector data~");
    ftl_write(5, sectorbuf);
    sprintf(sectorbuf, "third sector data:)");
    ftl_write(7, sectorbuf);
    sprintf("fourth sector data on lpn3!!@@$%#");
    ftl_print(3, sectorbuf);  
} 
