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
#define COMMAND_ARGC 7
#define MAXFILE 100
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


    /*
	ifd = inotify_init();
	if (ifd == -1) {
		fprintf(stderr, "inotify_init error\n");
		return;
	}

	wd = inotify_add_watch(ifd, checkDir, IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO);
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
                    
                    intertime = time(NULL);
                    tm = *localtime(&intertime);    
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
					
                    intertime = time(NULL);
                    tm = *localtime(&intertime);
                    
                    logfp = fopen(logFile, "a");
					fprintf(logfp, "[%d-%02d-%02d %02d:%02d:%02d]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
					fprintf(logfp, "[delete_%s]\n", event->name);
					fclose(logfp);
				}
				else if (event->mask & IN_MODIFY | IN_CREATE) {
					sprintf(tmp, "%s/%s", checkDir, event->name);
					stat(tmp, &statbuf);
					intertime = statbuf.st_atime;
					tm = *gmtime(&intertime);

                    intertime = time(NULL);
                    tm = *localtime(&intertime);

					logfp = fopen(logFile, "a");
					fprintf(logfp, "[%d-%02d-%02d %02d:%02d:%02d]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
					fprintf(logfp, "[modify_%s]\n", event->name);
					fclose(logfp);
				}
			}
			i += EVENT_SIZE + event->len;
			//sleep(1);
		}
	}
	inotify_rm_watch(ifd, wd);
	close(ifd);
	fclose(logfp);

    */
	return;
}

