#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ssu_crond.h"

typedef struct tm tm;
time_t cur_time;
extern char crontabFile[BUFLEN]; //명령어가 저장된 파일
extern char crontabLog[BUFLEN]; //명령어 정상 수행시 기록할 로그파일
extern char workDir[BUFLEN]; //작업 디렉토리 위치
int mon, day, hour, min, wday;

int main(void) {
    pid_t crond_pid;
    getcwd(workDir, BUFLEN); //현재 작업 경로 저장
    sprintf(crontabFile, "%s/%s", workDir, "ssu_crontab_file"); //크론탭 파일 이름저장
    sprintf(crontabLog, "%s/%s", workDir, "ssu_crontab_log"); //크론탭 로그 파일 이름 저장
    crond_pid = crond_daemon_init();
    chdir(workDir);
    if(crond_pid == 0) { //생성한 데몬프로세스는 명령어 수행
        tm *now;
        //명령어 실행 시작
        while(1) {
            if(!access(crontabFile, F_OK)) //파일이 있는 경우
                break;
            else //파일 없는 경우
                sleep(60);
        }
        //파일 있는 경우 제어가 여기로 옴
        while(1) { //현재 시간 저장 후, doCrond()실행
            time(&cur_time);
            now = localtime(&cur_time);
            mon = now->tm_mon + 1; //tm구조체의 월은 0~11까지 저장되어 있으므로 +1해줌
            day = now->tm_mday;
            hour = now->tm_hour;
            min = now->tm_min;
            wday = now->tm_wday;
            //printf("현재시간 : min:%d, hour:%d, day:%d, mon:%d, wday:%d\n",min,hour,day,mon,wday);
            if (doCrond()) { //호출 후 리턴되면
                sleep(60); //다음 1분 까지 기다림
            }
        }
    }
    exit(0);
}

int crond_daemon_init(void) {//명령어를 실행시킬 디몬 생성
    pid_t pid;
    int fd, maxfd;

    if ((pid = fork()) < 0) {
        fprintf(stderr, "fork error\n");
        exit(1);
    }
    else if (pid != 0)
        return pid;
    
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
    return pid; //정상 생성된 경우 자식 프로세스인 0이 리턴될 것
}

int doCrond() {
    FILE *cronfp, *logfp;
    char listbuf[BUFLEN], timebuf[BUFLEN], tmpbuf[BUFLEN];
    char *ptr;
    char schedule[5][BUFLEN];
    char *command;
    int idx = 0, i = 0;
    int isMatch = TRUE;
    int commandlen=0, res;
  

    if ((cronfp = fopen(crontabFile, "r")) == NULL) {
        fprintf(stderr, "fopen error for %s\n", crontabFile);
        return FALSE;
    }
    while(fgets(listbuf, BUFLEN, cronfp) != NULL) { //저장된 모든 명령어 한줄씩 가져옴
        strcpy(tmpbuf, listbuf); //명령어를 tmpbuf에 복사
        command = malloc(sizeof(char) * BUFLEN); //읽어온 명령어를 저장할 배열의 메모리 할당
        idx = commandlen = 0;
        isMatch = TRUE; //시간이 일치하는지 확인하는 변수
        ptr = strtok(listbuf, " "); //공백을 기준으로 토큰을 나눔
        while (ptr != NULL) {
            if (idx > 4) {//실행 주기 부분 모두 저장한 경우 빠져나감
                strcpy(command, tmpbuf + commandlen);
                break;
            }
            commandlen += (strlen(ptr)+1); //해당 명령어 길이 크기를 저장
            strcpy(schedule[idx++], ptr); //실행 주기 부분만 schedule에 저장하기
            ptr = strtok(NULL, " "); //다음 토큰 값 저장
        }
        //해당 명령어가 지금 실행되어야 하는지 확인
        for(i = 0; i < 5; i++) { //항목별로 하나씩 확인
            //명령어에 , 있는 경우 또 토큰으로 나눔
            if (!checkTime(schedule[i], i)) { 
                //하나라도 시간 틀린 경우 다음 명령어 확인 위해 빠져나감
                isMatch = FALSE; //시간 맞지 않음을 저장
                break;
            }
        }
        if (isMatch) { //시간이 같은 경우 
            res = system(command); //system()함수로 명령어 실행
            if (res == -1 || res == 127) { //fork() error, exec계열 error시 로그 안찍음
                fclose(cronfp);
                return TRUE;
            }
            //정상 수행 완료되었으면 로그 쓰기
            ctime_r(&cur_time, timebuf);
            timebuf[19] = 0; //요일 월 일 시간 까지만 출력할것임
            sprintf(tmpbuf, "[%s] run %s", timebuf, command);
            if((logfp = fopen(crontabLog, "a+")) < 0){
                fprintf(stderr, "fopen error for %s\n", crontabLog);
                return FALSE;
            }
            if (fputs(tmpbuf, logfp) < 0) { //로그 파일 작성
                fprintf(stderr, "write error\n");
                return FALSE;
            }
            fclose(logfp);
        }
    }
    fclose(cronfp);
    return TRUE;
}

