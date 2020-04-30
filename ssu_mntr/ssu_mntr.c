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
char filesDir[BUFLEN];
char infoDir[BUFLEN];
char workDir[BUFLEN];

void ssu_mntr() {
    pid_t dmpid;
    FILE *logfp;
    int logfd;
    char logFile[BUFLEN];
    char saved_path[BUFLEN];

    getcwd(saved_path, BUFLEN);
    strcpy(checkDir, saved_path);
    strcpy(workDir, saved_path);
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
            if(event->len && (event->name[0] != '.')) {
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
    char tmp[BUFLEN];
    char argv[7][BUFLEN];
    int argc;
    int i;
    memset(command, 0, BUFLEN);
    while(1) {
        argc = 0;
        for(i = 0; i < 7; i++) {
            memset(argv[i], 0, BUFLEN);
        }

        printf("20180753>");
        scanf("%s", command);

        if (strstr(command, "exit") != NULL) { //exit명령어 사용시 종료
            //kill(deamonpid, SIGKILL);
            break;
        }
        else if (strstr(command, "delete") !=NULL) {
            char *ptr = strtok(command, " ");
            while (ptr != NULL) {
                strcpy(argv[argc++], ptr);
                ptr = strtok(NULL, " ");
            }
            if(argc < 3) {
                fprintf(stderr, "usage : DELETE [FILENAME] [END_TIME] [OPTION]\n");
                return;
            }
            doDelete(argc, *argv[5]);
        }
        else if (strstr(command, "size") != NULL) {
            doSize();
        }
        else if (strstr(command, "recover") != NULL) {
            doRecover();
        }
        else if (strstr(command, "tree") != NULL) {
            doTree();
        }
        else {
            doHelp();
        }
    }
    printf("모니터링을 종료합니다.\n");
    return;
}

long getDirSize(char *dirName) {
    struct dirent *dirp;
    struct stat statbuf;
    DIR *dp;
    char tmp[BUFLEN];
    int type;
    off_t dsize = 0;

    if ((dp = opendir(dirName)) == NULL) {
        fprintf(stderr, "opendir error for %s\n", dirName);
        return 0;
    }

    while ((dirp = readdir(dp)) != NULL) {
        if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
            continue;
        strcpy(tmp, dirName);
        strcat(tmp, "/");
        strcat(tmp, dirp->d_name);
        
        if (stat(tmp, &statbuf) < 0) {
            fprintf(stderr, "stat error\n");
            return 0;
        }

        if((statbuf.st_mode & S_IFMT) == S_IFREG) {
            dsize += statbuf.st_size;
        }
        else if((statbuf.st_mode & S_IFMT) == S_IFDIR) {
            dsize += getDirSize(tmp);
        }
    }
    return dsize;
}
int compTime(struct tm t1, struct tm t2) { //더 오래된게 리턴
    if (t1.tm_year+1900 < t2.tm_year+1900)
        return true;
    else if(t1.tm_year+1900 > t2.tm_year+1900)
        return false;

    if (t1.tm_mon+1 < t2.tm_mon+1)
        return true;
    else if(t1.tm_mon+1 > t2.tm_mon+1)
        return false;

    if (t1.tm_mday < t2.tm_mday)
        return true;
    else if(t1.tm_mday > t2.tm_mday) 
        return false;
}


void findOldFile(char *dirName, char *oldest, struct tm oldtm) {
    struct dirent *dirp;
    DIR *dp;
    struct stat statbuf;
    char fname[BUFLEN];
    time_t newtime;
    struct tm newtm;

    
    if ((dp = opendir(dirName)) == NULL) {
        fprintf(stderr, "opendir error\n");
        return;
    }
    while ((dirp = readdir(dp)) != NULL) {
        if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
            continue;
        
        strcpy(fname, dirName);
        strcat(fname, "/");
        strcat(fname, dirp->d_name);
        
        if (stat(fname, &statbuf) < 0) {
            fprintf(stderr, "stat error\n");
            return;
        }

        if ((statbuf.st_mode & S_IFMT) == S_IFREG) {
            newtime = statbuf.st_mtime;
            newtm = *gmtime(&newtime);
            if(!compTime(oldtm, newtm)) { //newtm이 더 오래되면 갱신
                strcpy(oldest, fname);
                oldtm = newtm;
            }
        }
        else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
            findOldFile(fname, oldest, oldtm);
        }
    }
    return ;
}
int isExist(char *fname, char *pathname){
    //checkDir내에 fname파일 존재시 true(1)리턴, 없으면 false(0)리턴
    struct dirent *dirp;
    struct stat statbuf;
    DIR *dp;
    char tmp[BUFLEN];
    char tmpname[BUFLEN];

    chdir(workDir);
    if ((dp = opendir("/check")) == NULL) {
        fprintf(stderr, "opendir error for checkDir\n");
        return false;
    }
    while ((dirp = readdir(dp)) != NULL) {
    
    while ((dirp = readdir(dp)) != NULL) {
        
        if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
            continue;
        strcpy(tmp, checkDir);
        strcat(tmp, "/");
        strcat(tmp, dirp->d_name);
        
        strcpy(tmpname, dirp->d_name);

        if (stat(tmp, &statbuf) < 0) {
            fprintf(stderr, "stat error\n");
            return false;
        }

        if((statbuf.st_mode & S_IFMT) == S_IFREG) {
            if(!strcmp(fname, tmpname)) {
                    strcpy(pathname, tmp);
                    return true;
            }
        }
        else if((statbuf.st_mode & S_IFMT) == S_IFDIR) {
            isExist(fname, pathname);   
        } 
    }
    return false;
    
}
int makeInfo(char *fname, char *fnamepath, struct tm dtm) {
    // fpath : 삭제할 파일 경로(이름), tm : 삭제시간(함수호출시간)
    //checkDir에서 해당 파일 이름 찾기
    //fname : 파일 이름만 저장
    
    struct stat statbuf;
    time_t t;
    struct tm mtm; 
    char dtime[BUFLEN];
    char mtime[BUFLEN];
    FILE *fp;

    sprintf(dtime, "D : %d-%02d-%02d %02d:%02d:%02d\n", dtm.tm_year+1900, dtm.tm_mon+1, dtm.tm_mday, dtm.tm_hour, dtm.tm_min, dtm.tm_sec);
    
    if ((stat(fnamepath, &statbuf)) < 0) {
        fprintf(stderr, "stat error\n");
        return false;
    }
    
    t = statbuf.st_mtime;
    mtm = *gmtime(&t);
    sprintf(mtime, "M : %d-%02d-%02d %02d:%02d:%02d\n",mtm.tm_year+1900, mtm.tm_mon+1, mtm.tm_mday, mtm.tm_hour, mtm.tm_min, mtm.tm_sec);

    chdir(infoDir);
    if ((fp = fopen(fname, "a")) == NULL) {
        fprintf(stderr, "fopen error for %s\n", fname);
        return false ;
    }
    fprintf(fp, "%s\n", fnamepath);
    fprintf(fp, "%s\n", dtime);
    fprintf(fp, "%s\n", mtime);
    fclose(fp);

    return true;
}
    
int intoTrash(char *fname, char *pathname) { //rename()으로 경로 바꾸기!
    char newpath[BUFLEN];
    sprintf(newpath, "%s/%s", filesDir, fname);
    
    if (rename(pathname, newpath) < 0) {
        fprintf(stderr, "rename error\n");
        return false;
    }
    return true;

}
    

void doDelete(int argc, char *argv[5]) {
    char saved_path[BUFLEN];
    char oldest[BUFLEN];
    DIR *dp;
    struct dirent *dirp;
    int idx=0, i;
    struct stat statbuf;
    off_t dsize;
    time_t nowt;
    struct tm tm;

    char fname[BUFLEN];
    char deldate[BUFLEN];
    char deltime[BUFLEN];
    char curdate[BUFLEN];
    char curtime[BUFLEN];
    char fullpath[BUFLEN];
    int iOption = false;
    int rOption = false;
    int isexist = false;
    char yesOrno;

    //trash 디렉토리 없는경우 생성
    getcwd(saved_path, BUFLEN);
    strcpy(trashDir, saved_path);
    strcat(trashDir, "/trash");
    if (access(trashDir , F_OK)) {
        mkdir(trashDir, 0753);
    }
    chdir(trashDir);
    
    getcwd(saved_path, BUFLEN);
    strcpy(filesDir, saved_path);
    strcat(filesDir, "/files");
    if (access(filesDir, F_OK)) { //files디렉토리 없는경우 생성
        mkdir(filesDir, 0753);
    }

    strcpy(infoDir, saved_path);
    strcat(infoDir, "/info");
    if (access(infoDir, F_OK)) //info 디렉토리 없는경우 생성
        mkdir(infoDir, 0753);

    chdir(workDir);
    //infoDir 크기확인
    dsize = getDirSize(infoDir);

    nowt = time(NULL);
    tm = *localtime(&nowt);
    if(dsize > 2000) { //2KB넘을 경우 제일 오래된 파일 삭제
        findOldFile(infoDir, oldest, tm); //제일 오래된 파일이름 oldest에 저장됨
        remove(oldest);
    }

    strcpy(fname, argv[1]);
    if (strlen(fname) < 0 || !(isexist = isExist(fname, fullpath))){
        fprintf(stderr, "%s file don't exist!\n", argv[1]);
        return;
    }
    chdir(workDir);
    if (argc == 2) { //ENDTIME 없는경우 즉시삭제
        //argv[1]인 fname 삭제!
        remove(fullpath);
    }
    //여기부터는 삭제시간 있는 경우
    strcpy(deldate, argv[2]);
    strcpy(deltime, argv[3]);

    for(i = 0; i < argc; i++) { //옵션저장
        if(strstr(argv[i], "-r") != NULL) 
            rOption = true;
        if(strstr(argv[i], "-i") != NULL)
            iOption = true;
    }

    //삭제시간 기다리기
    while(1) {
        nowt = time(NULL);
        tm = *localtime(&nowt);
        sprintf(curdate, "%d-%02d-%02d", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday);
        sprintf(curtime, "%d:%d", tm.tm_hour, tm.tm_sec);
        
        if(!strcmp(curdate,deldate) && !strcmp(curtime,deltime)) {
            if(rOption) {
                while(1) {
                    printf("Delete [y/n]?");
                    scanf(" %c", &yesOrno);
                    if(yesOrno == 'y')
                        break;
                    else if(yesOrno == 'n')
                        return; //삭제하지 않기
                }

            }   
            else
                break; 
        }
    }


    //삭제시간 된 경우
    if(iOption){
        remove(fullpath);
    }
    //i옵션 없는경우
    nowt = time(NULL);
    tm = *localtime(&nowt); //삭제시간 저장    
    if (intoTrash(fname, fullpath) < 0) //삭제파일 filesDir로 이동!(rename)
        return;

    if (makeInfo(fname, fullpath, tm) < 0) //infoDir에 삭제한 파일정보 저장
        return;


    return;
}


void doSize() {

    return;
}

void doRecover() {

    return;
}

void doTree() {
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

