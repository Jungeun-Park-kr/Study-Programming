#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
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

	logfp = fopen(logFile, "w");
	fclose(logfp);
	
	strcpy(trashDir, saved_path);
	strcat(trashDir, "/trash");		
    if (access(trashDir, F_OK)) {//trashDir 없으면 생성
	    mkdir(trashDir, 0753);
	}

	strcpy(filesDir, trashDir);
	strcat(filesDir, "/files");
	if (access(filesDir, F_OK)) { //files디렉토리 없는경우 생성
		mkdir(filesDir, 0753);
	}
	strcpy(infoDir, trashDir);
	strcat(infoDir, "/info");
	if (access(infoDir, F_OK)) //info 디렉토리 없는경우 생성
		mkdir(infoDir, 0753);

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

void do_Monitor(char *logFile) {
	int ifd, wd;
	FILE *logfp;
	char buf[EVENTLEN];
	struct stat statbuf;
	time_t intertime;
	struct tm tm;
	int len, i = 0;
	char tmp[BUFLEN];

	if ((logfp = fopen(logFile, "a+")) == NULL) {
		fprintf(stderr, "fopen error\n");
		exit(1);
	}

	ifd = inotify_init();
	if (ifd == -1) {
		fprintf(stderr, "inotify_init error\n");
		return;
	}

	wd = inotify_add_watch(ifd, checkDir, IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM|IN_MOVED_TO);
	if (wd == -1) {
		fprintf(stderr, "inotify_add_watch error\n");
		return;
	}

	while (1) {
		i = 0;
		len = read(ifd, buf, EVENTLEN);
		while (i < len) {
			struct inotify_event *event = (struct inotify_event *)&buf[i];
			if (event->len && (event->name[0] != '.')) {
				if (event->mask & IN_CREATE || event->mask & IN_MOVED_TO) {
					sprintf(tmp, "%s/%s", checkDir, event->name);
					stat(tmp, &statbuf);
					intertime = statbuf.st_atime;
					tm = *gmtime(&intertime);
					logfp = fopen(logFile, "a");
					fprintf(logfp, "[%d-%02d-%02d %02d:%02d:%02d]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
					fprintf(logfp, "[create_%s]\n", event->name);
					fclose(logfp);
				}
				else if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM) {
					sprintf(tmp, "%s/%s", checkDir, event->name);
					stat(tmp, &statbuf);
					intertime = statbuf.st_atime;
					tm = *gmtime(&intertime);
					logfp = fopen(logFile, "a");
					fprintf(logfp, "[%d-%02d-%02d %02d:%02d:%02d]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
					fprintf(logfp, "[delete_%s]\n", event->name);
					fclose(logfp);
				}
				else if (event->mask & IN_MODIFY) {
					sprintf(tmp, "%s/%s", checkDir, event->name);
					stat(tmp, &statbuf);
					intertime = statbuf.st_atime;
					tm = *gmtime(&intertime);
					logfp = fopen(logFile, "a");
					fprintf(logfp, "[%d-%02d-%02d %02d:%02d:%02d]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
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
	int argc, i, cnt=0;
    char c;
    char *ptr;
	while (1) {
        argc = cnt = 0;
        memset(tmp, (char)0, BUFLEN);
        memset(command, (char)0, BUFLEN);
		for (i = 0; i < 7; i++) {
			memset(argv[i], (char)0, BUFLEN);
		}
		printf("20180753>");
        fgets(command, BUFLEN, stdin);

		if (strstr(command, "exit") != NULL) { //exit명령어 사용시 종료
			//kill(deamonpid, SIGKILL);
			break;
		}
		else if (strstr(command, "delete") != NULL) { //detete
			ptr = strtok(command, " ");
			while (ptr != NULL) {
				strcpy(argv[argc++], ptr);
				ptr = strtok(NULL, " ");
			}
			if (argc < 2) {
				fprintf(stderr, "usage : DELETE [FILENAME] [END_TIME] [OPTION]\n");
                continue;
			}
			doDelete(argc, argv);
		}
		else if (strstr(command, "size") != NULL) {
			doSize();
		}
		else if (strstr(command, "recover") != NULL) { //recover
            ptr = strtok(command, " ");
            while(ptr != NULL) {
                strcpy(argv[argc++], ptr);
                ptr = strtok(NULL, " ");
            }
            if (argc < 2) {
                fprintf(stderr, "usage : RECOVER [FILENAME] [OPTION]\n");
                continue;
            }
			doRecover(argc, argv);
		}
		else if (strstr(command, "tree") != NULL) {
			doTree();
		}
		else {
			doHelp();
		}
	}
	printf("모니터링을 종료합니다\n");
    return;
}
char *rtrim(char *str) {
    char tmp[BUFLEN];
    char *end;

    strcpy(tmp, str);
    end = tmp + strlen(tmp) - 1;
    while(end != str && isspace(*end))
        --end;

    *(end+1) = '\0';
    str = tmp;
    return str;
}

char *ltrim(char *str) {
    char *start = str;
    while(*start != '\0' && isspace(*start))
        ++start;
    str = start;
    return str;
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
		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;
		strcpy(tmp, dirName);
		strcat(tmp, "/");
		strcat(tmp, dirp->d_name);

		if (stat(tmp, &statbuf) < 0) {
			fprintf(stderr, "stat error\n");
			return 0;
		}

		if ((statbuf.st_mode & S_IFMT) == S_IFREG) {
			dsize += statbuf.st_size;
		}
		else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
			dsize += getDirSize(tmp);
		}
	}
    closedir(dp);
	return dsize;
}
int compTime(struct tm t1, struct tm t2) { //더 오래된게 리턴
	if (t1.tm_year + 1900 < t2.tm_year + 1900)
		return true;
	else if (t1.tm_year + 1900 > t2.tm_year + 1900)
		return false;

	if (t1.tm_mon + 1 < t2.tm_mon + 1)
		return true;
	else if (t1.tm_mon + 1 > t2.tm_mon + 1)
		return false;

	if (t1.tm_mday < t2.tm_mday)
		return true;
	else if (t1.tm_mday > t2.tm_mday)
		return false;
}


void findOldFile(char *dirName, char *oldest, struct tm oldtm) {
	printf("fildOldFile() 실행\n");
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
		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
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
			if (!compTime(oldtm, newtm)) { //newtm이 더 오래되면 갱신
				strcpy(oldest, fname);
				oldtm = newtm;
			}
		}
		else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
			findOldFile(fname, oldest, oldtm);
		}
	}
    printf("oldest : %s\n", oldest);
	return;
}
int isExist(char *dirName, char *fname, char *pathname) {
	//checkDir내에 fname파일 존재시 true(1)리턴, 없으면 false(0)리턴
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmpname[BUFLEN];
    char nxtdir[BUFLEN];
    int result = false;
	if ((dp = opendir(dirName)) == NULL) {
		fprintf(stderr, "opendir error for %s\n",dirName);
		return false;
	}
	while ((dirp = readdir(dp)) != NULL) {
		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

        sprintf(tmpname, "%s/%s", dirName, dirp->d_name);
        if (stat(tmpname, &statbuf) < 0) {
			fprintf(stderr, "stat error\n");
			return false;
		}

		if ((statbuf.st_mode & S_IFMT) == S_IFREG) {
            if(!strcmp(fname, dirp->d_name)) {
                strcpy(pathname, tmpname);
                return true;            
            }
        }
		else if ((statbuf.st_mode & S_IFMT) == S_IFDIR){
            sprintf(nxtdir, "%s/%s", dirName,dirp->d_name);
			result = isExist(nxtdir, fname, pathname);
        } 
	}
    if(result)
        return true;
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
        char info[BUFLEN];
        char infopath[BUFLEN];
		FILE *fp;
        int fd;

        sprintf(infopath, "%s/%s", infoDir,fname);
        
        fp = fopen(infopath, "w");
        fclose(fp);

        if ((fp = fopen(infopath, "a")) == NULL){
            fprintf(stderr, "fopen error for %s\n", fname);
            //return false;
        }


		sprintf(dtime, "D : %d-%02d-%02d %02d:%02d:%02d\n", dtm.tm_year + 1900, dtm.tm_mon + 1, dtm.tm_mday, dtm.tm_hour, dtm.tm_min, dtm.tm_sec);

		if ((stat(fnamepath, &statbuf)) < 0) {
			fprintf(stderr, "stat error\n");
			return false;
		}
		t = statbuf.st_mtime;
		mtm = *gmtime(&t);
		sprintf(mtime, "M : %d-%02d-%02d %02d:%02d:%02d\n", mtm.tm_year + 1900, mtm.tm_mon + 1, mtm.tm_mday, mtm.tm_hour, mtm.tm_min, mtm.tm_sec);

		fprintf(fp, "%s\n", fnamepath);
		fprintf(fp, "%s", dtime);
		fprintf(fp, "%s", mtime);
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


	void doDelete(int argc, char (*argv)[BUFLEN]) {
		char oldest[BUFLEN];
		int idx = 0, i;
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
		char answer;
        char *ptr;
        int isdel = false;

		//infoDir 크기확인
		dsize = getDirSize(infoDir);
        nowt = time(NULL);
		tm = *localtime(&nowt);

		if (dsize > 2000) { //2KB넘을 경우 제일 오래된 파일 삭제
			//findOldFile(infoDir, oldest, tm); //제일 오래된 파일이름 oldest에 저장됨
			//remove(oldest);
		}
		strcpy(fname, rtrim(argv[1]));

		if (strlen(fname) < 0) {
			fprintf(stderr, "file name error!\n", argv[1]);
			return;
        }
        chdir(workDir);
        if (!isExist(checkDir, fname, fullpath)) {
            fprintf(stderr, "%s file don't exist!\n", fname);
            return;
        }
        chdir(workDir);
		if (argc == 2) { //ENDTIME 없는경우 즉시삭제
		    nowt = time(NULL);
		    tm = *localtime(&nowt); //삭제시간 저장    
		    
            if (makeInfo(fname, fullpath, tm) < 0) //infoDir에 삭제한 파일정보 저장
                return ;
            
            if (intoTrash(fname, fullpath) < 0) //삭제파일 filesDir로 이동!(rename)
			    return ;
            return;
		}
		//여기부터는 삭제시간 있는 경우
		strcpy(deldate, rtrim(argv[2]));
		strcpy(deltime, rtrim(argv[3]));

		for (i = 0; i < argc; i++) { //옵션저장
			if ((ptr = strstr(argv[i], "-r")) != NULL)
				rOption = true;
			if ((ptr=strstr(argv[i], "-i")) != NULL)
				iOption = true;
		}
        
		//삭제시간 기다리기
		while (1) {
			nowt = time(NULL);
			tm = *localtime(&nowt);
			sprintf(curdate, "%d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
			sprintf(curtime, "%02d:%02d", tm.tm_hour, tm.tm_min);
            //printf("curtime : %s\n",curtime);
            //printf("deltime : %s\n",deltime);
			if (!strcmp(curdate, deldate) && !strcmp(curtime, deltime)) {
				if (rOption) {
					printf("Delete [y/n]? ");
				    scanf(" %c", &answer);
					getchar();
                    if (answer == 'y') {
						    isdel = true;
                            break;
                    }
					else if (answer ==  'n') 
                        return ; //삭제하지 않고 종료 
				}
                isdel = true;
                break;
			}
            if(isdel)
                break;
		}


		//삭제시간 된 경우
		if (iOption) {
			remove(fullpath);
            return;
		}
		//i옵션 없는경우
		nowt = time(NULL);
		tm = *localtime(&nowt); //삭제시간 저장    
		if (makeInfo(fname, fullpath, tm) < 0) //infoDir에 삭제할 파일정보 저장
            return;
        
        if (intoTrash(fname, fullpath) < 0) //삭제파일 filesDir로 이동!(rename)
			return;

		return;
	}


	void doSize() {

		return;
	}

int intoCheck(char *fname, char *pathname) {
    //pathname : 새이름, fname : 파일 이름만
    char buf[BUFLEN];
    sprintf(buf, "%s/%s", filesDir, fname); //원래 있던 파일
    if (rename(buf, pathname) <0) {
        fprintf(stderr, "remove error\n");
        return false;
    }
    return true;
}

void doRecover(int argc, char (*argv)[BUFLEN]) {
    char tmp[BUFLEN];
    char fname[BUFLEN];
    char pathname[BUFLEN];
    char buf[BUFLEN];
    int i, lOption = false;    
    FILE *infofp, *fillefp;

   
    sprintf(fname, rtrim(argv[1]));
    if(argc == 3)
        if(strstr(argv[2], "-l"))
            lOption = true;
    
    if(isExist(infoDir, fname, pathname) < 0) {
        fprintf(stderr, "There is no '%s' in the 'trash' directory\n", fname);
        return ;
    }

    //같은 파일이 여러개인 경우 : 중복파일 정보 출력 후 선택한 파일을 복구

    
    //같은 파일 없는경우 => OK
    sprintf(pathname, "%s/%s", infoDir, fname);
    if ((infofp = fopen(pathname, "r")) == NULL) {
        fprintf(stderr, "fopen error\n");
        return ;
    }
    
    fscanf(infofp, "%s" , buf);
    strcpy(pathname, buf); //파일의 원래 있던 경로로 옮기기
    if (intoCheck(fname, pathname) < 0) {
        return;
    }
    //info파일 삭제하기
    sprintf(tmp, "%s/%s", infoDir, fname);
    if (remove(tmp) < 0 )
        return ;

    return;
}

void doTree() {
    return ;
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


