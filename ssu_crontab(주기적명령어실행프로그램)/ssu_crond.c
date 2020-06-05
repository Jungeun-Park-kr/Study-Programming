//명령어를 수행할 디몬 프로세스를 위한 ssu_crond.c
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
    sprintf(crontabFile, "%s/%s",workDir, "ssu_crontab_file"); //크론탭 파일 이름저장
    sprintf(crontabLog, "%s/%s", workDir, "ssu_crontab_log"); //크론탭 로그 파일 이름 저장
    crond_pid = crond_daemon_init();
    chdir(workDir);
    //printf("pid : %d\n", crond_pid);
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
            //printf("현재 시간 : min:%d hour:%d day:%d mon:%d wday:%d", min,hour,day,mon, wday);
            if (doCrond()) { //호출 후 리턴되면
                //printf("--crond수행--\n");
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

    // for (fd=0; fd < maxfd; fd++)
    //     close(fd);

    umask(0);
    chdir("/");
    fd = open("dev/null", O_RDWR);
    dup(0);
    dup(0);
    return pid; //정상 생성된 경우 자식 프로세스인 0이 리턴될 것
}

int doCrond() {
    FILE *cronfp, *logfp;
    char listbuf[BUFLEN], timebuf[BUFLEN], logbuf[BUFLEN], tmpbuf[BUFLEN];
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
        strcpy(tmpbuf, listbuf);
        //printf("읽어온 명령어 : %s\n", tmpbuf);
        command = malloc(sizeof(char) * BUFLEN);
        idx = commandlen = 0;
        isMatch = TRUE;
        ptr = strtok(listbuf, " ");
        while (ptr != NULL) {
            //printf("ptr : %s\n",ptr);
            if (idx > 4) {//실행 주기 부분 모두 저장한 경우 빠져나감
                strcpy(command, tmpbuf + commandlen);
                //fputs(command,stdout);
                break;
            }
            commandlen += (strlen(ptr)+1);
            strcpy(schedule[idx++], ptr); //실행 주기 부분만 schedule에 저장하기
            //printf("주기:%s\n",ptr);
            ptr = strtok(NULL, " ");
        }
        //해당 명령어가 지금 실행되어야 하는지 확인
        for(i = 0; i < 5; i++) {
            //printf("검사시간 : %s\n", schedule[i]);
            if(!checkTime(i, schedule[i])) { //하나라도 시간 틀린 경우 다음 명령어 확인 위해 빠져나감
                //printf("틀린 시간 - scheduel[%d]=%s\n", i, schedule[i]);
                isMatch = FALSE;
                //printf("시간아님\n");
                break;
            }

        }
        if (isMatch) { //시간이 같은 경우 
            printf("시간 같음\n");
            //system()함수로 명령어 실행
            res = system(command);
            printf("res : %d\n", res);
            if (res == -1 || res == 127) { //fork() error, exec계열 error시 로그 안찍음
                fclose(cronfp);
                //printf("system()실패 끝냠\n");
                return TRUE;
            }

            //정상 수행 완료되었으면 로그 쓰기
            ctime_r(&cur_time, timebuf);
            timebuf[19] = 0; //요일 월 일 시간 까지만 출력할것임
            sprintf(logbuf, "[%s] run %s", timebuf, command);
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
    fclose(cronfp);
    //printf("끝냠\n");
    return TRUE;
}

int checkTime(int idx, char *time) { //idx는 실행주기의 각 항목, time은 항목 문자열
    //현재 시간과 항목의 시간이 일치하면 TRUE, 불일치하면 FALSE 리턴
    int num = 0, n1 = 0, n2 = 0;
    char *ptr, *ptr2;
    int i = 0, tmp = 0;
    //printf("strlen : %ld\ntime:%s\n", strlen(time),time);
    switch(idx) {
    case 0: //분인 경우
        //printf("--분(min : %d)--\n",min);
        if ((strlen(time) == 1)||(strlen(time) == 2)) { //항목 길이가 1이나 2면 무조건 숫자여야 함
           // printf("길이 1, 2\n");
            //printf("time : %s\n", time);
            if (!strcmp(time, "*")) { //*만 있는 경우 무조건 맞음
               // printf("*만있음\n");
                return TRUE;
            }
            else if(isdigit(time[0])) { //일단 숫자인지 확인
                //숫자 맞으면 현재 시간과 일치하는지 확인
                //printf("숫자임\n");
                num = atoi(time);
                if (num == min) {
                   // printf("같음\n");
                    return TRUE;
                }
               // printf("다름\n");
                return FALSE;
            }
            return FALSE;
        }
        else { //길이 3 이상인 경우
        //printf("길이 3이상\n");
            if(strstr(time, "-") != NULL) {
                if (sscanf(time, "%d-%d", &n1, &n2) != 2) //숫자 2개 아닌 경우
                    return FALSE;
                //printf("n1 : %d, n2 : %d\n", n1,n2);
                if ((ptr = strstr(time, "/")) != NULL) { // 하이픈이랑 슬래쉬 같이 사용된 경우
                    //printf("-/두개다 쓰임\n");
                    if (sscanf(time, "%d-%d/%d", &n1, &n2, &num) != 3)
                        return FALSE;
                    for (i=1; ; i++) {
                        tmp = n1+(i*num);
                        if (tmp > n2) //tmp가 n2보다 크면 범위 벗어나므로 break
                            break;
                        if (min == tmp) //tmp가 min과 같으면 맞음
                            return TRUE;
                    }
                }
                else { //범위에 맞는 숫자인 경우
                //printf("-만 쓰임\n");
                    if ((n1 <= min) && (min <= n2))
                        return TRUE;
                }
                return FALSE;
            }
            else if (strstr(time, ",") != NULL) {
                ptr = strtok(time, ",");
                while (ptr != NULL) { //목록의 숫자가 현재 시간과 일치하는지 확인
                    num = atoi(ptr);
                    if (num == min)
                        return TRUE;
                    if(strstr(ptr, "-") != NULL) { //목록 안에 범위 있는 경우
                        if (sscanf(ptr, "%d-%d", &n1, &n2) != 2) //숫자 2개 아닌 경우
                            return FALSE;
                    
                        if ((ptr2 = strstr(ptr, "/")) != NULL) { // 하이픈이랑 슬래쉬 같이 사용된 경우
                            if (sscanf(time, "%d-%d/%d", &n1, &n2, &num) != 3) {
                                return FALSE;
                                tmp = 0;
                                for (i=1; ; i++) {
                                    tmp = n1+(i*num);
                                    if (tmp > n2) //tmp가 n2보다 크면 범위 벗어나므로 break
                                        break;
                                    if (min == tmp) //tmp가 시간과 같으면 맞음
                                        return TRUE;
                                }
                            }
                        }
                        else { //범위에 맞는 숫자인 경우
                            if (n1 <= min && min <= n2)
                                return TRUE;
                        }
                    }
                    ptr = strtok(NULL, ",");
                }
                return FALSE; //목록 끝까지 못찾은 경우
            }
            else if (strstr(time, "*/") != NULL) {
                if (sscanf(time, "*/%d", &num) != 1) { //문자 뒤 내용이 숫자가 아니면 오류
                    return FALSE;
                }
                //printf("num : %d\n",num);
                if (min % num == 1) { //현재시간/num의 나머지가 0이면 현재 시간에 해당함! 
                    return TRUE;
                }
                return FALSE;
            }
            else //그 외의 다른 경우 다 틀림
                return FALSE;
        }
        return FALSE;

    case 1: //시인 경우
   // printf("--시--\n");
        if ((strlen(time) == 1)||(strlen(time) == 2)) { //항목 길이가 1이나 2면 무조건 숫자여야 함
            if (!strcmp(time, "*")) { //*만 있는 경우 무조건 맞음
                //printf("*만있음\n");
                return TRUE;
            }
            else if(isdigit(time[0])) { //일단 숫자인지 확인
                //숫자 맞으면 현재 시간과 일치하는지 확인
                num = atoi(time);
                if (num == hour)
                    return TRUE;
            }
        }
        else { //길이 3 이상인 경우
            if(strstr(time, "-") != NULL) {
                if (sscanf(time, "%d-%d", &n1, &n2) != 2) //숫자 2개 아닌 경우
                    return FALSE;
            
                if ((ptr = strstr(time, "/")) != NULL) { // 하이픈이랑 슬래쉬 같이 사용된 경우
                    if (sscanf(time, "%d-%d/%d", &n1, &n2, &num) != 3) {
                        return FALSE;
                        tmp = 0;
                        for (i=1; ; i++) {
                            tmp =  n1+(i*num);
                            if (tmp > n2) //tmp가 n2보다 크면 범위 벗어나므로 break
                                break;
                            if (hour == tmp) //tmp가 시간과 같으면 맞음
                                return TRUE;
                        }
                    }
                }
                else { //범위에 맞는 숫자인 경우
                    if ((n1 <= hour) && (hour <= n2))
                        return TRUE;
                }  
            }
            else if (strstr(time, ",") != NULL) {
                ptr = strtok(time, ",");
                while (ptr != NULL) { //목록의 숫자가 현재 시간과 일치하는지 확인
                    num = atoi(ptr);
                    if (num == hour)
                        return TRUE;
                    if(strstr(ptr, "-") != NULL) { //목록 안에 범위 있는 경우
                        if (sscanf(ptr, "%d-%d", &n1, &n2) != 2) //숫자 2개 아닌 경우
                            return FALSE;
                    
                        if ((ptr2 = strstr(ptr, "/")) != NULL) { // 하이픈이랑 슬래쉬 같이 사용된 경우
                            if (sscanf(time, "%d-%d/%d", &n1, &n2, &num) != 3) {
                                return FALSE;
                                tmp = 0;
                                for (i=1; ; i++) {
                                    tmp =  n1+(i*num);
                                    if (tmp > n2) //tmp가 n2보다 크면 범위 벗어나므로 break
                                        break;
                                    if (hour == tmp) //tmp가 시간과 같으면 맞음
                                        return TRUE;
                                }
                            }
                        }
                        else { //범위에 맞는 숫자인 경우
                            if (n1 <= hour && hour <= n2)
                                return TRUE;
                        }
                    }
                    ptr = strtok(NULL, ",");
                }
                return FALSE; //목록 끝까지 못찾은 경우
            }
            else if (strstr(time, "*/") != NULL) {
                ptr = strtok(time, "*/");
                if (!isdigit(time[0])) { //문자 뒤 내용이 숫자가 아니면 오류
                    return FALSE;
                }
                num = atoi(ptr);
                if (hour % num == 1) { //현재시간/num의 나머지가 0이면 현재 시간에 해당함! 
                    return TRUE;
                }
                return FALSE;
            }
            else //그 외의 다른 경우 다 틀림
                return FALSE;
        }
        return FALSE;

    case 2: //일인 경우
   // printf("--일--\n");
        if ((strlen(time) == 1)||(strlen(time) == 2)) { //항목 길이가 1이나 2면 무조건 숫자여야 함
            if (!strcmp(time, "*")) { //*만 있는 경우 무조건 맞음
                //printf("*만있음\n");
                return TRUE;
            }
            else if (!strcmp(time, "*")) { //*만 있는 경우 무조건 맞음
                //printf("*만있음\n");
                return TRUE;
            }
            else if(isdigit(time[0])) { //일단 숫자인지 확인
                //숫자 맞으면 현재 시간과 일치하는지 확인
                num = atoi(time);
                if (num == day)
                    return TRUE;
            }
            return FALSE;
        }
        else { //길이 3 이상인 경우
            if(strstr(time, "-") != NULL) {
                if (sscanf(time, "%d-%d", &n1, &n2) != 2) //숫자 2개 아닌 경우
                    return FALSE;
            
                if ((ptr = strstr(time, "/")) != NULL) { // 하이픈이랑 슬래쉬 같이 사용된 경우
                    if (sscanf(time, "%d-%d/%d", &n1, &n2, &num) != 3) {
                        return FALSE;
                        tmp = 0;
                        i=0;
                        for (i=(num-1); ; i += (num-1)) {
                            
                            tmp =  n1+i;
                            if (tmp > n2) //tmp가 n2보다 크면 범위 벗어나므로 break
                                break;
                            if (day == tmp) { //tmp가 시간과 같으면 맞음
                                printf("같음..\n");
                                return TRUE;
                            }
                        }
                    }
                }
                else { //범위에 맞는 숫자인 경우
                    if (n1 <= day && day <= n2)
                        return TRUE;
                }
            }
            else if (strstr(time, ",") != NULL) {
                ptr = strtok(time, ",");
                while (ptr != NULL) { //목록의 숫자가 현재 시간과 일치하는지 확인
                    num = atoi(ptr);
                    if (num == day)
                        return TRUE;
                    if(strstr(ptr, "-") != NULL) { //목록 안에 범위 있는 경우
                        if (sscanf(ptr, "%d-%d", &n1, &n2) != 2) //숫자 2개 아닌 경우
                            return FALSE;
                    
                        if ((ptr2 = strstr(ptr, "/")) != NULL) { // 하이픈이랑 슬래쉬 같이 사용된 경우
                            if (sscanf(time, "%d-%d/%d", &n1, &n2, &num) != 3) {
                                return FALSE;
                                tmp = 0;
                                for (i=1; ; i++) {
                                    tmp =  n1+(i*num);
                                    if (tmp > n2) //tmp가 n2보다 크면 범위 벗어나므로 break
                                        break;
                                    if (day == tmp) //tmp가 시간과 같으면 맞음
                                        return TRUE;
                                }
                            }
                        }
                        else { //범위에 맞는 숫자인 경우
                            if (n1 <= day && day <= n2)
                                return TRUE;
                        }
                    }
                    ptr = strtok(NULL, ",");
                }
                return FALSE; //목록 끝까지 못찾은 경우
            }
            else if (strstr(time, "*/") != NULL) {
                ptr = strtok(time, "*/");
                if (!isdigit(time[0])) { //문자 뒤 내용이 숫자가 아니면 오류
                    return FALSE;
                }
                num = atoi(ptr);
                if (day % num == 1) { //현재시간/num의 나머지가 0이면 현재 시간에 해당함! 
                    return TRUE;
                }
                return FALSE;
            }
            else //그 외의 다른 경우 다 틀림
                return FALSE;
        }
        return FALSE;

    case 3: //월인 경우
    //printf("--월--\n");
        if ((strlen(time) == 1)||(strlen(time) == 2)) { //항목 길이가 1이나 2면 무조건 숫자여야 함
            if (!strcmp(time, "*")) { //*만 있는 경우 무조건 맞음
                //printf("*만있음\n");
                return TRUE;
            }
            else if(isdigit(time[0])) { //일단 숫자인지 확인
                //숫자 맞으면 현재 시간과 일치하는지 확인
                num = atoi(time);
                if (num == mon)
                    return TRUE;
            }
            return FALSE;
        }
        else { //길이 3 이상인 경우
            if(strstr(time, "-") != NULL) {
               if (sscanf(time, "%d-%d", &n1, &n2) != 2) //숫자 2개 아닌 경우
                    return FALSE;
            
                if ((ptr = strstr(time, "/")) != NULL) { // 하이픈이랑 슬래쉬 같이 사용된 경우
                    if (sscanf(time, "%d-%d/%d", &n1, &n2, &num) != 3) {
                        return FALSE;
                        tmp = 0;
                        for (i=1; ; i++) {
                            tmp =  n1+(i*num);
                            if (tmp > n2) //tmp가 n2보다 크면 범위 벗어나므로 break
                                break;
                            if (mon == tmp) //tmp가 시간과 같으면 맞음
                                return TRUE;
                        }
                    }
                }
                else { //범위에 맞는 숫자인 경우
                    if (n1 <= mon && mon <= n2)
                        return TRUE;
                }  
            }
            else if (strstr(time, ",") != NULL) {
                ptr = strtok(time, ",");
                while (ptr != NULL) { //목록의 숫자가 현재 시간과 일치하는지 확인
                    num = atoi(ptr);
                    if (num == mon)
                        return TRUE;
                    if(strstr(ptr, "-") != NULL) { //목록 안에 범위 있는 경우
                        if (sscanf(ptr, "%d-%d", &n1, &n2) != 2) //숫자 2개 아닌 경우
                            return FALSE;
                    
                        if ((ptr2 = strstr(ptr, "/")) != NULL) { // 하이픈이랑 슬래쉬 같이 사용된 경우
                            if (sscanf(time, "%d-%d/%d", &n1, &n2, &num) != 3) {
                                return FALSE;
                                tmp = 0;
                                for (i=1; ; i++) {
                                    tmp =  n1+(i*num);
                                    if (tmp > n2) //tmp가 n2보다 크면 범위 벗어나므로 break
                                        break;
                                    if (mon == tmp) //tmp가 시간과 같으면 맞음
                                        return TRUE;
                                }
                            }
                        }
                        else { //범위에 맞는 숫자인 경우
                            if (n1 <= mon && mon <= n2)
                                return TRUE;
                        }
                    }
                    ptr = strtok(NULL, ",");
                }
                return FALSE; //목록 끝까지 못찾은 경우
            }
            else if (strstr(time, "*/") != NULL) {
                ptr = strtok(time, "*/");
                if (!isdigit(time[0])) { //문자 뒤 내용이 숫자가 아니면 오류
                    return FALSE;
                }
                num = atoi(ptr);
                if (mon % num == 1) { //현재시간/num의 나머지가 0이면 현재 시간에 해당함! 
                    return TRUE;
                }
                return FALSE;
            }
            else //그 외의 다른 경우 다 틀림
                return FALSE;
        }
        return FALSE;

    case 4: //요일인 경우
    //printf("--요일--\n");
        if ((strlen(time) == 1)||(strlen(time) == 2)) { //항목 길이가 1이나 2면 무조건 숫자여야 함
            if (!strcmp(time, "*")) { //*만 있는 경우 무조건 맞음
                //printf("*만있음\n");
                return TRUE;
            }
            else if(isdigit(time[0])) { //일단 숫자인지 확인
                //숫자 맞으면 현재 시간과 일치하는지 확인
                num = atoi(time);
                if (num == wday)
                    return TRUE;
            }
            return FALSE;
        }
        else { //길이 3 이상인 경우
            if(strstr(time, "-") != NULL) {
               if (sscanf(time, "%d-%d", &n1, &n2) != 2) //숫자 2개 아닌 경우
                    return FALSE;
            
                if ((ptr = strstr(time, "/")) != NULL) { // 하이픈이랑 슬래쉬 같이 사용된 경우
                    if (sscanf(time, "%d-%d/%d", &n1, &n2, &num) != 3) {
                        return FALSE;
                        tmp = 0;
                        for (i=1; ; i++) {
                            tmp =  n1+(i*num);
                            if (tmp > n2) //tmp가 n2보다 크면 범위 벗어나므로 break
                                break;
                            if (wday == tmp) //tmp가 시간과 같으면 맞음
                                return TRUE;
                        }
                    }
                }
                else { //범위에 맞는 숫자인 경우
                    if (n1 <= wday && wday <= n2)
                        return TRUE;
                } 
            }
            else if (strstr(time, ",") != NULL) {
                ptr = strtok(time, ",");
                while (ptr != NULL) { //목록의 숫자가 현재 시간과 일치하는지 확인
                    num = atoi(ptr);
                    if (num == wday)
                        return TRUE;
                    ptr = strtok(NULL, ",");
                    if(strstr(ptr, "-") != NULL) { //목록 안에 범위 있는 경우
                        if (sscanf(ptr, "%d-%d", &n1, &n2) != 2) //숫자 2개 아닌 경우
                            return FALSE;
                    
                        if ((ptr2 = strstr(ptr, "/")) != NULL) { // 하이픈이랑 슬래쉬 같이 사용된 경우
                            if (sscanf(time, "%d-%d/%d", &n1, &n2, &num) != 3) {
                                return FALSE;
                                tmp = 0;
                                for (i=1; ; i++) {
                                    tmp =  n1+(i*num);
                                    if (tmp > n2) //tmp가 n2보다 크면 범위 벗어나므로 break
                                        break;
                                    if (wday == tmp) //tmp가 시간과 같으면 맞음
                                        return TRUE;
                                }
                            }
                        }
                        else { //범위에 맞는 숫자인 경우
                            if (n1 <= wday && wday <= n2)
                                return TRUE;
                        }
                    }
                }
                return FALSE; //목록 끝까지 못찾은 경우
            }
            else if (strstr(time, "*/") != NULL) {
                ptr = strtok(time, "*/");
                if (!isdigit(time[0])) { //문자 뒤 내용이 숫자가 아니면 오류
                    return FALSE;
                }
                num = atoi(ptr);
                if (wday % num == 1) { //현재시간/num의 나머지가 0이면 현재 시간에 해당함! 
                    return TRUE;
                }
                return FALSE;
            }
            else //그 외의 다른 경우 다 틀림
                return FALSE;
        }
        return FALSE;
    }
    return FALSE;
}


