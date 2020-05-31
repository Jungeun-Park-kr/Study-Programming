
/*
 *  실행주기 확인 필요
 *  remove 명령어 정상 작동 되는지 확인필요
 *      remove할때, crond프로그램이 해당 파일 접근 불가하도록 막아야 함
 *  add 명령어 추가 필요 (doAdd()함수 작성하기)
 *  crond프로그램 작성 필요
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ssu_crontab.h"
#include "ssu_crond.h"

extern char crontabFile[BUFLEN];
extern char crontabLog[BUFLEN];
char Command[BUFLEN];
char argv[COMMAND_ARGC][BUFLEN];
int argc;
FILE *cronfp;

int main() {
    char saved_path[BUFLEN];
    FILE *fp;
    getcwd(saved_path, BUFLEN); //현재 작업 경로 저장

    sprintf(crontabFile, "%s/%s",saved_path, "ssu_crontab_file"); //크론탭 파일 이름저장
    sprintf(crontabLog, "%s/%s", saved_path, "ssu_crontab_log"); //크론탭 로그 파일 이름 저장
    if (access(crontabLog, F_OK)) { //로그파일 없는경우 생성
        if ((fp = fopen(crontabLog, "w+")) <0) {
            fprintf(stderr, "fopen error for %s\n", crontabLog);
            exit(1);
        }
        close(fp);
    }
    //프롬프트 실행
    if(!doPrompt())
        exit(1);

}

int printPrompt(void) {
    char *prompt = " \n20180753> ";
    char listbuf[BUFLEN], idxbuf[BUFLEN];
    int idx = 0;
    char *ptr;
    if (!access(crontabFile, F_OK)) { //파일이 이미 존재하는 경우
        if ((cronfp = fopen(crontabFile, "r")) == NULL) {
            fprintf(stderr, "fopen error for %s\n", crontabFile);
            return FALSE;
        }
        while(fgets(listbuf, BUFLEN, cronfp) != NULL) { //저장된 모든 명령어 출력
            sprintf(idxbuf, "%d. ", idx++);
            puts(idxbuf); //현재 명령어 번호 출력
            if (puts(listbuf) == EOF) { //뿌려주기
                fprintf(stderr, "puts error\n");
                return FALSE;
            }
        }
        fclose(cronfp);
    } 
    //파일 없는 경우 제어 바로 여기로 옴
    fputs(prompt, stdout); //프롬프트 출력
    return idx;
}

int doPrompt(void) {
    char *prompt = " \n20180753> ";
    char listbuf[BUFLEN], idxbuf[BUFLEN], logbuf[BUFLEN], timebuf[BUFLEN];
    int idx = 0, i;
    char *ptr;
    Schedule schedule;
    char usrCommand[BUFLEN];
    FILE *logfp;
    time_t cur_time;

    while(1) {
        argc = 0;
        for (i = 0; i < COMMAND_ARGC; i++) {
            memset(argv[i], (char)0, BUFLEN);
        }

        if ((idx = printPrompt()) == 0) { //프롬프트 출력
            fprintf(stderr, "printPrompt error!\n");
            return FALSE;
        }
        while (fgets(Command, BUFLEN, stdin) == NULL) { //엔터만 받으면 다시 재입력
            printPrompt();
        }
        if (ferror(stdin)) { //stdin에러처리
            fprintf(stderr, "standard input error\n");
            return FALSE;
        }

        //Command 분리
        ptr = strtok(Command, " "); 
        while (ptr != NULL) {
            strcpy(argv[argc++], ptr);
            ptr = strtok(NULL, " ");
        }

        memset(usrCommand, 0,BUFLEN);
        if(strstr(argv[0], "add") != NULL) {
            printf("--add 명령어--\n");
            //ssu_crontab_file 여부 확인
            if (access(crontabFile, F_OK)) { //파일이 없는 경우
                if ((cronfp = fopen(crontabFile, "w+")) == NULL) { //새로 생성
                    fprintf(stderr, "fopen error for %s\n", crontabFile);
                    exit(1);
                }
            }
            else { //파일 이미 존재하는 경우, 추가 모드로 열기
                if ((cronfp = fopen(crontabFile, "a+")) == NULL) {
                    fprintf(stderr, "fopen error for %s\n", crontabFile);
                    exit(1);
                }
            }
            
            if (!checkSchedule()) //실행주기 올바르게 입력되었는지 확인
                continue; //안된경우 프롬프트 출력
            else { //실행주기 확인 완료!
                for(i=1; i<6; i++) { //argv[1]~argv[5]까지 저장
                    strcat(usrCommand, argv[i]);
                    strcat(usrCommand, " ");
                }
            }
            //이제 명령어 부분 저장
            for(i=6; i<argc; i++) {  //argv[6]부터 끝까지 저장
                strcat(usrCommand, argv[i]);
                strcat(usrCommand, " ");
            }

            if (!doAdd(usrCommand)) //Add명령어 ssu_crontab_file에 저장
                continue; //실패한 경우, 프롬프트 출력
            else { //정상적으로 파일에 저장된 경우 -> 로그 파일에 저장한 명령어 기록
                time(&cur_time);
	            ctime_r(&cur_time, timebuf);
                timebuf[19] = 0; //요일 월 일 시간 까지만 출력할것임
                sprintf(logbuf, "[%s] %s\n", timebuf, usrCommand);
                if((logfp = fopen(crontabLog, "a+")) < 0){
                    fprintf(stderr, "fopen error for %s\n", crontabLog);
                    return FALSE;
                }
                if (fputs(logbuf, logfp) < 0) { //로그 파일 작성
                    fprintf(stderr, "write error\n");
                    return FALSE;
                }
                fclose(logfp);
            }
        }
        else if(strstr(argv[0], "remove") != NULL) {
            printf("--remove 명령어--\n");
            if (argc != 2) { //명령어 인자 개수 확인!
                fprintf(stderr, "usage : remove <REMOVE NUMBER>\n");
                continue;
            }
            int rmvnum = atoi(argv[1]);
            if (rmvnum > idx && rmvnum < 0) { //존재하는 명령어 개수의 범위에 벗어나는 경우 에러처리 후 프롬프트
                fprintf(stderr, "Invalid remove number!");
                continue;
            }
            strcat(usrCommand, argv[0]);
            strcat(usrCommand, argv[1]);
            if (!doRemove(rmvnum)) //삭제 실패
                continue;
            else { //정상적으로 삭제된 경우 -> 로그파일에 기록
                time(&cur_time);
	            ctime_r(&cur_time, timebuf);
                timebuf[19] = 0; //요일 월 일 시간 까지만 출력할것임
                sprintf(logbuf, "[%s] %s\n", timebuf, usrCommand);
                if((logfp = fopen(crontabLog, "a+")) < 0){
                    fprintf(stderr, "fopen error for %s\n", crontabLog);
                    return FALSE;
                }
                if (fputs(logbuf, logfp) < 0) { //로그 파일 작성
                    fprintf(stderr, "write error\n");
                    return FALSE;
                }
                fclose(logfp);
            }
        }
        else if (strstr(argv[0], "exit") != NULL) {
            printf("--exit 명령어--\n");
            break;
        }
        else { //이외의 명령어 -> 프롬프트 출력'
            fprintf(stderr, "<%s> command not exist!", argv[1]);
            continue;
        }
    }
    return TRUE;
}

int doAdd(char *command) {
    //파라미터로 넘어온 명령어를 ssu_crontab_file 파일에 추가한다.

}

int doRemove(int num) {
    //파라미터로 들어온 번호의 명령어를 ssu_crontab_file에 삭제한다
    char listbuf[BUFLEN];
    int idx=0;
    FILE *tmpfp;
    char tmpfile[BUFLEN];
    sprintf(tmpfile, "%s_tmp", crontabFile);
    
    if ((cronfp = fopen(crontabFile, "r")) == NULL) {
        fprintf(stderr, "fopen error for %s\n", crontabFile);
        return FALSE;
    }
    if ((tmpfp = fopen(tmpfile, "w+")) == NULL) {
        fprintf(stderr, "fopen error for %s\n", crontabFile);
        return FALSE;
    }
    //해당 번호 제외 임시 파일에 복사
    while (fgets(listbuf, BUFLEN, cronfp) != NULL) {
        if (idx == num) {
            idx++;
            continue;
        }
        if (fputs(listbuf, tmpfp) == NULL) {
            fprintf(stderr, "fputs error\n");
            return FALSE;
        }
        idx++;
    }
    fclose(cronfp);
    fclose(tmpfile);
    if (remove(crontabFile) < 0) { //원본파일 삭제
        fprintf(stderr, "remove error\n");
        return FALSE;
    }
    if (rename(tmpfile, crontabFile) < 0) { //임시 파일을 원본으로 변경
        fprintf(stderr, "rename error\n");
        return FALSE;
    }
    return TRUE; 
}

int checkToken(Schedule schedule) {
    //실행 주기가 제대로 입력되었는지 확인 후, 올바르면 TRUE 틀리면 FALSE 리턴
    int i;
    int min;
   
}

int checkSchedule(void) {
    char *ptr;
    int i = 0;
    Schedule schedule;
    if (argc < 7) 
        return FALSE;
    strcpy(schedule.min, argv[1]);
    strcpy(schedule.hour,argv[2]);
    strcpy(schedule.day, argv[3]);
    strcpy(schedule.mon, argv[4]);
    strcpy(schedule.dayofweek, argv[5]);
    if (!checkToken(schedule))
        return FALSE;
    else {
        printf("Schedule : %s %s %s %s %s\n", schedule.min, schedule.hour, schedule.day, schedule.mon, schedule.dayofweek);
        return TRUE;
    }
}


int crond_daemon_init(void) { //명령어를 실행시킬 디몬 생성
    pid_t pid;
    int fd, maxfd;

    if ((pid = fork()) < 0) {
        fprintf(stderr, "fork error\n");
        exit(1);
    }
    else if (pid != 0)
        exit(0);
    
    pid = getpid();
    setsid();
    signal(SIGTTIN, SIG_IGN); //작업제어와 관련된 시그널 무시
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    maxfd = getdtablesize();

    for (fd=0; fd < maxfd; fd++)
        close(fd);

    umask(0);
    chdir("/");
    fd = open("dev/null", O_RDWR);
    dup(0);
    dup(0);
    return 0;
}