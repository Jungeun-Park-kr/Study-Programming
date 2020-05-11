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
#include <sys/stat.h>
#include <sys/types.h>
#include "ssu_mntr.h"

#define true 1
#define false 0
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
	int pid;

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
		do_Monitor(logFile); //데몬프로세스 모니터링 시작, logFile에 기록
	}

	pid = getpid();
	do_Prompt(pid); //명령어를 실행하는 do_Prompt실행

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

int except_tmp_file(const struct dirent *info) {
	char *name;

	strcpy(name, info->d_name);

	if (info->d_name[0] != '.') //'.'으로 시작하는 swp파일 등은 포함시키지 않음
		return 1;
	else
		return 0;
}

int save_dir_info(char *path, char(*flist)[BUFLEN], struct tm *tm, int idx) {
	//path(디렉토리)에 있는 파일 이름 flist에 저장, st_mtime을 tm에 저장 총 파일 개수 리턴
	int cnt = 0, allcnt = 0;
	int i, j;
	struct stat statbuf;
	char nxtpath[BUFLEN];
	struct dirent **filelist;

	chdir(path);
	if ((cnt = scandir(".", &filelist, except_tmp_file, alphasort)) == -1) {
		fprintf(stderr, "scandir error for %s\n", path);
		return false;
	}
	allcnt = cnt;

	for (i = idx, j = 0; j < cnt; j++, i++) {

		strcpy(flist[i], filelist[j]->d_name);

		if (stat(flist[i], &statbuf) < 0) {
			fprintf(stderr, "stat error\n");
			return false;
		}
		if ((statbuf.st_mode & S_IFMT) == S_IFREG) {
			tm[i] = *localtime(&statbuf.st_mtime);
		}

		else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
			tm[i] = *localtime(&statbuf.st_mtime);
			sprintf(nxtpath, "%s/%s", path, flist[i]);
			allcnt += save_dir_info(nxtpath, flist, tm, (idx + allcnt));
			chdir(path);
		}

	}

	//메모리 해제 후 파일개수 리턴
	for (i = 0; i < cnt; i++)
		free(filelist[i]);
	free(filelist);

	return allcnt;
}

void update_dir_info(char(*flist1)[BUFLEN], char(*flist2)[BUFLEN], struct tm *tm1, struct tm *tm2, int *cnt1, int *cnt2) { //파일 정보를 변경된 것으로 업데이트
	int i;

	if (*cnt1 < *cnt2) {
		for (i = 0; i < *cnt2; i++) {
			strcpy(flist1[i], flist2[i]);
			tm1[i] = tm2[i];
		}
		*cnt1 = *cnt2;
	}
	else {
		for (i = 0; i < *cnt1; i++) {
			strcpy(flist1[i], flist2[i]);
			tm1[i] = tm2[i];
		}
		*cnt1 = *cnt2;
	}
}