void do_Prompt() {
	char command[BUFLEN];
	char tmp[BUFLEN];
	char argv[COMMAND_ARGC][BUFLEN];
	int argc, i, cnt = 0;
	char c;
	char *ptr;
	while (1) {
		argc = cnt = 0;
		memset(tmp, (char)0, BUFLEN);
		memset(command, (char)0, BUFLEN);
		for (i = 0; i < COMMAND_ARGC; i++) {
			memset(argv[i], (char)0, BUFLEN);
		}
		printf("20180753>");
		fgets(command, BUFLEN, stdin);

		if (strstr(command, "exit") != NULL) { //exit명령어 사용시 종료
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
			while (ptr != NULL) {
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
	while (end != str && isspace(*end))
		--end;

	*(end + 1) = '\0';
	str = tmp;
	return str;
}

char *ltrim(char *str) {
	char *start = str;
	while (*start != '\0' && isspace(*start))
		++start;
	str = start;
	return str;
}

char *rmvpath(char *str) { //앞의 경로는 제외하고 이름만 뽑아내는 함수
    char *start;
    
    start = str + strlen(str) - 1;
    while (*start != '/')
        --start;
    str = start + 1;
    
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

int compTime(struct tm t1, struct tm t2) { //t1이 오래되면 1, t2가 오래되면 0 리턴
	printf("시간비교 실행\n");
  
    if (t1.tm_year < t2.tm_year)
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

    if (t1.tm_hour < t2.tm_hour)
        return true;
    else if (t1.tm_hour > t2.tm_hour)
        return false;

    if (t1.tm_min < t2.tm_min)
        return true;
    else if (t1.tm_min > t2.tm_min)
        return false;

    if (t1.tm_sec < t2.tm_min)
        return true;
    else if (t1.tm_sec > t2.tm_sec)
        return false;
}

void findOldFile(char *dirName, char *oldest, struct tm oldtm) {
//void findOldFile(char *dirName, char *oldest, char *oldtime) {
    printf("fildOldFile() 실행\n");
	struct dirent *dirp;
	DIR *dp;
	struct stat statbuf;
	char fname[BUFLEN];
	time_t newtime;
	struct tm newtm;

    //FILE *fp;

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
			printf("검색할 파일 : %s\n",fname);
            /*if ((fp = fopen(fname, "r")) == NULL) {
                fprintf(stderr, "fopen error for %s\n", fname);
                return;
            }*/
            
            newtime = statbuf.st_mtime;
			newtm = *gmtime(&newtime);
			if (!compTime(oldtm, newtm)) { //newtm이 더 오래되면 0리턴
				//printf("갱신됨 : %s\n", fname);
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

char *rmvdelimeter(char *str) {
    char tmp[BUFLEN];
    char *start, *end;

    strcpy(tmp, str);
    start = tmp;
    while (*start != '\0' && *start != '*')
        ++start;
    end = start;
    start = tmp;
    *(end) = '\0';
    //printf("start : %s\n", start);
    
    return start;
}

int isExist(char *dirName, char *fname, char *pathname) {
	//dirName 디렉토리에 fname파일 존재시 true(1)리턴, 없으면 false(0)리턴, 파일경로 pathname에 저장
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmpname[BUFLEN], dname[BUFLEN];
	char nxtdir[BUFLEN];
	int result = false;
	if ((dp = opendir(dirName)) == NULL) {
		fprintf(stderr, "opendir error for %s\n", dirName);
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
            strcpy(dname, rmvdelimeter(dirp->d_name));
			if (!strcmp(fname, dname)) {  //delimeter 제거하고 이름 비교하기
				strcpy(pathname, tmpname); //경로까지 합친 이름 저장
				return true;
			}
		}
		else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
			sprintf(nxtdir, "%s/%s", dirName, dirp->d_name);
			result = isExist(nxtdir, fname, pathname);
		}
	}
	if (result)
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
	char dtime[BUFLEN], mtime[BUFLEN];
	char info[BUFLEN], infopath[BUFLEN];
    char trashinfo[BUFLEN];
	FILE *fp;
	int fd;
    
	sprintf(infopath, "%s/%s", infoDir, fname);
    strcpy(trashinfo, "[Trash info]");
	fp = fopen(infopath, "w");
	fclose(fp);

	if ((fp = fopen(infopath, "a")) == NULL) {
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

	fprintf(fp, "%s\n", trashinfo);
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


void doDelete(int argc, char(*argv)[BUFLEN]) {
	char oldest[BUFLEN], oldestfname[BUFLEN];
	int idx = 0, i;
	struct stat statbuf;
	off_t dsize;
	time_t nowt;
	struct tm tm;

	char fname[BUFLEN], tmpbuf[BUFLEN];
	char deldate[BUFLEN], deltime[BUFLEN], curdate[BUFLEN], curtime[BUFLEN];
	char fullpath[BUFLEN], filespath[BUFLEN];
    char newname[BUFLEN];
	int iOption = false, rOption = false;
	char answer;
	char *ptr;
	int isdel = false;
    int nums = 0, res, flen;
    char dupfiles[MAXFILE][BUFLEN];
    char delimeter[BUFLEN];


    while(1) { //infoDir크기 확인
        dsize = getDirSize(infoDir);
        printf("디렉토리의 크기 : %ld\n", dsize);
	    if (dsize > 2000) { //2KB넘을 경우 제일 오래된 파일 삭제
		    nowt = time(NULL);
            tm = *localtime(&nowt);
            findOldFile(infoDir, oldest, tm); //제일 오래된 파일이름 oldest에 저장됨
            printf("제일 오래된 파일 : %s\n", oldest);
            strcpy(oldestfname, rmvpath(oldest)); //경로 제외한 이름만복사
		    remove(oldest); //info 디렉토리 내의 파일 삭제
            sprintf(oldest, "%s/%s", filesDir,oldestfname);
            remove(oldest); //files 디렉토리 내의 파일 삭제
	    }
        else
            break;
    }

    strcpy(fname, rtrim(argv[1]));
    if (strlen(fname) < 0) {
		fprintf(stderr, "file name error!\n", argv[1]);
		return;
	}
	
    if (!isExist(checkDir, fname, fullpath)) { //상대경로(파일이름)로 파일 존재 확인
        strcpy(tmpbuf, rmvpath(fname));
        if (!isExist(checkDir, tmpbuf, fullpath)) { //절대경로로 파일 존재 확인
		    fprintf(stderr, "%s file don't exist!\n", fname); //그래도 없으면 메시지 출력 후 종료
		    return;
        }
        else { //절대경로이면 파일 이름만 fname에 저장
            strcpy(fname, tmpbuf);
        }
	}
    

    printf("first fname(무조건 이름만 나와야됨) : %s\n", fname);
    

    //중복파일있는지 확인
    res = isExist(filesDir, fname, filespath); //하나라도 있으면 true(1) 리턴
    if (res) {
        res = isDup(filesDir, fname, &nums, dupfiles); 
        //모든 파일 저장해서 개수 nums, 파일이름 dupfiles에 저장
        if (res == 0) {
            fprintf(stderr, "isDup Error\n");
            return;
        }
        if (nums > 0) { //중복된 파일이 있는경우
            //파일 이름 뒤에 delimeter숫자 추가해 이름변경 (a.txt*2, a.txt*3, ...)
            strcpy(newname, fname);
            sprintf(delimeter, "%c%d", '*', nums+1);
            strcat(newname, delimeter);
            strcpy(fname, newname);
        }
    }

	if (argc == 2) { //ENDTIME 없는경우 즉시삭제
		nowt = time(NULL);
		tm = *localtime(&nowt); //삭제시간 저장    

		if (makeInfo(fname, fullpath, tm) < 0) //infoDir에 삭제한 파일정보 저장
			return;

		if (intoTrash(fname, fullpath) < 0) //삭제파일 filesDir로 이동!(rename)
			return;
		return;
	}
	//여기부터는 삭제시간 있는 경우
	strcpy(deldate, rtrim(argv[2]));
	strcpy(deltime, rtrim(argv[3]));

    //입력받은 삭제시간이 현재보다 이전인 경우 -> 에러처리
    nowt = time(NULL);
    tm = *localtime(&nowt);
	sprintf(curdate, "%d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	sprintf(curtime, "%02d:%02d", tm.tm_hour, tm.tm_min);
    if ((strcmp(curdate, deldate)==1||strcmp(curtime, deltime)==1)) { 
        fprintf(stderr, "delete time not be past!\n");
        return;
    }
    

	for (i = 0; i < argc; i++) { //옵션저장
		if ((ptr = strstr(argv[i], "-r")) != NULL)
			rOption = true;
		if ((ptr = strstr(argv[i], "-i")) != NULL)
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
				else if (answer == 'n')
					return; //삭제하지 않고 종료 
			}
			isdel = true;
			break;
		}
		if (isdel)
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

int isDup(char *dirName, char *fname, int *num, char(*dupfiles)[BUFLEN]) {
    char tmp[BUFLEN];
	char fpath[BUFLEN];
	char nxtDir[BUFLEN];
    char delname[BUFLEN];
	DIR *dirp;
	struct dirent *dp;
	struct stat statbuf;
	int i;
   
   printf("들어옴 "); 
	//sprintf(fpath, "%s/%s", dirName, fname);
	if ((dirp = opendir(dirName)) == NULL) {
		fprintf(stderr, "opendir error for %s\n", dirName);
		return false;
	}

	while ((dp = readdir(dirp)) != NULL) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;
		strcpy(tmp, dirName);
		strcat(tmp, "/");
		strcat(tmp, dp->d_name);

		if (stat(tmp, &statbuf) < 0) {
			fprintf(stderr, "stat error\n");
			return false;
		}
		if ((statbuf.st_mode & S_IFMT) == S_IFREG) {
            strcpy(delname, rmvdelimeter(dp->d_name));
			if (!strcmp(fname, delname)) { //구분자(*) 제거후 이름 비교
                strcpy(dupfiles[(*num)++], dp->d_name); //이름 복사해 저장
			}
		}
		else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
			sprintf(nxtDir, "%s/%s", dirName, dp->d_name);
			isDup(nxtDir, fname, num, dupfiles);
		}
	}
    printf("나감\n");
	return true;
}

int printDup(int num, char *fname, char(*dupFiles)[BUFLEN]) {
	FILE *fp;
	int i, choice = 0;
    char orgfname[BUFLEN];
	char path[BUFLEN], buf[BUFLEN], buf1[BUFLEN], buf2[BUFLEN];
    char tmpbuf[BUFLEN];

	//파일 풀네임 만들기(경로합쳐서) -> 오픈해서 D, M 정보 출력하기

	for (i = 0; i < num; i++) {
		sprintf(path, "%s/%s", infoDir, dupFiles[i]);
        strcpy(tmpbuf, path);

		if ((fp = fopen(path, "r")) == NULL) {
			fprintf(stderr, "fopen error for %s\n", dupFiles[i]);
			return;
		}

		/////경로 제외하고 읽기
		fgets(buf, BUFLEN, fp); //첫번째 줄 skip (Trash Info)
		fgets(buf, BUFLEN, fp);
        fgets(buf, BUFLEN, fp);
        strcpy(buf1, rtrim(buf)); // 삭제시간
		fgets(buf, BUFLEN, fp);
        strcpy(buf2, rtrim(buf)); //최종접근시간

		printf("%d. %s  %s %s\n", i+1, fname, buf1, buf2); 
		fclose(fp);
	}
	printf("Choose : ");
	scanf("%d", &choice);
    getchar();

	if (choice < 1)
		return false;

	return choice;
}

int intoCheck(char *fname, char *pathname) {
    //pathname : 저장될경로, fname : 기존이름(경로 포함됨)
	char dupfiles[MAXFILE][BUFLEN], tmpbuf[BUFLEN];
	char curname[BUFLEN], curpath[BUFLEN], newname[BUFLEN], newpath[BUFLEN];
    int isdup = false, dupnum = 0;

    printf("파라미터 fname :%s\n",fname);
    strcpy(pathname, rtrim(pathname));
    strcpy(curname, rmvpath(fname)); //경로 제외해서 이름만
    sprintf(curpath, "%s/%s", filesDir, curname); //이동시킬 파일 경로

    strcpy(newname, rmvdelimeter(curname)); //저장될 이름 저장
    isdup = isExist(checkDir, newname, tmpbuf); //딜리미터 제거한 이름(원래이름)이 중복인지 확인
    if (isdup) { //중복 파일 존재시 숫자_추가 후 rename()
        printf("checkDir에 중복 파일 존재!\n");
       
        printf("newname:%s\n",newname); 
        isDup(checkDir, newname, &dupnum, dupfiles); //check파일에 존재하는 파일의 개수 가져오기
        printf("dupnum:%d\n",dupnum);
        sprintf(newpath, "%s/_%d%s", checkDir, dupnum+1, newname);
        
        printf("현재이름 :%s변경이름 :%s\n", curpath, newpath);
        if (rename(curpath, newpath) < 0) {
            fprintf(stderr, "rename error\n");
            return false;
        }
        return true;
    }

    //rename()으로 원래의 경로로 파일 이동(복구)
    if (rename(curpath, pathname) < 0) { //buf에서 pathname으로 변경
		fprintf(stderr, "rename error\n");
		return false;
	}
	return true;
}

void relocateDup(char *dirName, char *fname) {
    DIR *dp;
    struct dirent *dirp;
    char tmp[BUFLEN], path[BUFLEN], newpath[BUFLEN];
    int idx = 1;

    if ((dp = opendir(dirName)) == NULL) {
        fprintf(stderr, "opendir error for %s\n", dirName);
        return;
    }

   while ((dirp = readdir(dp)) != NULL) {
       if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
           continue;
        
       if (!strcmp(fname, dirp->d_name)) {
           printf("파일발견!\n");
           sprintf(tmp, "%s/%s", dirName, dirp->d_name);

           strcpy(path, tmp);
           sprintf(newpath, "%s/%s*%d", dirName, dirp->d_name, idx++);

           if (rename(path, newpath) < 0) {
               fprintf(stderr, "rename error\n");
               return ;
           }
       }
   }

}

void doRecover(int argc, char(*argv)[BUFLEN]) {
	char tmp[BUFLEN];
	char fname[BUFLEN];
	char pathname[BUFLEN];
	char buf[BUFLEN];
	int i, lOption = false, choice, res;
	FILE *infofp, *fillefp;
	int dupn = 0;
	char dupfiles[MAXFILE][BUFLEN];
    char mvfname[BUFLEN], orgfname[BUFLEN];

	sprintf(fname, rtrim(argv[1]));
	if (argc == 3)
		if (strstr(argv[2], "-l"))
			lOption = true;

	if (isExist(infoDir, fname, pathname) < 0) {
		fprintf(stderr, "There is no '%s' in the 'trash' directory\n", fname);
		return;
	}

	if (isDup(filesDir, fname, &dupn, dupfiles) < 0) {
		fprintf(stderr, "duplication confirm error \n");
		return;
	}
	if (dupn > 1) { //파일 여러개인 경우 : 중복파일 정보 출력 후 선택된 파일을 복구
        choice = printDup(dupn, fname, dupfiles);
		if (choice == false) {
			fprintf(stderr, "printDup error\n");
			return;
		}
        sprintf(orgfname, "%s/%s", infoDir, dupfiles[choice-1]);
		//선택한 파일의 delimeter 지운 파일 이름으로 recover 해야함
        if ((infofp = fopen(orgfname, "r")) == NULL) {
            fprintf(stderr, "fopen error for %s\n", dupfiles[choice-1]);
            return ;
        }
        //sprintf(mvfname, "%s/%s", filesDir, dupfiles[choice-1]);
        //printf("mvfname, : %s\n", mvfname);
        fgets(buf, BUFLEN, infofp); //Trash Info skip
        fgets(buf, BUFLEN, infofp); //경로저장
        strcpy(pathname, rtrim(buf));
        if ((res= intoCheck(dupfiles[choice-1], pathname)) < 0) {
            return;
        }
        else if (res) {
            sprintf(tmp, "%s/%s", infoDir, rtrim(dupfiles[choice-1]));
            if (remove(tmp) < 0)
                return;
        }
        
        //recover 한 후에, 다른 파일 이름 delimeter 다시 정리
        //relocateDup(infoDir, fname); //같은 이름 가진 파일들 정렬
	    //relocateDup(filesDir, fname);
    }
	else {
		//같은 파일 없는경우 => OK
        //같은 파일 없어도 복수파일로 인해 딜리미터가 붙은 이름인경우
		if ((infofp = fopen(pathname, "r")) == NULL) {
			fprintf(stderr, "fopen error\n");
			return;
		}
        fgets(buf, BUFLEN, infofp); //첫번째 줄은 필요 x
		fgets(buf, BUFLEN, infofp); //기존의 파일 경로 저장
		strcpy(pathname, buf); //파일의 원래 있던 경로로 옮기기
        isExist(filesDir, fname, buf);
        if (strcmp(fname, buf))
            strcpy(fname, buf);
        if ((res = intoCheck(fname, pathname)) < 0) { //fname을 pathname으로 이
			return;
		}
        else if(res) { //원래 파일위치로 이동한 후에는 info 파일도 삭제가능
		    //info파일 삭제하기 
            isExist(infoDir, fname, buf);
            if (strcmp(fname, buf))
                strcpy(fname, buf);       
		    sprintf(tmp, "%s/%s", infoDir, fname);
		    if (remove(tmp) < 0)
                return;
        }
    }
	return;
}

int except_tmp_file(const struct dirent *info) {
}



int makeTree(int depth, char *dname, char (*fname)[BUFLEN], char *ftype, int *fdep, int idx) 
{ //배열 만드는 함수
    int fcnt, allcnt;
    int i, j, fidx = 0;
    struct dirent **filelist;
    char fpath[BUFLEN], tmp[BUFLEN];
    char newdname[BUFLEN];
    struct stat statbuf;
    int size = 0, dsize=0;

    if ((fcnt = scandir(dname, &filelist, NULL, alphasort)) == -1) {
        fprintf(stderr, "scandir error for checkDir\n");
        exit(1);
    }
    size = fcnt;
    //printf("경로:");
    system("pwd");
    printf("파일 개수 %d\n",fcnt);
    for(fidx = 0, i = idx; fidx < fcnt; fidx++, i++) {
        chdir(dname);
        //printf("%s\n", filelist[i]->d_name);
        strcpy(tmp, filelist[fidx]->d_name);
        printf("현재 depth : %d\n", depth);
        if (depth == 1)
            printf("파일 : %s\n", tmp);
        if (!strcmp(tmp, ".")) {
            strcpy(fname[i], tmp);
            if(depth == 0) {
                ftype[i] = 'c'; //check 자기자신
                fdep[i] = depth;
                depth++;
            }
            else {
                
                ftype[i] = 'r'; //무시
                fdep[i] = depth;
            }
            continue;
        }
        
        if (!strcmp(tmp, "..")) {
            strcpy(fname[i], tmp);
            fdep[i] = depth;
            ftype[i] = 'p'; //무시
            continue;
        }
        if (stat(tmp, &statbuf) < 0 ) {
            fprintf(stderr, "stat error for %s\n", tmp);
            return;
        }
        
        if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
            //printTree(depth, dname, fname, ftype);
            strcpy(fname[i], tmp);
            ftype[i] = 'd';
            fdep[i] = depth;
            sprintf(newdname, "%s/%s", dname, tmp);
            dsize = makeTree(depth+1, newdname, fname, ftype, fdep, i+1);
            size += dsize;
            i += dsize;
        }
        else if ((statbuf.st_mode & S_IFMT) == S_IFREG) {
            strcpy(fname[i], tmp);
            ftype[i] = 'f'; 
            fdep[i] = depth;
        }
        idx = i;
    }
    return size;
    
}


void printTree(int fsize, char (*fname)[BUFLEN], char *ftype, int *fdep, int idx) {
    int i, j, k, l;
    int fcnt = 0;
    char tab[10] = "\t\t";
    char line[13] = "------------";
    int depth;

    for (i = idx; i < fsize; i++) {
        depth = fdep[i];
        if (ftype[i] == 'c') {
            printf("check%s", line);
            continue;
        }
        
        else if(ftype[i] == 'f') {
            if (ftype[i-1] == 'p') { //첫 번째 출력되는 일반 파일
                printf("%s", fname[i]);
                continue;
            }
            if (depth < 5) {
                if (ftype[i-1] == 'f' && ftype[i-2] == 'p') //두 번째 출력되는 일반파일에 공백추가
                    printf("\n");
                    
                //세 번째 이후 출력되는 파일
                for (k = 0; k < 4; k++) {
                    for (j = 0; j < depth; j++) { 
                        if (j == 0)
                            printf("%s|", tab);
                        else {
                            printf("%s", tab);
                            for(l = 0; l < j; l++)
                                printf(" ");
                            printf("|");
                        }
                    }
                    if (k == 3)
                        break;
                    printf("\n");
                }
            }
            else { //depth가 깊어져서 길이 부족해질 때
                    for (j = 0; j < depth; j++) { 
                        if (j == 0)
                            printf("%s|", tab);
                        else
                            printf("%s |", tab);
                    }
                    printf("\n");
                    for (j = 0; j < depth; j++) { 
                        if(j == 0)
                            printf("%s|", tab);
                        else {
                            printf("%s", tab);
                            for(l = 0; l < j; l++)
                                printf(" ");
                            printf("|");
                        }
                    }
            }
            
            printf("-%s\n", fname[i]); 
        }
        else if (ftype[i] == 'd') {
                
            if (ftype[i] == 'c') {
                printf("check%s", line);
                continue;
            }
            if (ftype[i-1] == 'p') {
                printf("+%s%s", fname[i],line);
                continue;
            }
        
            if (depth < 5) {

                if (ftype[i-1] == 'f' && ftype[i-2] =='p') 
                    printf("\n");

                for (k = 0; k < 4; k++) {
                    for (j = 0; j < depth; j++) { 
                        if (j==0)
                            printf("%s|", tab);
                        else {
                            printf("%s", tab);
                            for(l =0; l < j; l++) {
                                printf(" ");
                            }
                            printf("|");
                        }
                    }
                    if (k == 3)
                        break;
                    printf("\n");
                }
            }
            else { //depth가 깊어져서 길이 부족해질 때
                    for (j = 0; j < depth; j++) { 
                        if (j == 0)
                            printf("%s|", tab);
                        else
                            printf("%s |", tab);
                    }
                    printf("\n");
                    for (j = 0; j < depth; j++) { 
                        if(j==0)
                            printf("%s|", tab);
                        else {
                            printf("%s", tab);
                            for (l = 0; l < j; l++) 
                                printf(" ");
                            printf("|");
                        }
                    } 
            }
            printf("+%s%s", fname[i], line);
        }
    }
}

void doTree() {

    char fname[BUFLEN][BUFLEN];
    char ftype[BUFLEN];
    int fdep[BUFLEN];
    int fsize = 0;
    int depth = 0, d = 0;

    int i, fcnt = 0;
    for (i = 0; i < BUFLEN; i++) {
        memset(fname[i], (char)0, BUFLEN);
    }
    memset(fname, (char)0, BUFLEN);
    memset(ftype, (char)0, BUFLEN);
    memset(fdep, 0, BUFLEN);

    fsize = makeTree(d, checkDir, fname, ftype, fdep, 0);
    printTree(fsize, fname, ftype, fdep, 0);
    
    

    
    for(i=0; i< fsize; i++) {
        //printf("fdep[%d]:%d\n", i, fdep[i]);
        if (fdep[i] > depth)
            depth = fdep[i];
    }

   /* 
    printf("\nfsize : %d, depth : %d\n", fsize, depth);
    for (i=0; i<fsize; i++) {
        printf("[%d][%c]fname = %s\n", fdep[i],ftype[i], fname[i]);
    }*/
   


    //fsize = fsize - (depth*2); //실제 총 파일 개수
    /*
    for(i = 0; i < fsize; i++) {
        if(ftype[i] == 'c')
            continue;
        else if(ftype[i] == 'p')
            continue;
        else if(ftype[i] == 'r')
            continue;
        else
            printf("%s\n", fname[i]); 
    }
    */
    //printTree(depth, checkDir, fname, ftype);
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
}
