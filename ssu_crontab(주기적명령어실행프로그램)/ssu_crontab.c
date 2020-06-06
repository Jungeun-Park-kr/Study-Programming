#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ssu_crontab.h"

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
    strcpy(workDir, saved_path);
    sprintf(crontabFile, "%s/%s",saved_path, "ssu_crontab_file"); //크론탭 파일 이름저장
    sprintf(crontabLog, "%s/%s", saved_path, "ssu_crontab_log"); //크론탭 로그 파일 이름 저장
    if (access(crontabLog, F_OK)) { //로그파일 없는경우 생성
        if ((fp = fopen(crontabLog, "w+")) <0) {
            fprintf(stderr, "fopen error for %s\n", crontabLog);
            exit(1);
        }
        fclose(fp);
    }
    if(!doPrompt())
        exit(1);

    exit(0);
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
    fputs(prompt, stdout); //프롬프트 출력
    return idx;
}

int doPrompt(void) {
    char errorbuf[BUFLEN], logbuf[BUFLEN], timebuf[BUFLEN];
    int idx = 0, i;
    char *ptr;
    char usrCommand[BUFLEN];
    FILE *logfp;
    time_t cur_time;

    while(1) {
        argc = 0;
        for (i = 0; i < COMMAND_ARGC; i++) {
            memset(argv[i], (char)0, BUFLEN);
        }

        if ((idx = printPrompt()) < 0) { //프롬프트 출력
            fprintf(stderr, "printPrompt() error!\n");
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

        memset(usrCommand, 0, BUFLEN);
        if(strstr(argv[0], "add") != NULL) { //--add명령어--
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
        else if(strstr(argv[0], "remove") != NULL) { //--remove명령어--
            if (argc != 2) { //명령어 인자 개수 확인!
                fprintf(stderr, "usage : remove <REMOVE NUMBER>\n");
                continue;
            }
            int rmvnum = atoi(argv[1]);
            if (rmvnum > idx && rmvnum < 0) { //존재하는 명령어 개수의 범위에 벗어나는 경우 에러처리 후 프롬프트
                fprintf(stderr, "Invalid remove number!");
                continue;
            }

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
        else if (strstr(argv[0], "exit") != NULL) { //--exit 명령어--
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

int checkRange(int num, int type) {
    switch (type) { //각 항목의 type(분,시,일,월,요일)에 따른 범위가 잘 사용되었는지 확인 후, 맞으면 TRUE, 틀리면 FALSE 리턴
        case 1: //분 항목의 범위 확인
        //printf("case 1:");
            if (num < 0 || num > 59) {
                return FALSE;
            }
            else
                break;
        case 2 : //시 항목의 범위 확인
            if (num < 0 || num > 23) {
                return FALSE;
            }
            else
                break;
        case 3 : //일 항목의 범위 확인
            if (num < 1 || num > 31) {
                return FALSE;
            }
            else
                break;
        case 4 : //월 항목의 범위 확인
            if (num < 1 || num > 12) {
                return FALSE;
            }
            else
                break;
        case 5 : //요일 항목의 범위 확인
            if (num < 0 || num > 6) {
                return FALSE;
            }
            else
                break;
    }
    return TRUE;
}

int checkItem(char *item, int type) {
   //해당 항목을 확인해서 올바르게 사용 되었으면 true, 아니면 false를 리턴하는 함수임
   //파라미터 : item - 입력받은 항목, type - 입력받은 항목의 종류(분 0, 시1, 일2, 월3, 요일4 로 지정)
    int num, n1, n2;
    if (strlen(item) < 3) { // *나, 숫자 밖에 없는 경우
        if (!strcmp(item, "*")) { // *만 사용 된 경우 
            return TRUE; //무조건 ok
        }
        else if (isdigit(item[0])) { //숫자인 경우
            num = atoi(item);
            if (checkRange(num, type)) //범위 올바르게 사용된경우 TRUE
                return TRUE;
            else {
                return FALSE;
            }
        }
        else {
            return FALSE;
        }
    }
    else { //숫자가 3이상인 경우
        if (strstr(item, "*/") != NULL) { //*/사용된 경우
            if ((sscanf(item, "*/%d", &num) != 1)) { //문자 뒤 내용이 숫자가 아니면 오류
                return FALSE;
            }
            if (checkRange(num, type)) { //해당 숫자의 범위 확인
                return TRUE; //숫자 잘 사용 되었으면 true
            }
            else
                return FALSE;
        }
        else if (strstr(item, "-") != NULL) {
            if (sscanf(item, "%d-%d", &n1, &n2) != 2) { //숫자 아닌 경우
                return FALSE;
            }
            if (!checkRange(n1, type)) { //n1 범위 확인
                return FALSE;
            }
            if (!checkRange(n2, type)) { //n2 범위 확인
                return FALSE;
            }
            if (strstr(item, "/") != NULL) { //-와 /가 함께 사용된 경우
                if (sscanf(item, "%d-%d/%d", &n1, &n2, &num) != 3) { //형식에 맞게 사용되지 않은 경우 false
                    return FALSE;
                }
                if (abs(n2-n1) < num) { //주기가 올바르게 사용되지 않은 경우
                    return FALSE;
                }
            }
            return TRUE;
        }
        else { //이 외의 기호 사용시 false리턴
            return FALSE;
        }
    }
}

int checkSchedule(void) {
    char *ptr;
    int i = 0, j = 0, idx = 0;
    char token[BUFLEN][BUFLEN];
    char buf[BUFLEN];
    if (argc < 7) 
        return FALSE;
    
    for (i = 1; i < 6; i++) {
        //printf("argv[%d]:%s길이 : %ld\n", i, argv[i],strlen(argv[i]));
        if (strstr(argv[i], ",") != NULL) { // 목록의 경우 : 모든 숫자가 범위 안에 포함되어야 함
            idx = 0;
            strcpy(buf, argv[i]); //문자열 복사 (원본보호)
            ptr = strtok(buf, ",");
            for(j = 0; j < BUFLEN; j++)
                memset(token[j], 0, BUFLEN);
            while (ptr != NULL) { //목록의 경우 한 개의 목록으로 토큰을 나눠 토큰 배열 생성
                strcpy(token[idx++],ptr);
                ptr = strtok(NULL, ",");
            }
            for(j = 0; j < idx; j++) { //생성된 목록 토큰 중 하나라도 틀렸으면 다 틀린것임
                if (!checkItem(token[i], i)) {
                    fprintf(stderr, "---실행주기 입력 오류---\n");
                    return FALSE;
                }
            }
            return TRUE;
        }
        else { //목록아닌 경우 : 단일 항목이므로 바로 확인하면 됨
            if(!checkItem(argv[i], i)) { //올바르게 항목 사용 되었는지 확인
                fprintf(stderr, "---실행주기 입력 오류---\n");
                return FALSE;
            }
        }
    }
    //for loop로 5개 항목 모두 확인 올바르게 되었으면 TRUE 리턴!
    return TRUE;
}