void do_Monitor(char *logFile) {
	FILE *logfp;
	char flist1[BUFLEN][BUFLEN], flist2[BUFLEN][BUFLEN];
	int cnt1 = 0, cnt2 = 0, i, j, fixed = false;
	time_t nowt;
	struct tm tm, tm1[BUFLEN], tm2[BUFLEN];
	char *path = ".";

	chdir(checkDir);

	if ((cnt1 = (save_dir_info(checkDir, flist1, tm1, 0))) == 0) {
		fprintf(stderr, "saved_dir_info error\n");
		return;
	}
	while (1) {
		sleep(1);
		fixed = false;
		if ((cnt2 = (save_dir_info(checkDir, flist2, tm2, 0))) == 0) {
			fprintf(stderr, "saved_dir_info error\n");
			return;
		}
		nowt = time(NULL);
		tm = *localtime(&nowt);

		if (cnt1 < cnt2) { //파일의 생성이 일어난 경우
			if ((logfp = fopen(logFile, "a")) == NULL) {
				fprintf(stderr, "fopen error for %s\n", logFile);
				return;
			}
			for (i = 0; i < cnt1; i++) {
				if (!strcmp(flist1[i], flist2[i]))
					continue;
				else {
					fprintf(logfp, "[%d-%02d-%02d %02d:%02d:%02d]", tm2[i].tm_year + 1900, tm2[i].tm_mon + 1, tm2[i].tm_mday, tm2[i].tm_hour, tm2[i].tm_min, tm2[i].tm_sec);
					fprintf(logfp, "[create_%s]\n", flist2[i]);
					fclose(logfp);
					update_dir_info(flist1, flist2, tm1, tm2, &cnt1, &cnt2);
					fixed = true;
					break;
				}
			}
			if (fixed == false) {
				//끝에 파일이 생성된 경우
				fprintf(logfp, "[%d-%02d-%02d %02d:%02d:%02d]", tm2[i].tm_year + 1900, tm2[i].tm_mon + 1, tm2[i].tm_mday, tm2[i].tm_hour, tm2[i].tm_min, tm2[i].tm_sec);
				fprintf(logfp, "[create_%s]\n", flist2[i]);
				fclose(logfp);
				//리스트1 = 리스트2 로 업데이트
				update_dir_info(flist1, flist2, tm1, tm2, &cnt1, &cnt2);
			}
			else
				continue;
		}

		else if (cnt1 > cnt2) { //파일의 삭제가 일어난 경우
			if ((logfp = fopen(logFile, "a")) == NULL) {
				printf("fopen error for %s\n", logFile);
				return;
			}
			fprintf(logfp, "[%d-%02d-%02d %02d:%02d:%02d]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
			for (i = 0; i < cnt2; i++) {
				if (!strcmp(flist1[i], flist2[i]))
					continue;
				else {
					fprintf(logfp, "[delete_%s]\n", flist1[i]);
					fclose(logfp);
					update_dir_info(flist1, flist2, tm1, tm2, &cnt1, &cnt2);
					fixed = true;
				}
			}
			if (fixed == false) { //끝 파일이 변경됨
				fprintf(logfp, "[delete_%s]\n", flist2[i]);
				fclose(logfp);
				//리스트1 = 리스트2로 업데이트
				update_dir_info(flist1, flist2, tm1, tm2, &cnt1, &cnt2);
			}
			else
				continue;
		}
		else { //파일의 수정이 일어났거나 변화 없는 경우
			for (i = 0; i < cnt1; i++) {
				if (memcmp(&tm1[i], &tm2[i], sizeof(struct tm)) != 0) { //수정(변화)경우
					if ((logfp = fopen(logFile, "a")) == NULL) {
						fprintf(stderr, "fopen error for %s\n", logFile);
						return;
					}
					fprintf(logfp, "[%d-%02d-%02d %02d:%02d:%02d]", tm2[i].tm_year + 1900, tm2[i].tm_mon + 1, tm2[i].tm_mday, tm2[i].tm_hour, tm2[i].tm_min, tm2[i].tm_sec);
					fprintf(logfp, "[modify_%s]\n", flist2[i]);
					fclose(logfp);
					if (fixed == false) { //파일 변경때문에 한번 온 경우 (디렉토리일수도 있으므로 반복 마저함)
						fixed = true;
						//sleep(1);
						continue;
					}
					else { //디렉토리 수정이후 파일 수정 감지한 경우 출력 후 업데이트
						//리스트 1 = 리스트2로 업데이트
						update_dir_info(flist1, flist2, tm1, tm2, &cnt1, &cnt2);
						break;
					}
				}
			}
			if (fixed == true)  //디렉토리가 아닌 파일 한 번만 변경 감지된 경우 업데이트 후, 다음 검사 재개
				update_dir_info(flist1, flist2, tm1, tm2, &cnt1, &cnt2);
		}

	}
}


void do_Prompt(int pid) {
	char command[BUFLEN];
	char tmp[BUFLEN];
	char argv[COMMAND_ARGC][BUFLEN];
	int argc, i, cnt = 0;
	char c;
	char *ptr;
	int curpid, tmpid;
	while (1) {

        tmpid = getpid();
		argc = cnt = 0;
		memset(tmp, (char)0, BUFLEN);
		memset(command, (char)0, BUFLEN);
		for (i = 0; i < COMMAND_ARGC; i++) {
			memset(argv[i], (char)0, BUFLEN);
		}
		printf("20180753>");
		fgets(command, BUFLEN, stdin);
		if (!strcmp(command, "\n"))
			continue;
		if (strstr(command, "exit") != NULL) { //exit명령어 사용시 종료
			if (curpid == 0 && pid != tmpid) { //아직 파일 삭제 대기중일 때 exit명령어 사용되는 경우
                kill(pid, SIGKILL); //삭제 대기중인 부모 프로세스를 죽임 
				break;
			}
			else
				break;
		}
		else if (strstr(command, "delete") != NULL) { //detete
			ptr = strtok(command, " ");
			while (ptr != NULL) {
				strcpy(argv[argc++], ptr);
				ptr = strtok(NULL, " ");
			}
			if (argc < 2 || strcmp(argv[0], "delete")) {
				fprintf(stderr, "usage : DELETE [FILENAME] [END_TIME] [OPTION]\n");
				continue;
			}
			curpid = doDelete(argc, argv); //자식이 리턴하면 pid2 = 0, 부모가 성공리턴이면 >0
		}
		else if (strstr(command, "size") != NULL) {
			ptr = strtok(command, " ");
			while (ptr != NULL) {
				strcpy(argv[argc++], ptr);
				ptr = strtok(NULL, " ");
			}
			if (argc < 2 || strcmp(argv[0], "size")) {
				fprintf(stderr, "usage : SIZE [FILENAME] [OPTION]\n");
				continue;
			}
			doSize(argc, argv);
		}
		else if (strstr(command, "recover") != NULL) { //recover
			ptr = strtok(command, " ");
			while (ptr != NULL) {
				strcpy(argv[argc++], ptr);
				ptr = strtok(NULL, " ");
			}
			if (argc < 2 || strcmp(argv[0], "recover")) {
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

char *rmvpath(char *str) { //앞의 경로는 제외하고 이름만 뽑아내는 함수
	char *start;

	start = str + strlen(str) - 1;
	while (*start != '/')
		--start;
	str = start + 1;

	return str;
}


long getDirSize(char *dirName) {
	//파라미터로 입력받은 dirName디렉토리의 사이즈를 구해서 리턴해주는 함수
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmp[BUFLEN];
	int type;
	off_t dsize = 0;
	if ((dp = opendir(dirName)) == NULL) { //파일 오픈 실패한 경우 0리턴
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

		if ((statbuf.st_mode & S_IFMT) == S_IFREG) { //일반파일인 경우 파일사이즈 더해줌
			dsize += statbuf.st_size;
		}
		else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { //디렉토리인 경우
			dsize += getDirSize(tmp); //재귀호출하여 해당 디렉토리의 파일을 구해서 더해줌
		}
	}
	closedir(dp);
	return dsize; //총 디렉토리의 사이즈 리턴
}

int deleteDir(char *dirname) {
    //파라미터로 입력받은 디렉토리 내의 모든 파일을 삭제하는 함수
    int cnt, i;
    struct dirent **flist;
    char buf[BUFLEN], fname[BUFLEN];
    struct stat statbuf;

    if ((cnt = scandir(dirname, &flist, NULL, alphasort)) == -1) {
        fprintf(stderr, "scandir error\n");
        return false;
    }
    for (i=0; i<cnt;i++) {
        strcpy(fname, flist[i]->d_name);

        if (!strcmp(fname, ".") || !strcmp(fname, ".."))
            continue;
        sprintf(buf, "%s/%s", dirname, fname);
        printf("파일이름 : %s\n", buf);
        if(stat(buf, &statbuf) < 0) {
            fprintf(stderr, "stat error\n");
            return false;
        }

        if ((statbuf.st_mode & S_IFMT) == S_IFREG) { //일반파일인 경우
            remove(buf); //바로파일삭제
        }
        else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
            deleteDir(buf); //재귀호출로 해당디렉토리 내의 파일 모두 삭제
            remove(buf); //그후 빈 디렉토리인 자기자신도 삭제
        }
    }
    return true;
}
        
        
        

void removeOldFile() {
	oldFileList lists[BUFLEN];
	int cnt, i;
	char oldpath[BUFLEN], oldname[BUFLEN], dupfiles[BUFLEN][BUFLEN];
	char buf[BUFLEN];
    struct stat statbuf;

	cnt = get_old_files(lists);
	sort_old_files(lists, cnt);
	//sort한 후, lsits[0]번째 파일이 제일 삭제시간이 오래됨!
	strcpy(oldname, rtrim(lists[0].fname));

	int dupn = 0;
	if (isDup(filesDir, oldname, &dupn, dupfiles) < 0) {
		fprintf(stderr, "isdup error\n");
		return;
	}
	if (dupn == 1) { //파일이 하나 뿐인경우
		get_path(filesDir, oldname, oldpath); //삭제할 files디렉토리의 원본파일 이름 가져오기
		
        printf("삭제할 이름 : %s\n경로 : %s\n", oldname, oldpath);
        if (stat(oldpath, &statbuf) < 0) {
            fprintf(stderr, "stat error\n");
            return;
        }
        if ((statbuf.st_mode & S_IFMT) == S_IFREG) //파일이면 바로삭제가능
            remove(oldpath);
        else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { //디렉토리면 안의 내용 다지워야 삭제가능
            deleteDir(oldpath);
            remove(oldpath);
        }
		//info 디렉토리 파일 정보 삭제하기
		sprintf(buf, "%s/%s", infoDir, oldname);
		remove(buf);
	}
	else { //중복파일이 존재하는 경우 -> 해당 이름 가진 원본파일 모두 삭제 후 info파일 삭제
		for (i = 0; i < dupn; i++) {
			sprintf(buf, "%s/%s", filesDir, dupfiles[i]);
			remove(buf);
		}
		//info디렉토리의 정보 삭제
		sprintf(buf, "%s/%s", infoDir, oldname);
		remove(buf);
	}


}

char *rmvdelimeter(char *str) {
	//str으로 입력받은 파일이름에서 (*숫자)를 제거해서 원본 이름을 리턴해줌
	char tmp[BUFLEN];
	char *start, *end;

	strcpy(tmp, str);
	start = tmp;
	while (*start != '\0' && *start != '*')
		++start;
	end = start;
	start = tmp;
	*(end) = '\0';

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

		if ((statbuf.st_mode & S_IFMT) == S_IFREG) { //일반파일인 경우
			strcpy(dname, rmvdelimeter(dirp->d_name));
			if (!strcmp(fname, dname)) {  //delimeter 제거하고 이름 비교하기
				strcpy(pathname, tmpname); //내가찾는 파일이면 경로까지 합친 이름 저장
				return true;
			}
		}
		else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { //디렉토리인 경우
			sprintf(nxtdir, "%s/%s", dirName, dirp->d_name);
			result = isExist(nxtdir, fname, pathname); //재귀호출해서 해당 디렉토리 내의 파일도 검색함
		}
	}
	if (result)
		return true;
	return false;
}

int makeInfo(char *fname, char *fnamepath, struct tm dtm) {
	//fnamepath : 원래 있던 절대경로, tm : 삭제시간(함수호출시간)
	//fname : 파일의 원본 이름

	struct stat statbuf;
	time_t t;
	struct tm mtm;
	char dtime[BUFLEN], mtime[BUFLEN];
	char info[BUFLEN], infopath[BUFLEN];
	char trashinfo[BUFLEN] = "[Trash info]";
	FILE *fp;

	sprintf(infopath, "%s/%s", infoDir, rmvdelimeter(fname)); //경로를 제거한 파일이름으로 info파일생성

	if ((stat(fnamepath, &statbuf)) < 0) {
		fprintf(stderr, "stat error\n");
		return false;
	}
	sprintf(dtime, "D : %d-%02d-%02d %02d:%02d:%02d\n", dtm.tm_year + 1900, dtm.tm_mon + 1, dtm.tm_mday, dtm.tm_hour, dtm.tm_min, dtm.tm_sec); //파일의 삭제시간 문자열에 저장
	t = statbuf.st_mtime;
	mtm = *gmtime(&t);
	sprintf(mtime, "M : %d-%02d-%02d %02d:%02d:%02d\n", mtm.tm_year + 1900, mtm.tm_mon + 1, mtm.tm_mday, mtm.tm_hour, mtm.tm_min, mtm.tm_sec); //파일의 수정시간 문자열에 저장

	if (access(infopath, F_OK) < 0) { //해당 파일이 존재하는지 확인
		if ((fp = fopen(infopath, "w")) == NULL) { //없으면 해당 이름으로 새로 생성
			fprintf(stderr, "fopen error for %s\n", fname);
			return false;
		}
		fprintf(fp, "%s\n", trashinfo); //첫 생성시 [Trash Info] 문자열 추가
	}
	else { //파일이 이미 존재한다면, 기존파일에 내용 추가
		if ((fp = fopen(infopath, "a")) == NULL) {
			fprintf(stderr, "fopen error for %s\n", fname);
			return false;
		}
	}
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

int find_max_num(char *fname) { //files디렉토리 내의 fname을 가진 중복파일들 중 가장 큰 사본번호+1을 리턴 (다음 파일을 저장하기 위해서)
	int cnt, i, max = 0, tmpnum = 0;
	struct dirent **flist;
	char file[BUFLEN];
	char *s;
	if ((cnt = scandir(filesDir, &flist, except_tmp_file, alphasort)) == -1) {
		fprintf(stderr, "scandir error\n");
		return false;
	}

	for (i = 0; i < cnt; i++) {
		if (!strcmp(rmvdelimeter(flist[i]->d_name), fname)) { //내가 찾고자하는 파일발견
			strcpy(file, flist[i]->d_name);

			s = file;
			while (*s != '\0' && *s != '*') //딜리미터 이전까지
				++s;
			++s; //딜리미터 이후 (숫자 얻어오도록)
			tmpnum = atoi(s); //딜리미터 번호 저장
			if (tmpnum > max) //max보다 크면 갱신
				max = tmpnum;
		}
	}
	return max; //amx리턴
}


int doDelete(int argc, char(*argv)[BUFLEN]) {
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
	int isdel = false, delnum;
	int nums = 0, res;
	char dupfiles[MAXFILE][BUFLEN];
	char delimeter[BUFLEN];

	pid_t pid;

	while (1) { //infoDir크기 확인
		dsize = getDirSize(infoDir);
		if (dsize > 2000) { //2KB넘을 경우 제일 오래된 파일 삭제
			removeOldFile();
		}
		else
			break;
	}

	strcpy(fname, rtrim(argv[1]));
	if (strlen(fname) < 0) {
		fprintf(stderr, " %s file name error!\n", argv[1]);
		return false;
	}

	if (!isExist(checkDir, fname, fullpath)) { //상대경로(파일이름)로 파일 존재 확인
		if (get_path(checkDir, fname, fullpath) == 0) { //파일이름으로 디렉토리 존재 확인
			strcpy(tmpbuf, rmvpath(fname));
			if (!isExist(checkDir, tmpbuf, fullpath)) { //절대경로로 파일 존재 확인
				if (get_path(checkDir, tmpbuf, fullpath) == 0) { //절대경로로 파일아닌 디렉토리도 확인
					fprintf(stderr, "%s file don't exist!\n", fname); //그래도 없으면 메시지 출력 후 종료
					return false;
				}
			}

			else { //절대경로이면 파일 이름만 fname에 저장
				strcpy(fname, tmpbuf);
			}
		}
	}

	for (i = 0; i < argc; i++) { //옵션저장
		if ((ptr = strstr(argv[i], "-r")) != NULL)
			rOption = true;
		if ((ptr = strstr(argv[i], "-i")) != NULL)
			iOption = true;
	}


	if (iOption && argc == 3) { //삭제시간(ENDTIME) 없이 i옵션만 사용한 경우
		//삭제할 파일이 디렉토리인경우 에러처리
		if ((stat(fullpath, &statbuf)) < 0) {
			fprintf(stderr, "stat error\n");
			return false;
		}
		if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
			fprintf(stderr, "directory can not use -i option\n");
			return false;
		}
		remove(fullpath); //해당 파일을 trash에 보내지 않고 바로 삭제처리
		return false;
	}


	//trash 디렉토리 안에 중복파일있는지 확인
	res = isExist(filesDir, fname, filespath); //하나라도 있으면 true(1) 리턴
	if (res) {
		res = isDup(filesDir, fname, &nums, dupfiles);
		//모든 파일 저장해서 개수 nums, 파일이름 dupfiles에 저장
		if (res == 0) {
			fprintf(stderr, "isDup Error\n");
			return false;
		}
		if (nums > 0) { //중복된 파일이 있는경우
			//파일 이름 뒤에 delimeter숫자 추가해 이름변경 (a.txt*1, a.txt*2, ...)
			strcpy(newname, fname);
			delnum = find_max_num(fname);
			sprintf(delimeter, "%c%d", '*', delnum + 1); //이렇게 하면 중간꺼 복구 후 다시 삭제하면 겹쳐버림 안됨!!
			strcat(newname, delimeter); //files파일에 저장할 새로운 이름 생성
			strcpy(fname, newname);
		}
	}


	if (argc == 2) { //ENDTIME 없는경우 즉시삭제
		nowt = time(NULL);
		tm = *localtime(&nowt); //삭제시간 저장    

		if (makeInfo(fname, fullpath, tm) < 0)
			return false;
		if (intoTrash(fname, fullpath) < 0)
			return false;
		return true;
	}

	//여기부터는 삭제시간 있는 경우
	//삭제시간 기다리기 - 커널은 대기 불가능하므로 fork()로 자식 프로세스가 대기 후 삭제 수행
	if ((pid = fork()) < 0) {
		fprintf(stderr, "fork error\n");
		return false;
	}
	else if (pid == 0) { //자식프로세스는 doPrompt를 수행하려 간다
		return pid;
	}

	else if (pid > 0) { //부모프로세스인 경우 삭제시간을 기다린 후 수행

		strcpy(deldate, rtrim(argv[2]));
		strcpy(deltime, rtrim(argv[3]));

		//입력받은 삭제시간이 현재보다 이전인 경우 -> 에러처리
		nowt = time(NULL);
		tm = *localtime(&nowt);
		sprintf(curdate, "%d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
		sprintf(curtime, "%02d:%02d", tm.tm_hour, tm.tm_min);
		if ((strcmp(curdate, deldate) == 1 || strcmp(curtime, deltime) == 1)) {
			fprintf(stderr, "delete time not be past!\n");
			return false;
		}

		while (1) {
			nowt = time(NULL); //현재 시간을 [ENDTIME] 포맷에 맞춰서 저장함
			tm = *localtime(&nowt);
			sprintf(curdate, "%d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
			sprintf(curtime, "%02d:%02d", tm.tm_hour, tm.tm_min);
			if (!strcmp(curdate, deldate) && !strcmp(curtime, deltime)) { //만든 시간 문자열로 비교
				//삭제시간이 된 경우, doPrompt 실행중인 자식프로세스를 죽이고 파일 삭제를 진행
				kill(pid, SIGKILL);
				if (rOption) {
					printf("Delete [y/n]? ");
					scanf(" %c", &answer);
					getchar();
					if (answer == 'y') {
						isdel = true; //삭제 진행을 위해 while문 빠져나가기
						break;
					}
					else if (answer == 'n')
						return true; //삭제하지 않고 종료 
				}
				isdel = true;
				break;
			}
			if (isdel)
				break;
		}


		//삭제시간 된 경우
		if (iOption) { //-i옵션 사용된 경우
			//삭제할 파일이 디렉토리인경우 에러처리
			if ((stat(fullpath, &statbuf)) < 0) {
				fprintf(stderr, "stat error\n");
				return false;
			}
			if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
				fprintf(stderr, "directory can not use -i option\n");
				return false;
			}
			remove(fullpath);
			return true;
		}
		//i옵션 없는경우
		nowt = time(NULL);
		tm = *localtime(&nowt); //삭제시간 저장

		if (makeInfo(fname, fullpath, tm) < 0) //infoDir에 삭제할 파일정보 저장
			return false;
		if (intoTrash(fname, fullpath) < 0) //삭제파일 filesDir로 이동!(rename)
			return false;

		return true;
	}
}

int get_path(char *path, char *fname, char *pathname) {
	//fname이름을 가진 파일(일반파일/디렉토리)찾아서 경로를 pathname에 저장 (있으면 1, 없으면 0리턴)
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmp[BUFLEN], nxtpath[BUFLEN];
	int res = false;

	if ((dp = opendir(path)) == NULL) { //현재 실행파일의 경로
		fprintf(stderr, "opendir error for %s\n", path);
		return false;
	}

	while ((dirp = readdir(dp)) != NULL) { //해당 디렉토리 내의 모든 파일 탐색
		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;
		sprintf(tmp, "%s/%s", path, dirp->d_name); //파일 절대경로 저장
		if (stat(tmp, &statbuf) < 0) { //stat()으로 파일정보확인
			fprintf(stderr, "stat error\n");
			return false;
		}

		if ((statbuf.st_mode & S_IFMT) == S_IFREG) { //일반 파일인 경우
			if (!strcmp(dirp->d_name, fname)) { //내가 찾는 이름과 동일한 경우
				strcpy(pathname, tmp); //해당파일의 절대경로를 pathname에 저장
				return true;
			}
		}

		else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { //디렉토리인 경우
			if (!strcmp(dirp->d_name, fname)) { //내가 찾는 파일이름과 동일한경우
				strcpy(pathname, tmp); //해당 파일의 절대경로를 pathname에저장해 리턴
				return true;
			}
			else { //내가 찾는 이름이 아닌경우
				strcpy(nxtpath, tmp); //해당 디렉토리의 경로 저장
				res = get_path(nxtpath, fname, pathname); //해당 디렉토리 하위를 탐색하기위해서 재귀호출
				if (res)
					return true;
			}
		}

	}
	if (res)
		return true;
	else
		return false;

}

int do_dOption(char *printpath, char *statpath, int depth, int curdepth) {
	int cnt = 0;
	int i;
	struct stat statbuf;
	char nxtpath1[BUFLEN], nxtpath2[BUFLEN], tmp[BUFLEN], fname[BUFLEN];
	struct dirent **filelist;
	long fsize;

	if (curdepth > depth)
		return false;


	if ((cnt = scandir(statpath, &filelist, except_tmp_file, alphasort)) == -1) {
		fprintf(stderr, "scandir error for %s\n", printpath);
		return false;
	}

	for (i = 0; i < cnt; i++) {

		sprintf(fname, "%s/%s", printpath, filelist[i]->d_name); //출력용 상대경로

		sprintf(tmp, "%s/%s", statpath, filelist[i]->d_name); //stat용 

		if (stat(tmp, &statbuf) < 0) { //stat으로 파일정보 가져옴
			fprintf(stderr, "stat error\n");
			return false;
		}
		if ((statbuf.st_mode & S_IFMT) == S_IFREG) { //일반파일인 경우
			if (curdepth <= depth) { //아직 입력받은 number(depth)만큼 출력안한경우
				fsize = statbuf.st_size; //파일 사이즈 및 상대경로 출력
				printf("%ld     %s\n", fsize, fname);
			}
		}
		else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { //디렉토리인 경우
			if (curdepth <= depth) { //아직 입력받은 number(depth)만큼 출력 안한경우
				fsize = getDirSize(tmp); //getDirSize()로 해당디렉토리 사이즈 구하기
				printf("%ld     %s\n", fsize, fname);  //파일 사이즈및 상대경로 출력
			}
			if (curdepth < depth) { //아직 number보다 덜탐색한 경우, 재귀호출
				sprintf(nxtpath1, "%s/%s", printpath, filelist[i]->d_name);
				sprintf(nxtpath2, "%s/%s", statpath, filelist[i]->d_name);
				do_dOption(nxtpath1, nxtpath2, depth, curdepth + 1);
			}
		}
	}

}


void doSize(int argc, char(*argv)[BUFLEN]) {
	char fname[BUFLEN], pathname[BUFLEN], path[BUFLEN] = ".";
	char *ptr;
	int number = 0, i, dOption = false;
	long fsize;
	struct stat statbuf;

	strcpy(fname, rtrim(argv[1]));

	if (argc == 4) { //옵션 저장
		if ((ptr = strstr(argv[2], "-d")) != NULL) //-d옵션 사용되었으면 dOption true로!
			dOption = true;
		else {
			fprintf(stderr, "option error!\n");
			return;
		}
		number = atoi(argv[3]); //탐색할 number를 저장
	}

	chdir(workDir); //상대경로를 얻기 위해 현재 실행 디렉토리로 이동

	isExist(workDir, fname, pathname); //일반파일일 경우 전체 경로를 알아오기 위해 호출

	stat(pathname, &statbuf);  //stat()으로 파일 정보 가져옴 (stat실패해도 아래에서 한번 더 검사함. isExist로 가져온 pathname으로는 일반파일밖에 확인못하기 때문)

	if ((statbuf.st_mode & S_IFMT) == S_IFREG) { //일반파일인 경우,
		if (get_path(path, fname, pathname) == 0) { //상대경로 가져오기
			fprintf(stderr, "get_path error\n");
			return;
		}
		fsize = statbuf.st_size; //해당 파일의 사이즈 구하기
		printf("%ld     %s\n", fsize, pathname); //상대경로와 사이즈 출력
		return;
	}
	else { //디렉토리이거나, 파일이 존재하지 않는 경우 (isExist함수는 디렉토리까지 찾을수없기때문에 get_path로 한번 더 확인해야함)
		if (get_path(path, fname, pathname) == 0) {
			fprintf(stderr, "%s is not exist!\n", fname);
			return;
		}
		if ((fsize = getDirSize(pathname)) == 0) {
			fprintf(stderr, "getDirSize error\n");
			return;
		}
		printf("%ld     %s\n", fsize, pathname);
		if (argc == 4 && dOption) { //옵션 사용한 경우 다른 파일들도 출력
			strcpy(path, pathname);
			do_dOption(path, checkDir, number, 2); //depth 2부터 함수로 출력
		}
	}

	return;
}

int isDup(char *dirName, char *fname, int *num, char(*dupfiles)[BUFLEN]) {
	//해당 파일과 동일한 이름을 가진 중복파일이 존재하는지 확인하여 정보를 저장해주는 함수
	//fname과 동일한 이름을 가진 파일들의 개수를 num에 저장, 해당 파일의 이름들을 dupfiles에 저장
	char tmp[BUFLEN];
	char nxtDir[BUFLEN];
	char delname[BUFLEN];
	struct stat statbuf;
	int i, cnt;
	struct dirent **flist;

	//파일정보를 읽어올 디렉토리를 오픈해서 모든 파일 정보를 flist에 가져옴. 임시파일은 제거하여 오름차순으로 가져옴
	if ((cnt = scandir(dirName, &flist, except_tmp_file, alphasort)) == -1) {
		fprintf(stderr, "scandir error\n");
		return false;
	}
	for (i = 0; i < cnt; i++) { 
		if (!strcmp(flist[i]->d_name, ".") || !strcmp(flist[i]->d_name, ".."))
			continue; //파일이름이 "."이나 ".."는 필요없으므로 skip
		strcpy(tmp, dirName); 
		strcat(tmp, "/");
		strcat(tmp, flist[i]->d_name); //이외의 파일은 stat()을 위해서 경로를 합쳐 풀네임 만듦

		if (stat(tmp, &statbuf) < 0) { //stat()으로 파일 정보 가져옴
			fprintf(stderr, "stat error\n");
			return false;
		}
		if ((statbuf.st_mode & S_IFMT) == S_IFREG) { //일반파일인 경우
			strcpy(delname, rmvdelimeter(flist[i]->d_name)); //내가 찾는 파일과 동일한 파일인지 확인하기 위해 구분자를 제거하여 저장
			if (!strcmp(fname, delname)) { //구분자(*숫자) 제거후 이름 비교
				strcpy(dupfiles[(*num)++], flist[i]->d_name); //찾는 이름과 동일하면 이름 복사해 저장
			}
		}
		else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { //디렉토리일 경우 재귀호출해서 더 검색
			strcpy(delname, rmvdelimeter(flist[i]->d_name));
            if (!strcmp(fname, delname)) { //우리가 찾는 이름일경우 dubfile에 이름저장
                strcpy(dupfiles[(*num)++], flist[i]->d_name);
            }
            /*else { //아닌경우 재귀호출
                sprintf(nxtDir, "%s/%s", dirName, flist[i]->d_name);
			    isDup(nxtDir, fname, num, dupfiles); //해당디렉토리로 경로를 만들어서 재귀호출
		    }*/ 
        }
	}
	return true;
}

int printDup(int num, char *fname, char(*dupFiles)[BUFLEN]) {
	//isDup()으로 읽어온 중복 파일 정보를 출력한 후, 사용자가 선택한 파일번호를 리턴해줌
	//num은 파일개수, fname은 원본파일이름(사용자가 입력한이름), dupfiles는 저장된 중복파일정보
	FILE *fp;
	int i = 1, choice = 0;
	char path[BUFLEN], buf[BUFLEN], buf1[BUFLEN], buf2[BUFLEN];

	sprintf(path, "%s/%s", infoDir, fname);
	if ((fp = fopen(path, "r")) == NULL) { //해당 파일의 info파일 열기
		fprintf(stderr, "fopen error for %s\n", path);
		return false;
	}
	//dupfiles 생성시 오름차순으로 했기 때문에 파일의 생성순서와 동일하게 나올것이다
	fgets(buf, BUFLEN, fp); //첫번째 [Trash Info] 무시
	while (feof(fp) == 0) { //동일한 파일의 이름을 하나씩 정보를 읽어 모두 출력하기
		if (i > num)
			break;
		fgets(buf, BUFLEN, fp); //두 번째 줄(경로) 무시
		fgets(buf, BUFLEN, fp); //D시간 읽기
		strcpy(buf1, rtrim(buf)); //D시간 저장
		fgets(buf, BUFLEN, fp); //M시간 읽기
		strcpy(buf2, rtrim(buf)); //M시간 저장
		printf("%d. %s  %s %s\n", i++, fname, buf1, buf2); //파일 정보 출력
	}
	fclose(fp);

	while (choice < 1 || choice > num) { //사용자가 적절한 범위의 번호를 입력할 때 까지 반복
		printf("Choose : ");
		scanf("%d", &choice);
		getchar();
	}

	return choice;
}

int get_recover_file(char *recoverfile, char *recoverpath, int isDup, int choice, int nums) {
	//파라미터 : recoverfile : 복구할 files내의 파일이름(사용자가 선택, 혹은 유일한 것)
	//           recoverpath : info 파일에 저장된 복구할 절대경로
	//           isDup       : 여러개의 중복파일이 존재하는지, 유일한지에 대한 정보
	//           choice      : 몇 번째 파일을 복구해야 하는지가 저장되어있음
	//           nums        : 중복 파일 존재시 총 중복 파일의 개수
	//recoverfile로 recoverpath를 찾은 후, 해당 info 정보를 지우는 함수임
	char fname[BUFLEN], pathname[BUFLEN], tmpfile[BUFLEN];
	FILE *fp, *fp2;
	char buf[BUFLEN];
	int i;

	strcpy(fname, rmvdelimeter(recoverfile));
	sprintf(tmpfile, "%s*", pathname);

	if (get_path(infoDir, fname, pathname) == 0) { //해당 info파일 못찾음
		fprintf(stderr, "file don't exist\n");
		return false;
	}

	if ((fp = fopen(pathname, "r+")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", pathname);
		return false;
	}

	if (isDup == false) { //중복파일없는경우 해당파일 복구경로 저장후, info는 바로삭제하면됨
		fgets(buf, BUFLEN, fp); //[Trash Info] 필요없는 부분
		fgets(buf, BUFLEN, fp); //파일의 절대 경로가 저장된 부분
		if (strstr(buf, fname) == NULL) { //해당줄에서 파일이름이 검색안되면 경로가 없는것
			fprintf(stderr, "recover path don't exist!\n"); //에러처리 해줌
			fclose(fp);
			return false;
		}
		strcpy(recoverpath, buf); //복구할 경로를 recoverpath에 저장
		fclose(fp);
		remove(pathname); //해당 info파일k, 더이상 필요 없으므로 삭제
		return true;
	}

	else { //중복파일있는경우 해당 info의 경로만 저장한 후, 해당  내용만 지워줘야 함.    
		if ((fp2 = fopen(tmpfile, "w")) == NULL) { //복구할 파일 제외한 부분은 파일에 남겨두기 위한 임시파일
			fprintf(stderr, "fopen error for %s\n", tmpfile);
			fclose(fp);
			fclose(fp2);
			return false;
		}

		fgets(buf, BUFLEN, fp); //[Trash info] skip
		fputs(buf, fp2);
		for (i = 1; i <= nums; i++) { //파일의 끝에 도달할 때 까지(복구할 파일 제외한)내용 옮기기
			if (i == choice) {
				//내가 원하는 부분임
				fgets(buf, BUFLEN, fp); //절대경로
				if (strstr(buf, fname) == NULL) { //해당 줄에 파일이름이 없다면 경로가 없는것임!
					fprintf(stderr, "recover path don't exist!\n");
					return false;
				}
				strcpy(recoverpath, buf); //원하는 경로 저장
				fgets(buf, BUFLEN, fp); //dtime
				fgets(buf, BUFLEN, fp); //mtime
			}
			else { //복구할 부분을 제외한 파일은 그대로 남아있어야 하므로 다른파일에 복사
				fgets(buf, BUFLEN, fp); //절대경로 저장부분
				fputs(buf, fp2); //파일에 옮겨줌
				fgets(buf, BUFLEN, fp); //dtime
				fputs(buf, fp2);
				fgets(buf, BUFLEN, fp);  //mtime
				fputs(buf, fp2);
			}
		}
		//파일 복사 완료
		fclose(fp);
		fclose(fp2);
		remove(pathname); //원본 파일 삭제
		rename(tmpfile, pathname); //복사해둔 파일을 rename()해서 원본파일 이름과 동일하게 함.
		return true;
	}
}


int intoCheck(char *fname, char *pathname) {
	//pathname : 저장될경로, fname : 기존이름(경로 포함됨), 성공시 1 에러시 0리턴
	char dupfiles[MAXFILE][BUFLEN], tmpbuf[BUFLEN];
	char curname[BUFLEN], curpath[BUFLEN], newname[BUFLEN], newpath[BUFLEN];
	int isdup = false, dupnum = 0;

	strcpy(pathname, rtrim(pathname)); //공백있는경우 제거
	strcpy(curname, rmvpath(fname)); //경로 제외해서 이름만 curname에 저장
	sprintf(curpath, "%s/%s", filesDir, curname); //이동시킬 파일 경로

	strcpy(newname, rmvdelimeter(curname)); //저장될 이름 저장
	isdup = isExist(checkDir, newname, tmpbuf); //딜리미터 제거한 이름(원래이름)이 중복인지 확인
	if (isdup) { //중복 파일 존재시 숫자_추가 후 rename()

		isDup(checkDir, newname, &dupnum, dupfiles); //check파일에 존재하는 파일의 개수 가져오기
		sprintf(newpath, "%s/_%d%s", checkDir, dupnum + 1, newname);

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

int get_old_files(oldFileList lists[BUFLEN]) {
	//info파일을 열어서, 파일의 이름과 해당파일의 삭제시간을 oldFileList 배열에 저장해준다.
	//리턴값은 총 파일의 개수이다.
	FILE *fp;
	int i, cnt, j = 0, k;
	struct dirent **flist;
	char fname[BUFLEN], fpath[BUFLEN], dupfiles[BUFLEN][BUFLEN];
	int dupn;
	char buf[BUFLEN];

	if ((cnt = scandir(infoDir, &flist,NULL, alphasort)) == -1) {
		fprintf(stderr, "scandir error\n");
		return false;
	}

	for (i = 0; i < cnt; i++) {
		dupn = 0;
    
		strcpy(fname, flist[i]->d_name);
		if (!strcmp(fname, ".") || !strcmp(fname, ".."))
            continue;
    
        sprintf(fpath, "%s/%s", infoDir, fname);
        
		isDup(filesDir, fname, &dupn, dupfiles);  //filesDir에 해당 이름 가진 파일이 여러개 있는지 확인`

		if ((fp = fopen(fpath, "r")) == NULL) {
			fprintf(stderr, "fopen error for %s\n", fpath);
			return false;
		}
		fgets(buf, BUFLEN, fp); //[Trash info]부분 필요x
		if (dupn == 1) { //중복파일이 없는경우 (하나만 존재)
			//printf("중복없음\n");
			strcpy(lists[j].fname, rtrim(fname)); //파일 이름 저장
			fgets(buf, BUFLEN, fp); //경로부분 필요x
			fgets(buf, BUFLEN, fp); //dtime 필요O
			strcpy(lists[j++].dtime, rtrim(buf)); //개행 제거해서 저장해야함
            fclose(fp);
		}
		else { //여러개인 경우
			for (k = 0; k < dupn; k++) { //중복이 있는 경우 여러개의 시간을 다 저장
				strcpy(lists[j].fname, rtrim(fname)); //개행 제거 후 저장
				fgets(buf, BUFLEN, fp); //경로 필요x
				fgets(buf, BUFLEN, fp); //dtime 저장해야함
				strcpy(lists[j++].dtime, rtrim(buf));
                fgets(buf, BUFLEN, fp); //mtime 필요x
			}
			fclose(fp);
		}
	}
	return j; //총 저장한 파일의 개수를 리턴!
}


int compareTime(char *tm1, char *tm2) { //문자열로 된 시간 두개를 비교하여 tm1이 오래된 경우 true(1)을 리턴, tm2가 더 오래된 경우 false(0)을 리턴
	char t1[BUFLEN], t2[BUFLEN];
	char buf[BUFLEN], date1[BUFLEN], time1[BUFLEN], date2[BUFLEN], time2[BUFLEN];
	char c1, c2;
	int y1, m1, d1, y2, m2, d2, h1, mm1, s1, h2, mm2, s2;

	strcpy(t1, tm1);
	strcpy(t2, tm2);

	sscanf(t1 + 3, "%s %s", date1, time1); //D : 이후로 날짜부분, 시간부분 문자열로 저장
	sscanf(t2 + 3, "%s %s", date2, time2);

	sscanf(date1, "%d%c%d%c%d", &y1, &c1, &m1, &c2, &d1); //날짜 가져와서 정수로 저장
	sscanf(time1, "%d%c%d%c%d", &h1, &c1, &mm1, &c2, &s1); //시간 문자열 정수에 저장

	sscanf(date2, "%d%c%d%c%d", &y2, &c1, &m2, &c2, &d2);
	sscanf(time2, "%d%c%d%c%d", &h2, &c1, &mm2, &c2, &s2);

	//년도, 월, 일, 시간, 분, 초 순으로 비교하여 tm1이 오래되면 1리턴 tm2가 오래되면 0리턴
	if (y1 > y2)
		return true;
	else if (y1 < y2)
		return false;

	if (m1 > m2)
		return true;
	else if (m1 < m2)
		return false;

	if (d1 > d2)
		return true;
	else if (d1 < d2)
		return false;

	if (h1 > h2)
		return true;
	else if (h1 < h2)
		return false;

	if (mm1 > mm2)
		return true;
	else if (mm1 < mm2)
		return false;

	if (s1 > s2)
		return true;
	else if (s1 < s2)
		return false;
	else 
		return -1;
}

void sort_old_files(oldFileList lists[BUFLEN], int cnt) {
	int i, j;
	char t1[BUFLEN], t2[BUFLEN];
	oldFileList tmp;

	//인덱스 0번부터 오래된 파일이 정렬되도록 함
	for (i = cnt - 1; i > 0; i--) {
		for (j = 0; j < i; j++) {
			strcpy(t1, lists[j].dtime); //비교할 시간 문자열 복사
			strcpy(t2, lists[j + 1].dtime);
			if (compareTime(t1, t2) == 1) {//삭제시간이 최근인게 더 앞에 있는경우 -> swap해서 오래된 순으로 되도록 함
				tmp = lists[j];
				lists[j] = lists[j + 1];
				lists[j + 1] = tmp;
			}
		}
	}

}

void do_lOption() {
	oldFileList lists[BUFLEN];
	int i, cnt;
   
    memset(lists, (char)0, sizeof(oldFileList)*BUFLEN);

	cnt = get_old_files(lists); //trash info파일에 있는 모든 파일의 D시간 가져오기
    sort_old_files(lists, cnt); //삭제시간이 오래된 순으로 정렬
	for (i = 0; i < cnt; i++) { //오래된 순으로 출력
		printf("%d %s\t\t\t%s\n", i + 1, lists[i].fname, lists[i].dtime);
	}
	printf("\n");
	return;
}

void doRecover(int argc, char(*argv)[BUFLEN]) {
	char fname[BUFLEN], pathname[BUFLEN];
	int i, lOption = false, choice, res;
	FILE *infofp, *fillefp;
	int dupn = 0;
	char dupfiles[MAXFILE][BUFLEN];
	char mvfname[BUFLEN], orgfname[BUFLEN];

	strcpy(fname, rtrim(argv[1]));
	if (argc == 3)
		if (strstr(argv[2], "-l"))
			lOption = true;

	if (isExist(infoDir, fname, pathname) == 0) { //입력받은 파일이 trash에 존재하지 않는경우 에러처리
		fprintf(stderr, "There is no '%s' in the 'trash' directory\n", fname);
		return;
	}

	if (lOption) { //RECOVER 명령어 실행 전, -l옵션 사용되었으면 먼저 실행
		do_lOption();
	}

	if (isDup(filesDir, fname, &dupn, dupfiles) < 0) {
		//중복파일이 존재하는 경우 개수 dupn에 저장, 해당 파일의 이름들 dupfiles에 저장
		fprintf(stderr, "duplication confirm error \n");
		return;
	}
	if (dupn > 1) { //파일 여러개인 경우 : 중복파일 정보 출력 후 선택된 파일을 복구
		choice = printDup(dupn, fname, dupfiles); //중복개수, 원본이름, 중복파일 이름 저장
		if (choice == false) {
			fprintf(stderr, "printDup error\n");
			return;
		}
		if (get_recover_file(dupfiles[choice - 1], mvfname, true, choice, dupn) == 0) {
			return;  //실패시 0이 리턴됨
		}
		//dupfile의 복구할 경로를 mvfname에 저장하고, 해당 내용을 info파일에서 삭제
		//선택한 파일의 delimeter 지운 파일 이름으로 recover 해야함
		sprintf(orgfname, "%s/%s", filesDir, dupfiles[choice - 1]);

		intoCheck(orgfname, mvfname); //check디렉토리로 다시 복구시킴(rename()사용)
	}
	else {
		//복구할 디렉토리(check)에 같은 파일 없는경우 
		if (isExist(filesDir, fname, orgfname)) { //같은 파일 없어도 복수파일로 인해 딜리미터가 붙은 이름인경우
			strcpy(pathname, orgfname);
		}

		get_recover_file(fname, mvfname, false, 0, 0); //info파일에 저장되어있는 복구경로를 읽어와 mvfname에 저장해옴
		intoCheck(pathname, mvfname); //복구 경로로 rename()해서 다시 원래 check디렉토리로 복구
	}
	return;
}


int makeTree(int depth, char *dname, char(*fname)[BUFLEN], char *ftype, int *fdep, int idx)
{ //배열 만드는 함수
	int fcnt, allcnt;
	int i, j, fidx = 0;
	struct dirent **filelist;
	char fpath[BUFLEN], tmp[BUFLEN];
	char newdname[BUFLEN];
	struct stat statbuf;
	int size = 0, dsize = 0;

	if ((fcnt = scandir(dname, &filelist, NULL, alphasort)) == -1) {
		fprintf(stderr, "scandir error for checkDir\n");
		exit(1);
	}
	size = fcnt;

	for (fidx = 0, i = idx; fidx < fcnt; fidx++, i++) {
		chdir(dname);
		strcpy(tmp, filelist[fidx]->d_name);
		if (!strcmp(tmp, ".")) { //현재 디렉토리라는 뜻의 "."
			strcpy(fname[i], tmp);
			if (depth == 0) { //depth가 0이면 check디렉토리(감시디렉토리)이므로
				ftype[i] = 'c'; //check 라는 뜻으로 타입 저장
				fdep[i] = depth; //현재 depth(0)저장
				depth++; //증가
			}
			else { //depth가 0이외의 "."(현재디렉토리)의 경우
				ftype[i] = 'r'; //root라는 뜻으로 r저장 해둠
				fdep[i] = depth;
			}
			continue;
		}

		if (!strcmp(tmp, "..")) { //부모 디렉토리 : ".."
			strcpy(fname[i], tmp);
			fdep[i] = depth; //현재 depth저장
			ftype[i] = 'p'; //parent라는 뜻으로 p타입으로 저장
			continue;
		}
		if (stat(tmp, &statbuf) < 0) { //이외의 파일 이름이 저장된 경우에는 stat()으로 정보 읽어옴
			fprintf(stderr, "stat error for %s\n", tmp);
			return false;
		}

		if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { //디렉토리 파일인 경우
			strcpy(fname[i], tmp);
			ftype[i] = 'd'; //directory라는 뜻에서 d 저장
			fdep[i] = depth;
			sprintf(newdname, "%s/%s", dname, tmp); //디렉토리를 더 탐색하기 위해 경로를 저장
			dsize = makeTree(depth + 1, newdname, fname, ftype, fdep, i + 1); //현재 디렉토리 뒤에 배열을 추가하기 위해 인덱를 더해주고 재귀호출한 후, 이 디렉토리 내의 파일개수를 리턴
			size += dsize; //총 파일개수에 현재 디렉토리 내의 파일개수를 더함
			i += dsize; //인덱스도 그만큼 더해줌
		}
		else if ((statbuf.st_mode & S_IFMT) == S_IFREG) { //일반파일인 경우
			strcpy(fname[i], tmp); //이름저장
			ftype[i] = 'f'; //file이라는 뜻으로 f로 타입저장
			fdep[i] = depth; //현재 depth 저장
		}
		idx = i;
	}
	return size;

}


void printTree(int fsize, char(*fname)[BUFLEN], char *ftype, int *fdep, int idx) {
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

		else if (ftype[i] == 'f') {
			if (ftype[i - 1] == 'p') { //첫 번째 출력되는 일반 파일
				printf("%s", fname[i]); //개행없이 가지옆에 바로 이름 출력
				continue;
			}
			if (depth < 5) {
				if (ftype[i - 1] == 'f' && ftype[i - 2] == 'p') //두 번째 출력되는 일반파일에 공백추가
					printf("\n"); //첫 번째 출력에 개행이 사용안되므로 개행해줌

				//세 번째 이후 출력되는 파일
				for (k = 0; k < 4; k++) {
					for (j = 0; j < depth; j++) {
						if (j == 0)
							printf("%s|", tab);
						else {
							printf("%s", tab);
							for (l = 0; l < j; l++)
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
				//tree의 가지를 출력
				for (j = 0; j < depth; j++) {
					if (j == 0)
						printf("%s|", tab);
					else
						printf("%s |", tab);
				}
				printf("\n");
				for (j = 0; j < depth; j++) {
					if (j == 0)
						printf("%s|", tab);
					else {
						printf("%s", tab);
						for (l = 0; l < j; l++)
							printf(" ");
						printf("|");
					}
				}
			}

			printf("-%s\n", fname[i]); // -파일이름 출력
		}
		else if (ftype[i] == 'd') { //해당 파일이 디렉토리인 경우

			if (ftype[i - 1] == 'p') { //첫 번째 출력되는 디렉토리(이전이p인경우면 이 파일이 해당 디렉토리 내의 상위 첫 번째 파일)
				printf("+%s%s", fname[i], line); //디렉토리는 +와 함께 출력
				continue;
			}

			if (depth < 5) { //depth가 아직 깊지않아서 다른 파일간의 간격을 3줄로 함

				if (ftype[i - 1] == 'f' && ftype[i - 2] == 'p')
					printf("\n"); //두 번째 출력되는 파일이면 이전과의 구분을 위해 스페이스 넣어줘야함

				for (k = 0; k < 4; k++) {
					for (j = 0; j < depth; j++) {
						if (j == 0)
							printf("%s|", tab);
						else {
							printf("%s", tab);
							for (l = 0; l < j; l++) {
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
			else { //depth가 깊어져서 길이 부족해질 때 다른 파일들과의 간격을 1줄로 해줌
				//tree의 가지들 출력
				for (j = 0; j < depth; j++) {
					if (j == 0)
						printf("%s|", tab);
					else
						printf("%s |", tab);
				}
				printf("\n");
				for (j = 0; j < depth; j++) {
					if (j == 0)
						printf("%s|", tab);
					else {
						printf("%s", tab);
						for (l = 0; l < j; l++)
							printf(" ");
						printf("|");
					}
				}
			}
			printf("+%s%s", fname[i], line); //+디렉토리 이름 출력
		}
	}
	printf("\n"); //트리모두 출력 후 개행해줌
}

void doTree() {

	char fname[BUFLEN][BUFLEN];
	char ftype[BUFLEN];
	int fdep[BUFLEN];
	int fsize = 0;
	int depth = 0;
	int i;

	//tree를 그리기 위한 파일정보를 저장할 배열들 초기화
	for (i = 0; i < BUFLEN; i++) {
		memset(fname[i], (char)0, BUFLEN);
	}
	memset(fname, (char)0, BUFLEN);
	memset(ftype, (char)0, BUFLEN);
	memset(fdep, 0, BUFLEN);

	fsize = makeTree(0, checkDir, fname, ftype, fdep, 0); //tree출력하기 위한 파일 정보들 저장, 총 파일의 개수 리턴
	printTree(fsize, fname, ftype, fdep, 0); //파일들을 트리 구조로 출력
	return;
}


void doHelp() { //프로그램 사용법 출력
	printf("Usage : COMMAND [OPTION]\n\n");
	printf("COMMAND : \n");
	printf(" delete [FILENAME] [END_TIME] [OPTION]      지정한 END_TIME 시간에 자동으로 파일 삭제\n");
	printf(" size [FILENAME] [OPTION]                   파일의 경로(상대경로), 파일 크기를 출력\n");
	printf(" recover [FILENAME] [OPTION]                trash 디렉토리 내의 파일을 원래 경로로복구\n");
	printf(" tree                                       check 디렉토리의 구조를 tree 형태로 보여줌\n\n");
	printf("OPTIONS : \n");
	printf(" delete [FILENAME] [END_TIME] -i            삭제시 'trash' 디렉토리로 삭제 파일과 정보를 이동시키지 않고 파일 삭제\n");
	printf(" delete [FILENAME] [END_TIME] -r            지정한 시간에 삭제시 삭제 여부 재확인\n\n");
	printf(" size [FILENAME] -d NUMBER                  NUMBER 단계 만큼의 하위 디렉토리까지 출력\n\n");
	printf(" recover [FILENAME] -l                      trash 디렉토리 밑에 있는 파일과 삭제 시간들을 삭제시간이 오래된 순으로 출력 후, 명령어 진행\n\n");

}
