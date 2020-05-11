#include <dirent.h>
#include <stdlib.h>
#ifndef true
    #define false 0
#endif

#ifndef false
    #define true 0
#endif

#ifndef BUFLEN
	#define BUFLEN 1024
#endif

#ifndef COMMAND_ARGC
	#define COMMAND_ARGC 7
#endif

#ifndef MAXFILE
	#define MAXFILE 100
#endif

struct oldFileList {
	char fname[BUFLEN];
	char dtime[BUFLEN];
};
typedef struct oldFileList oldFileList;

void ssu_mntr();
void do_Prompt(int pid);

void do_Monitor(char *logFile);
pid_t monitor_deamon_init();

char *rmvdelimeter(char *str);
char *rtrim(char *str);
char *rmvpath(char *str);
int except_tmp_file(const struct dirent *info);

int save_dir_info(char *path, char(*flist)[BUFLEN], struct tm *tm, int idx);
void update_dir_info(char(*flist1)[BUFLEN], char(*flist2)[BUFLEN], struct tm *tm1, struct tm *tm2, int *cnt1, int *cnt2);
long getDirSize(char *dirName);
void removeOldFile();
int makeInfo(char *fname, char *fnamepath, struct tm dtm);
int intoTrash(char *fname, char *pathname);
int find_max_num(char *fname);
int isExist(char *dirName, char *fname, char *pathname);
int get_path(char *path, char *fname, char *pathname);
int doDelete(int argc, char(*argv)[BUFLEN]);

int do_dOption(char *printpath, char *statpath, int depth, int curdepth);
void doSize(int argc, char(*argv)[BUFLEN]);

int isDup(char *dirName, char *fname, int *num, char(*dupfiles)[BUFLEN]);
int printDup(int num, char *fname, char(*dupFiles)[BUFLEN]);
int get_recover_file(char *recoverfile, char *recoverpath, int isDup, int choice, int nums);
int intoCheck(char *fname, char *pathname);
int get_old_files(oldFileList lists[BUFLEN]);
int compareTime(char *tm1, char *tm2);
void sort_old_files(oldFileList lists[BUFLEN], int cnt);
void do_lOption();
void doRecover(int argc, char(*argv)[BUFLEN]);

int makeTree(int depth, char *dname, char(*fname)[BUFLEN], char *ftype, int *fdep, int idx);
void printTree(int fsize, char(*fname)[BUFLEN], char *ftype, int *fdep, int idx);
void doTree();
void doHelp();
