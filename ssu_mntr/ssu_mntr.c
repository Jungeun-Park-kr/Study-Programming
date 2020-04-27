#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/inotify.h>

#define BUFLEN 1024
#define EVENTSIZE (sizeof(struct inotify_event))
#define EVENTBUF (1024*(EVENTSIZE+16))
#define true 1
#define false 0

char checkDir[BUFLEN]; //감시할 디렉토리(check)
char trashDir[BUFLEN]; //trash 디렉토리
char idDir[BUFLEN]; //내 학번 디렉토리
int iOption = false;
int rOption = false;
int lOption = false;
int dOption = false;

void ssu_mntr() {
    pid_t pid;
    char logFile[BUFLEN]; //변경사항 저장 log.txt의 경로
    char saved_path[BUFLEN];
    int logfd;

    memset(saved_path, 0, BUFLEN);
    memset(checkDir, 0, BUFLEN);
    
    getcwd(saved_path, BUFLEN);
    strcpy(checkDir, saved_path);
    
    strcat(checkDir, "/check");
    if (access(checkDir, F_OK)) { //check 디렉토리 없으면 새로생성
        mkdir(checkDir, 0755);   
    }

    sprintf(idDir, "%s/20180753", saved_path);
    if (access(idDir, F_OK)) { //내 학번 디렉토리 없는경우 새로생성
        mkdir(idDir, 0755);
    }
    
    sprintf(logFile, "%s/log.txt", idDir); //create log.txt
    logfd = creat(logFile, 0666);
    if (logfd == NULL) {
        fprintf(stderr, "create error\n");
        exit(1);
    }
    //모니터링을 위한 자식 프로세스 생성
    if ((pid = fork()) < 0) {
        fprintf(stderr, "fork error\n");
        exit(1);
    }
    else if (pid ==0) {
        doMonitor(logFile); //감시 시작, 로그파일에 기록
    }

    doPrompt();

    exit(0);
}

void doPrompt() {
    char command[BUFLEN];
    memset(command, 0, BUFLEN);
    while(1) {
        printf("20180753>");
        scanf("%s", command);
        
        if (strstr(command, "exit") != NULL) { //exit명령어 사용시 종료
            return ;
        }
        else if (strstr(command, "delete") !=NULL) {
            //doDelete();
        }
        else if (strstr(command, "size") != NULL) {
            //doSize();
        }
        else if (strstr(command, "recover") != NULL) {
            //doRecover();
        }
        else if (strstr(command, "tree") != NULL) {
            //doTree();
        }
        else {
            doHelp();
        }
    }
}

void doHelp() {
    printf("Usage : COMMAND [OPTION]\n");
    printf("COMMAND : \n");
    printf(" delete [FILENAME] [END_TIME] [OPTION]      지정한 END_TIME 시간에 자동으로 파일 삭제\n");
    printf("        OPTION : \n");
    printf("          -i                                삭제시 'trash' 디렉토리로 삭제 파일과 정보를 이동시키지 않고 파일 삭제\n");
    printf("          -r                                지정한 시간에 삭제시 삭제 여부 재확인\n");
    printf(" size [FILENAME] [OPTION]                   파일의 경로(상대경로), 파일 크기를 출력\n");
    printf("        OPTION : \n");
    printf("          -d NUMBER                         NUMBER 단계 만큼의 하위 디렉토리까지 출력\n");
    printf(" recover [FILENAME] [OPTION]                trash 디렉토리 내의 파일을 원래 경로로 복구\n");
    printf("        OPTION : \n");
    printf("          -l                                trash 디렉토리 밑에 있는 파일과 삭제 시간들을 삭제시간이 오래된 순으로 출력 후, 명령어 진행\n");
    printf(" tree                                       check 디렉토리의 구조를 tree 형태로 보여줌\n");
    printf(" exit                                       프로그램 종료\n");
    printf(" help                                       명령어 사용법 출력\n");
}
    

void doMonitor(char *logFile) {
    int fd, wd;
    char buf[EVENTBUF];
    char tmp[BUFLEN];
    int size;
    int i = 0;
    FILE *logfile;
    struct stat statbuf;
    time_t intertime;
    //struct tm *tm;
    struct tm tm;
    memset(tmp, 0, BUFLEN);

    fd = inotify_init();
    if (fd < 0) {
        fprintf(stderr, "inotify_init error\n");
        exit(1);
    }

    wd = inotify_add_watch(fd, checkDir, IN_MODIFY|IN_CREATE|IN_DELETE);
    while(1) {
        size = read(fd, buf, EVENTBUF);
        if (size < 0) {
            fprintf(stderr, "read error\n");
            exit(1);
        }

        while (i < size) {
            struct inotify_event * event = (struct inotify_event *)&buf[i];
            
            if(event->len) {
                if(event->mask & IN_CREATE) { //새로운 파일이 생성된경우
                    fprintf(stderr, "create file - %s\n", event->name);
                    logfile = fopen(logFile,"a");
                    if (logfile == NULL) {
                            fprintf(stderr, "fopen error\n");
                            exit(1);
                    }
                    sprintf(tmp, "%s/%s", checkDir, event->name);
                    printf("파일 이름: %s\n", tmp);
                    if((stat(tmp, &statbuf)) < 0) {
                        fprintf(stderr, "stat error\n");
                        //exit(1);
                    }
                    //최종 시간 얻어오기
                    printf("plz\n");
                    intertime = statbuf.st_mtime;
                    tm = *gmtime(&intertime);
                    printf("i'mherer\n");
                    //printf("[%d-%d-%d %d:%d:%d]",tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                    //fprintf(logfile, "[%d-%d-%d %d:%d:%d]",tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                    printf("[%d-%d-%d %d:%d:%d]",tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                    fprintf(logfile, "[%d-%d-%d %d:%d:%d]",tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                    fprintf(logfile, "[%s]\n", event->name);
                    fclose(logfile);
                }
                /*else if (event->mask & IN_DELETE) {
                    fprintf(stderr, "delete file\n");
                    logfile = fopen(logFile,"a");
                    if (logfile == NULL) {
                            fprintf(stderr, "fopen error\n");
                            exit(1);
                    }
                    if((stat(event->name, &statbuf)) < 0) {
                        fprintf(stderr, "stat error\n");
                        //exit(1);
                    }
                    //최종 시간 얻어오기
                    intertime = statbuf.st_mtime;
                    tm = gmtime(intertime);
                    fprintf(logfile, "[%d-%d-%d %d:%d:%d]",tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                    fprintf(logfile, "[%s]", event->name);
                    fclose(logfile);
                }
                else if (event->mask & IN_MODIFY) {
                    fprintf(stderr, "modify file\n");
                    logfile = fopen(logFile,"a");
                    if (logfile == NULL) {
                            fprintf(stderr, "fopen error\n");
                            exit(1);
                    }
                    if((stat(event->name, &statbuf)) < 0) {
                        fprintf(stderr, "stat error\n");
                        //exit(1);
                    }
                    //최종 시간 얻어오기
                    //intertime = statbuf.st_mtime;
                    tm = gmtime(statbuf.st_mtime);
                    fprintf(logfile, "[%d-%d-%d %d:%d:%d]",tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                    fprintf(logfile, "[%s]", event->name);
                    fclose(logfile);
                }*/
                i += EVENTSIZE + event->len;
            }
        }
    }
    //inotify_rm_watch(fd, wd);
    //close(fd);
    
}




