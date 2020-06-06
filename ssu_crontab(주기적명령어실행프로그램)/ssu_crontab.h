#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef BUFLEN
	#define BUFLEN 1024
#endif

#define COMMAND_ARGC 100

char crontabFile[BUFLEN]; //주기적으로 실행할 명령어가 들어가있는 파일 이름
char crontabLog[BUFLEN]; //명령어의 변경사항이나, 정상 수행된 경우 해당 내용을 기록할 로그 파일 이름
char workDir[BUFLEN]; //현재 작업디렉토리의 경로를 저장해줌

int printPrompt(void);
int doPrompt(void);
char *rtrim(char *str);
int doRemove(int num);
int doAdd(char *command);
int checkRange(int num, int type);
int checkItem(char *item, int type);
int checkSchedule(void);
