#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef BUFLEN
	#define BUFLEN 1024
#endif


char *rtrim(char *str);
void ssu_rsync(int argc, char *argv[]);
int doSync();
long getDirSize(char *dirName);
int compareFile(int isDir, char *file1, char *file2);
int syncDir();
int syncFile();
int isExist(char *dirName, char *fname, char *pathname);
char *rmvpath(char *str);
static void cancel_sync_handler(int signo);