int compareTime(char *time, int type) {
    // 단일 항목에 해당하는 time과 현재 시간 ctime을 비교해 맞으면 TRUE, 틀리면 FALSE를 리턴함
    // 파라미터 : time - 지금 확인할 단일 항목, type - 항목의 종류(분1 ,시2 ,일3 ,월4 ,요일5) 
    int num, n1, n2;
    int i;
    int begin, end; //항목별로 시작값, 끝값 다름
    int ctime; //ctime - 현재 시간
    switch(type) { //항목종류별로 최대 최소 값이 다름
    case 0: //분
        begin = 0;
        end = 59;
        ctime = min;
        break;
    case 1: //시
        begin = 0;
        end = 23;
        ctime = hour;
        break;
    case 2: //일
        begin = 1;
        end = 31;
        ctime = day;
        break;
    case 3: //월
        begin = 1;
        end = 12;
        ctime = mon;
        break;
    case 4: //요일
        begin = 0; //일요일
        end = 6; //월요일
        ctime = wday;
        break;
    }
    if (strlen(time) < 3) {
        if (!strcmp(time, "*")) //*면 모든 시간에 실행 가능함
            return TRUE;
        if (isdigit(time[0])) { //숫자인 경우
            num = atoi(time);
            if (num == ctime) //현재시간과 같으면 TRUE리턴
                return TRUE;
        }
        else //그 이외인 경우 false
            return FALSE;
    }
    else {
        if (strstr(time, "*/") != NULL) { //*/가 사용된 경우
            if ((sscanf(time, "*/%d", &num) != 1)) { //문자 뒤 내용이 숫자가 아니면 오류
                return FALSE;
            }
            for(i = begin-1; i <= end; i+=num) {
                if (i == ctime) //해당 주기 값이 현재 시간인 경우
                    return TRUE;
                else if (i > ctime)
                    return FALSE; //실행주기 아님
            }
            return FALSE;
        }
        else if (strstr(time, "-") != NULL) { // -가 사용된 경우
            if (strstr(time, "/") != NULL) { // n1-n2/num 구조인 경우
                if (sscanf(time, "%d-%d/%d", &n1, &n2, &num) != 3) //범위의 값 가져오기
                    return FALSE;
                if (n1 <= n2) { //n2가 더 큰 경우
                    if (n1 <= ctime && ctime <= n2) { //현재 시간이 항목 사이 값일 때만 실행
                        for (i = n1-1; i <= n2; i += num) {
                            if (i == ctime)  //시간 맞음
                                return TRUE;
                            else if (i > ctime)  //실행 주기 해당 X 
                                return FALSE;
                        }
                        return FALSE;
                    }
                    return FALSE;
                }
                else if (n1 > n2) { //n1이 더 큰 경우
                    if ((n1 <= ctime && ctime < end)&&(ctime <= n2 && begin <= ctime)) { //실행 가능한 범위 확인- 현재 시간이 n1보단 크고, n2보단 작으며 각 begin~end 사이 범위어야 함
                        for(i = n1-1;  ;i += num) {
                            if (i > end) {
                                i = i-end-i; //end값 넘어서는 경우 넘치는 만큼 빼줌(ex, 시:25 분 60 61 등)
                                continue;
                            }
                            if (i == ctime) //시간 맞는 경우
                                return TRUE;
                            else if (n1 < ctime && ctime < n2) //실행 주기에 해당하지 않는 값인 경우
                                return FALSE;
                        }
                    }
                    return FALSE;
                }
                else //현재 시간이 범위 안에
                    return FALSE;         
            }
            else { //그냥 n1-n2 범위만 사용된 경우
                if (sscanf(time, "%d-%d", &n1, &n2) != 2) //범위의 숫자 n1-n2 형태로 저장
                    return FALSE;
                if (n1 <= n2) { // n1<=n2인 경우
                    if (n1 <= ctime && ctime <= n2) //현재 시간이 범위 내의 값인 경우
                        return TRUE;
                    else
                        return FALSE;
                }
                else if (n1 > n2) { //앞 범위 시간이 더 큰경우
                    if (n1 >= ctime) //n1보다 현재 시간이 더 크면 true
                        return TRUE;
                    else if (n2 <= ctime) //n2보다 현재시간이 더 작으면 true
                        return TRUE;
                    else
                        return FALSE;
                }
                else //범위가 아닌 경우
                    return FALSE;
            }
        }
        else //그 외는 다 틀림
            return FALSE;
    }
}

int checkTime(char *time, int type) { 
    //현재 시간과 항목의 시간이 일치하면 TRUE, 불일치하면 FALSE 리턴
    //파라미터 : idx - 실행주기의 각 항목, time - 항목 문자열
    char token[BUFLEN][BUFLEN];
    char buf[BUFLEN]; 
    char *ptr;
    int i, idx;
    int isMatch = FALSE;
    if (strstr(time, ",") != NULL) { // 목록의 경우 : 해당하는 값이 1개만 있으면 됨
        for(i = 0; i < BUFLEN; i++) //토큰 배열 초기화
            memset(token[i], 0, BUFLEN);
        idx = 0;
        isMatch = FALSE;
        strcpy(buf, time); //원본 보호용 버퍼에 복사
        ptr = strtok(buf, ",");
        while (ptr != NULL) { //목록의 경우 한 개의 목록으로 토큰을 나눠 토큰 배열 생성
            strcpy(token[idx++],ptr); 
            ptr = strtok(NULL, ",");
        }
        for (i = 0; i < idx; i++) 
            if (compareTime(token[i], type)) { //토큰 배열 중 하나라도 만족하면 됨
                isMatch = TRUE;
                break;
            }
        if (isMatch) //목록에서 해당 시간 있는 경우 true 리턴
            return TRUE;
        else //목록 중에서 맞는 시간 없는 경우, 바로 다음 명령어 확인 위해 리턴;
            return FALSE;
    }
    else { //목록 아닌 경우
        if (compareTime(time, type)) { //해당 시간이 맞는 경우 바로 true 리턴
            return TRUE;
        }
        else
            return FALSE;
    }
}