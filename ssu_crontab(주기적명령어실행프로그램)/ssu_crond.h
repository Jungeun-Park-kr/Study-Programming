//디몬 프로세스 헤더
#ifndef TRUE
	#define TRUE 1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

#ifndef BUFLEN
	#define BUFLEN 1024
#endif

char crontabFile[BUFLEN]; //주기적으로 실행할 명령어가 들어가있는 파일 이름
char crontabLog[BUFLEN]; //명령어의 변경사항이나, 정상 수행된 경우 해당 내용을 기록할 로그 파일 이름
char workDir[BUFLEN]; //현재 작업디렉토리의 경로를 저장해줌

int crond_daemon_init(void);
int doCrond(void);
int checkTime(char *time, int type);
int compareTime(char *time, int type);