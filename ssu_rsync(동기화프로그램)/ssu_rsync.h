#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef BUFLEN
	#define BUFLEN 1024
#endif




void ssu_rsync(int argc, char *argv[]);
int checkOption(int argc, char *argv[]);
int doSync();
char *rtrim(char *str);
long getDirSize(char *dirName);
int rmvDir(char *dirname);
int compareFile( char *file1, char *file2);
int saveDirInfo(char *path, char(*flist)[BUFLEN], int idx);
int mySync(char *originFile, char *syncFile, int modify);
int syncDir();
int syncFile();
int isExist(char *dirName, char *fname, char *pathname);
char *rmvpath(char *str);
void printUsage();
static void cancel_sync_handler(int signo);

