#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>


#define true 1
#define false 0
#define BUFLEN 1024
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENTLEN (1024 * (EVENT_SIZE+16))
char checkDir[BUFLEN];
char stdidDir[BUFLEN];
char trashDir[BUFLEN];

void ssu_mntr() {
    pid_t dmpid;
    FILE *logfp;
    int logfd;
    char logFile[BUFLEN];
    char saved_path[BUFLEN];

    getcwd(saved_path, BUFLEN);
    strcpy(checkDir, saved_path);

    strcat(checkDir, "/check");
    if (access(checkDir, F_OK)) { //check 디렉토리 없으면 새로생성
        mkdir(checkDir, 0755);
    }
    
    sprintf(stdidDir, "%s/20180753", saved_path);
    if (access(stdidDir, F_OK)) { //내 학번 디렉토리 없는경우 새로생성
        mkdir(stdidDir, 0755);
    }
    
    sprintf(logFile, "%s/log.txt", stdidDir); //create log.txt

    /*logfd = creat(logFile, 0666);
    if (logfd == NULL) {
        fprintf(stderr, "create error\n");
       exit(1);
    }*/
    logfp = fopen(logFile, "w"); 
    fclose(logfp);

    if ((dmpid = monitor_deamon_init()) < 0) {
        fprintf(stderr, "monitor_deamon_init faled\n");
        exit(1);
    }
    
    else if (dmpid == 0) { //생성된 데몬 프로세스
        do_Monitor(logFile);
    }
    
    do_Prompt();

    return;
}

pid_t monitor_deamon_init() {
    pid_t pid;
    int fd, maxfd;

    if ((pid = fork()) < 0) {
        fprintf(stderr, "fork error\n");
        exit(1);
    }
    else if (pid != 0) //부모프로세스
        return pid;

    //디몬 코딩 규칙에 따라 프로세스 생성
    setsid();
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    maxfd = getdtablesize();

    for (fd = 0; fd < maxfd; fd++)
        close(fd);

    umask(0);
    chdir("/");
    fd = open("/dev/null", O_RDWR);
    dup(0);
    dup(0);

    return pid;
}

void do_Monitor(char *logFile){
    int ifd, wd;
    FILE *logfp;
    char buf[EVENTLEN];
    struct stat statbuf;
    time_t intertime;
    struct tm tm;
    int len, i = 0;
    char tmp[BUFLEN];

    if ((logfp = fopen(logFile, "a+")) == NULL) {
        fprintf(stderr,"fopen error\n");
        exit(1);
    }
    char sibal[1024] = "dhoaseifdasofjeaf";
    fprintf(logfp, "%s\n", sibal);
    
    ifd = inotify_init();
    if (ifd == -1) {
        fprintf(stderr, "inotify_init error\n");
        return;
    }
    
    wd = inotify_add_watch(ifd, checkDir, IN_CREATE|IN_DELETE|IN_MODIFY);
    if(wd == -1) {
        fprintf(stderr, "inotify_add_watch error\n");
        return;
    }

    while(1) {
        i = 0;
        len = read(ifd, buf, EVENTLEN);
        while(i < len) {
            struct inotify_event *event = (struct inotify_event *)&buf[i];
            if(event->name[0] != '.') {
                if (event->mask & IN_CREATE) {
                    sprintf(tmp, "%s/%s", checkDir, event->name);
                    stat(tmp, &statbuf);
                    intertime = statbuf.st_atime;
                    tm = *gmtime(&intertime);
                    logfp = fopen(logFile, "a");
                    fprintf(logfp, "[%d-%02d-%02d %02d:%02d:%02d]", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                    fprintf(logfp, "[create_%s]\n", event->name);
                    fclose(logfp);
                }
                else if (event->mask & IN_DELETE) {
                    sprintf(tmp, "%s/%s", checkDir, event->name);
                    stat(tmp, &statbuf);
                    intertime = statbuf.st_atime;
                    tm = *gmtime(&intertime);
                    logfp = fopen(logFile, "a");
                    fprintf(logfp, "[%d-%02d-%02d %02d:%02d:%02d]", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                    fprintf(logfp, "[delete_%s]\n", event->name);
                    fclose(logfp);
                }
                else if (event->mask & IN_MODIFY) {
                    sprintf(tmp, "%s/%s", checkDir, event->name);
                    stat(tmp, &statbuf);
                    intertime = statbuf.st_atime;
                    tm = *gmtime(&intertime);  
                    logfp = fopen(logFile, "a");
                    fprintf(logfp, "[%d-%02d-%02d %02d:%02d:%02d]", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                    fprintf(logfp, "[modify_%s]\n", event->name);
                    fclose(logfp);
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }
    inotify_rm_watch(ifd, wd);
    close(ifd);
    fclose(logfp);
    return;
}

void do_Prompt() {
    char command[BUFLEN];
    memset(command, 0, BUFLEN);
    while(1) {
        printf("20180753>");
        scanf("%s", command);

        if (strstr(command, "exit") != NULL) { //exit명령어 사용시 종료
            //kill(deamonpid, SIGKILL);
            break;
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
    return;
}

void doHelp() {
    printf("Usage : COMMAND [OPTION]\n");
    printf("COMMAND : \n");
    printf(" delete [FILENAME] [END_TIME] [OPTION]      지정한 END_TIME 시간에 자동으로 파일 삭제\n");
    printf("                    OPTION : \n");
    printf("                            -i              삭제시 'trash' 디렉토리로 삭제 파일과 정보를 이동시키지 않고 파일 삭제\n");
    printf("                            -r              지정한 시간에 삭제시 삭제 여부 재확인\n\n");
    printf(" size [FILENAME] [OPTION]                   파일의 경로(상대경로), 파일 크기를 출력\n");
    printf("                    OPTION : \n");
    printf("                            -d NUMBER       NUMBER 단계 만큼의 하위 디렉토리까지 출력\n\n");
    printf(" recover [FILENAME] [OPTION]                trash 디렉토리 내의 파일을 원래 경로로복구\n");
    printf("                    OPTION : \n");
    printf("                            -l              trash 디렉토리 밑에 있는 파일과 삭제 시간들을 삭제시간이 오래된 순으로 출력 후, 명령어 진행\n\n");
    printf(" tree                                       check 디렉토리의 구조를 tree 형태로 보여줌\n\n");
    printf(" exit                                       프로그램 종료\n\n");
    printf(" help                                       명령어 사용법 출력\n\n");
}

