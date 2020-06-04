
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
#include <ctype.h>
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
char logCommand[BUFLEN];
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
        fclose(fp);
    }
    //프롬프트 실행
    if(!doPrompt())
        exit(1);

}

int printPrompt(void) {
    char *prompt = "\n 20180753> ";
    char listbuf[BUFLEN], idxbuf[BUFLEN];
    int idx = 0;
    char *ptr;
    if (!access(crontabFile, F_OK)) { //파일이 이미 존재하는 경우
        if ((cronfp = fopen(crontabFile, "r")) == NULL) {
            fprintf(stderr, "fopen error for %s\n", crontabFile);
            return -1;
        }
        while(fgets(listbuf, BUFLEN, cronfp) != NULL) { //저장된 모든 명령어 출력
            sprintf(idxbuf, " %d. ", idx++);
            fputs(idxbuf,stdout); //현재 명령어 번호 출력
            if (fputs(listbuf, stdout) == EOF) { //뿌려주기
                fprintf(stderr, "puts error\n");
                return -1;
            }
        }
        fclose(cronfp);
        fputs(prompt, stdout);
        return idx;
    }
    //파일 없는 경우 제어 바로 여기로 옴
    printf("파일없음\n");
    fputs(prompt, stdout); //프롬프트 출력
    return idx;
}

int doPrompt(void) {
    char errorbuf[BUFLEN], logbuf[BUFLEN], timebuf[BUFLEN];
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

        if ((idx = printPrompt()) < 0) { //프롬프트 출력
            fprintf(stderr, "printPrompt error!\n");
            return FALSE;
        }
        fgets(Command, BUFLEN, stdin);
        while (!strcmp(Command, "\n")) { //엔터만 받으면 다시 재입력
            printPrompt();
            fgets(Command, BUFLEN, stdin);
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
            
            if (!checkSchedule()) { //실행주기 올바르게 입력되었는지 확인
                continue; //안된경우 프롬프트 출력
                
            }
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
                sprintf(logbuf, "[%s] add %s", timebuf, logCommand);
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
            if (argc != 2) { //명령어 인자 개수 확인!
                fprintf(stderr, "usage : remove <REMOVE NUMBER>\n");
                continue;
            }
            int rmvnum = atoi(argv[1]);
            if (rmvnum > idx && rmvnum < 0) { //존재하는 명령어 개수의 범위에 벗어나는 경우 에러처리 후 프롬프트
                fprintf(stderr, "Invalid remove number!");
                continue;
            }
            //strcat(usrCommand, argv[0]);
            if (!doRemove(rmvnum)) //삭제 실패
                continue;
            else { //정상적으로 삭제된 경우 -> 로그파일에 기록
                time(&cur_time);
	            ctime_r(&cur_time, timebuf);
                timebuf[19] = 0; //요일 월 일 시간 까지만 출력할것임
                sprintf(logbuf, "[%s] remove %s", timebuf, logCommand); //해당 명령어 
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
            //printf("--exit 명령어--\n");
            break;
        }
        else { //이외의 명령어 -> 프롬프트 출력'
            snprintf(errorbuf, strlen(argv[0]), "%s", argv[0]);
            fprintf(stderr, "<%s> command not exist!\n", errorbuf);
            continue;
        }
    }
    return TRUE;
}

char *rtrim(char *str) { //문자열의 우측에 존재하는 공백을 제거 후 리턴
	char tmp[BUFLEN];
	char *end;

	strcpy(tmp, str);
	end = tmp + strlen(tmp) - 1;
	while (end != str && isspace(*end))
		--end;

	*(end + 1) = '\0';
	str = tmp;
	return str;
}

int doAdd(char *command) { //파라미터로 넘어온 명령어를 ssu_crontab_file 파일에 추가한다.
    FILE *fp;
    if ((fp = fopen(crontabFile, "a")) == NULL) {
        fprintf(stderr, "fopen error for %s\n", crontabFile);
        return FALSE;
    }
    command[strlen(command)-1] = '\0';
    strcpy(logCommand, command); //로그에 작성할 추가한 명령어 저장
    if (fputs(command, fp) < 0) { //내용 추가
        fprintf(stderr, "fupts error\n");
        return FALSE;
    }
    fclose(fp);
    return TRUE;
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
            strcpy(logCommand, listbuf); //로그에 작성할 삭제된 명령어 저장
            idx++;
            continue;
        }
        if (fputs(listbuf, tmpfp) < 0) {
            fprintf(stderr, "fputs error\n");
            return FALSE;
        }
        idx++;
    }
    fclose(cronfp);
    fclose(tmpfp);
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

