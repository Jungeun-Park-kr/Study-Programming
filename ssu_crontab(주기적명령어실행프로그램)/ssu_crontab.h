#include <pthread.h>
//crontab이 사용할 헤더
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
crontabLog[BUFLEN]; //명령어의 변경사항이나, 정상 수행된 경우 해당 내용을 기록할 로그 파일 이름

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

typedef struct schedule {
    char min[8];
    char hour[8];
    char day[8];
    char mon[8];
    char dayofweek[8];
} Schedule;

int doPrompt(void);
int checkSchedule(void);
int crond_daemon_init(void);
int printPrompt(void);