int checkRange(int i, int num) {
    switch (i) {
        case 1: //분 항목의 범위 확인
        //printf("case 1:");
            if (num < 0 || num > 59) {
                    //fprintf(stderr, "실행주기 입력 오류\n");
                return FALSE;
            }
            else
                break;
        case 2 : //시 항목의 범위 확인
            if (num < 0 || num > 23) {
                    //fprintf(stderr, "실행주기 입력 오류\n");
                return FALSE;
            }
            else
                break;
        case 3 : //일 항목의 범위 확인
            if (num < 1 || num > 31) {
                    //fprintf(stderr, "실행주기 입력 오류\n");
                return FALSE;
            }
            else
                break;
        case 4 : //월 항목의 범위 확인
            if (num < 1 || num > 12) {
                    //fprintf(stderr, "실행주기 입력 오류\n");
                return FALSE;
            }
            else
                break;
        case 5 : //요일 항목의 범위 확인
            if (num < 0 || num > 6) {
                    //fprintf(stderr, "실행주기 입력 오류\n");
                return FALSE;
            }
            else
                break;
    }
    return TRUE;
}

int checkSchedule(void) {
    char *ptr;
    int i = 0;
    int num=0, n1 =0, n2=0;

    if (argc < 7) 
        return FALSE;
    
    for (i=1; i<6; i++) {
        //printf("argv[%d]:%s길이 : %ld\n", i, argv[i],strlen(argv[i]));
        if ((strlen(argv[i]) == 1) || (strlen(argv[i]) == 2)) { //해당 항목의 길이가 1이나 2인 경우
            if(!strcmp(argv[i], "*")) { //'*'만 있어야 함
                continue;
            }
            else if (isdigit(argv[i][0])) { //숫자만 있어야 함
                //숫자인 경우 범위 확인 필요
                num = atoi(argv[i]);
                if (!checkRange(i, num)) { //항복의 숫자 범위 확인
                    fprintf(stderr, "실행주기 입력 오류\n");
                    return FALSE;
                }
                if ((strlen(argv[i]) == 2) && num < 10) { //길이가 2인데 숫자 한자리면 오류
                    fprintf(stderr, "실행주기 입력 오류\n");
                    return FALSE;
                }
            }
            else {
                printf("숫자도 별도 아님\n");
                fprintf(stderr, "실행주기 입력 오류\n");
                return FALSE;
            }
            continue;
        }
        else { //길이가 3 이상, 1미만인 경우
            if (strstr(argv[i], "-") != NULL) { //범위 지정인 경우 : 숫자-숫자 구조여야 함
                ptr = strtok(argv[i], "-");
                if (!isdigit(ptr)) {
                    fprintf(stderr, "실행주기 입력 오류\n");
                    return FALSE;
                }
                n1 = atoi(ptr);
                if (!checkRange(i, n1)) {
                    fprintf(stderr, "실행주기 입력 오류\n");
                    return FALSE;
                }
                ptr = strtok(NULL, "-"); //하이픈 다음 숫자 가져옴
                if (!isdigit(ptr)) {
                    fprintf(stderr, "실행주기 입력 오류\n");
                    return FALSE;
                }
                n2 = atoi(ptr);
                if (!checkRange(i, n2)) {
                    fprintf(stderr, "실행주기 입력 오류\n");
                    return FALSE;
                }
                if (n1 > n2 || n1 == n2) { //숫자1-숫자2 구조중 두 숫자가 같거나, 앞 숫자가 더 큰경우
                    fprintf(stderr, "실행주기 입력 오류\n");
                    return FALSE;
                }
            }
            else if (strstr(argv[i], "*/") != NULL) { //건너뛰는 경우1 : */숫자 구조여야 함
                ptr = strtok(argv[i], "*/");
                if (!isdigit(ptr[0])) { //문자 뒤 내용이 숫자가 아니면 오류
                    fprintf(stderr, "실행주기 입력 오류\n");
                    return FALSE;
                }
                num = atoi(ptr);
                if (!checkRange(i, num)) {
                    fprintf(stderr, "실행주기 입력 오류\n");
                    return FALSE;
                }
            }
            else if (strstr(argv[i], ",") != NULL) { // 목록의 경우 : 모든 숫자가 범위 안에 포함되어야 함
                ptr = strtok(argv[i], ",");
                while (ptr != NULL) { //목록의 경우 모든 숫자가 범위에 속하는지 확인
                    num = atoi(ptr);
                    if (!checkRange(i, num)) { //항복의 숫자 범위 확인
                        fprintf(stderr, "실행주기 입력 오류\n");
                        return FALSE;
                    }
                    ptr = strtok(NULL, ",");
                }
            }
            else if ((strstr(argv[i], "*/") == NULL) && (strstr(argv[i], "/") != NULL)) { //건너뛰는 경우2 : 숫자/숫자 구조여야 함 (-과 같이 쓰일 수 있으므로 if문으로)
                ptr = strtok(argv[i], "/");
                if (isdigit(ptr[0])) { //숫자인지 확인
                    fprintf(stderr, "실행주기 입력 오류\n");
                    return FALSE;
                }
                ptr = strtok(NULL, "/"); 
                if (isdigit(ptr[0])) { // '/'다음 문자도 숫자인지 확인
                    fprintf(stderr, "실행주기 입력 오류\n");
                    return FALSE;
                }
            }
            else { //그 외의 문자가 사용된 경우
                fprintf(stderr, "실행주기 입력 오류\n");
                return FALSE;
            }
            
            
        }
    }
    return TRUE;
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