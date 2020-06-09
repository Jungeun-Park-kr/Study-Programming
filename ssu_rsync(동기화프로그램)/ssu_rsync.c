#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/time.h>
#include "ssu_rsync.h"

#define SECOND_TO_MICRO 1000000
#define TRUE 1
#define FALSE 0
#define BUFLEN 1024

int is_changed = FALSE;
int is_started = FALSE;
int is_finished = FALSE;
int is_logchanged = FALSE;

int rOption = FALSE;
int mOption = FALSE;
int tOption = FALSE;

char srcName[BUFLEN];
char srcPath[BUFLEN];
char srcBackup[BUFLEN];
char dstName[BUFLEN];
char dstPath[BUFLEN];
char dstBackup[BUFLEN];
char logFile[BUFLEN];
char logBackup[BUFLEN];
char usrCommand[BUFLEN];
char workPath[BUFLEN];

FILE *logfp;

int main(int argc, char *argv[]) {
	
	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL); //프로그램 실행 시간 저장

	ssu_rsync(argc, argv);

	gettimeofday(&end_time, NULL); //프로그램 종료 시간 저장

	end_time.tv_sec -= start_time.tv_sec; //프로그램 런타임 구함
	if (end_time.tv_usec < start_time.tv_usec) { 
		end_time.tv_sec--;
		end_time.tv_usec += SECOND_TO_MICRO;
	}
	end_time.tv_usec -= start_time.tv_usec;
	printf("Runtime: %ld:%06ld(sec:usec)\n", end_time.tv_sec, end_time.tv_usec); //런타임 출력 후 종료
	exit(0);
}

void ssu_rsync(int argc, char *argv[]) {
	char saved_path[BUFLEN];
	char cpcommand[BUFLEN];
	struct stat statbuf;

	if (argc < 3) {
		fprintf(stderr, "usage : %s [OPTION] <SRC> <DST>\n", argv[0]);
		exit(1);
	}
	//SIGINT시그널을 등록, cancel_sync_handler()함수 핸들러 등록
	if (signal(SIGINT, cancel_sync_handler) == SIG_ERR) {
		fprintf(stderr, "cannot handle SIGINT\n");
		exit(EXIT_FAILURE);
	}
	//명령어 복사
	if(argc == 3)
		sprintf(usrCommand, "ssu_rsync %s %s", argv[1], rtrim(argv[2]));
	else if (argc == 4)
		sprintf(usrCommand, "ssu_rsync %s %s %s", argv[1], rtrim(argv[2]), argv[3]);
	
	getcwd(saved_path, BUFLEN);
	strcpy(workPath, saved_path);
	sprintf(logFile, "%s/ssu_rsync_log", saved_path);
	if (access(logFile, F_OK)) { //로그파일 없으면 생성
		logfp = fopen(logFile, "w");
		fclose(logfp);
	}
	if (argc == 4) {
		if (realpath(argv[2], srcPath) == NULL) { //src이름 절대경로로 저장
			fprintf(stderr, "realpath error for %s! file don't exist!\n", argv[2]);
			exit(1);
		}
		if (realpath(argv[3], dstPath) == NULL) {  //dst 이름  절대경로로 저장
			fprintf(stderr, "realpath error for %s! file don't exist!\n", argv[3]);
			exit(1);
		}
	}
	else if (argc == 3) {
		if (realpath(argv[1], srcPath) == NULL) { //src이름 절대경로로 저장
			fprintf(stderr, "realpath error for %s! file don't exist!\n", argv[1]);
			exit(1);
		}
		if (realpath(argv[2], dstPath) == NULL) {  //dst 이름  절대경로로 저장
			fprintf(stderr, "realpath error for %s! file don't exist!\n", argv[2]);
			exit(1);
		}
	}
	strcpy(srcName, rmvpath(srcPath)); //src경로 제외 이름만 저장
	strcpy(dstName, rmvpath(dstPath)); //dst 경로 제외한 이름만 저장

	//파일의 존재여부와 접근권한 확인
	if (access(srcPath, F_OK)) { //src파일 존재 확인
		fprintf(stderr, "<%s> file don't exist!\n", srcPath);
		void printUsage();
		exit(1);
	}
	if (access(srcPath, R_OK) || access(srcPath, W_OK)) { //src파일 접근권한
		fprintf(stderr, "you don't have permission of <%s> file\n", srcPath);
		void printUsage();
		exit(1);
	}
	if (access(dstPath, F_OK)) { //dst 파일 존재 확인
		fprintf(stderr, "<%s> file don't exist!\n", dstPath);
		void printUsage();
		exit(1);
	}
	if (access(dstPath, R_OK) || access(dstPath, W_OK)) { //dst파일 접근권한 확인
		fprintf(stderr, "you don't have permission of <%s> file\n", dstPath);
		void printUsage();
		exit(1);
	}
	//dst는 디렉토리인지 확인 필요
	if (stat(dstPath, &statbuf) < 0) {
		fprintf(stderr, "stat error for %s\n", dstPath);
		exit(1);
	}
	//dst 디렉토리 아니면 에러처리 후 종료
	if (!S_ISDIR(statbuf.st_mode)) { //statbuf.st_mode & S_IFMT) ==  S_IFDIR
		fprintf(stderr, "<%s> file is NOT Directory file\n", dstPath);
		void printUsage();
		exit(1);
	}	
	
	sprintf(srcBackup, "%s_backup", srcPath); //백업용 src 이름 저장
	sprintf(dstBackup, "%s_backup", dstPath); //백업용 dst 이름 저장
	sprintf(logBackup, "%s_backup", logFile);

	sprintf(cpcommand, "cp -p -r %s %s", srcPath, srcBackup);
	system(cpcommand);
	sprintf(cpcommand, "cp -p -r %s %s", logFile, logBackup);
	system(cpcommand);
	sprintf(cpcommand, "cp -p -r %s %s", dstPath, dstBackup);
	system(cpcommand);
	
	if (argc == 3) //옵션 없는 경우 src, dst는 1,2번째
		compareFile(argv[1], argv[2]);
	else if (argc == 4) //옵션 사용된경우 src dst는 2,3번째
		compareFile(argv[2], argv[3]);
	
	if (!checkOption(argc, argv)) //옵션 체크
		exit(1);

	if (!doSync()) {
		fprintf(stderr, "doSync error\n");
		exit(1);
	}
}

int checkOption(int argc, char *argv[]) {
	int i, j;
	int c;

	while ((c = getopt(argc, argv, "rmt")) != -1) {
		switch(c) {
			case 'r' :
				rOption = TRUE;
				break;
			case 'm' :
				mOption = TRUE;
				break;
			case 't' :
				tOption = TRUE;
				break;
		}
	}
}

int doSync() {
	struct stat statbuf;
	int fd;
	struct flock lock;

	//src가 디렉토리인지 일반파일인지 확인
	if (stat(srcPath, &statbuf) < 0) {
		fprintf(stderr, "stat error for %s\n", srcPath);
		return FALSE;
	}
	//동기화 하는 파일 못 열게 함
	lock.l_type = F_WRLCK;
	lock.l_whence = 0;
	lock.l_start = 0L;
	lock.l_len = 0L;
	if (fcntl(fd, F_SETLK, &lock) == -1) { //동기화 할 파일 락
		fprintf(stderr, "fcntl error\n");
		return FALSE;
	}
	//동기화 시작
	if ((statbuf.st_mode & S_IFMT) == S_IFREG) { //src파일이 일반 파일인 경우
		syncFile();

		lock.l_type = F_UNLCK; //동기화 후 락 해제
		fcntl(fd, F_SETLK, &lock);

		//백업파일들 삭제 후 리턴
		if (remove(srcBackup) < 0) { //백업src파일 삭제
			fprintf(stderr, "remove error for %s\n", srcBackup);
			return FALSE;
		}
		if (remove(logBackup) < 0) { //백업로그파일 삭제
			fprintf(stderr, "remove error for %s\n", srcBackup);
			return FALSE;
		}
		if (!rmvDir(dstBackup)) { //dst백업 내의 내용 삭제
			fprintf(stderr, "rmvDir error for %s\n", dstBackup);
			return FALSE;
		}
		if (remove(dstBackup) < 0) { //dst파일 자체 삭제
			fprintf(stderr, "remove error for %s\n", dstBackup);
			return FALSE;
		}
		return TRUE;
	}
	else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { //src파일이 디렉토리인 경우
		syncDir();

		lock.l_type = F_UNLCK; //동기화 후 락 해제
		fcntl(fd, F_SETLK, &lock);

		//백업파일들 삭제 후 리턴
		if (!rmvDir(srcBackup)) { //src백업 내의 내용 삭제
			fprintf(stderr, "rmvDir error for %s\n", dstBackup);
			return FALSE;
		}
		if (remove(srcBackup) < 0) { //백업src파일 삭제
			fprintf(stderr, "remove error for %s\n", srcBackup);
			return FALSE;
		}
		if (remove(logBackup) < 0) { //백업로그파일 삭제
			fprintf(stderr, "remove error for %s\n", srcBackup);
			return FALSE;
		}
		if (!rmvDir(dstBackup)) { //dst백업 내의 내용 삭제
			fprintf(stderr, "rmvDir error for %s\n", dstBackup);
			return FALSE;
		}
		if (remove(dstBackup) < 0) { //dst파일 자체 삭제
			fprintf(stderr, "remove error for %s\n", dstBackup);
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
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

	while ((dirp = readdir(dp)) != NULL) { //모든 파일 정보 읽을 때 까지 반복
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

int rmvDir(char *dirname) {
	//파라미터로 입력받은 디렉토리 내의 모든 파일을 삭제하는 함수
	int cnt, i;
	struct dirent **flist;
	char buf[BUFLEN], fname[BUFLEN];
	struct stat statbuf;

	if ((cnt = scandir(dirname, &flist, NULL, alphasort)) == -1) { //모든 파일 정보 가져옴
		fprintf(stderr, "scandir error for %s\n", dirname);
		return FALSE;
	}
	for (i = 0; i < cnt; i++) {
		strcpy(fname, flist[i]->d_name);

		if (!strcmp(fname, ".") || !strcmp(fname, "..")) //무시
			continue;
		sprintf(buf, "%s/%s", dirname, fname); 
		if (stat(buf, &statbuf) < 0) {
			fprintf(stderr, "stat error\n");
			return FALSE;
		}

		if ((statbuf.st_mode & S_IFMT) == S_IFREG) { //일반파일인 경우
			remove(buf); //바로파일삭제
		}
		else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
			rmvDir(buf); //재귀호출로 해당디렉토리 내의 파일 모두 삭제
			remove(buf); //그후 빈 디렉토리인 자기자신도 삭제
		}
	}
	return TRUE;
}

int compareFile( char *file1, char *file2) {
	//두 파일을 비교해서 동일하면 0, 다르면1 리턴
	struct stat statbuf1, statbuf2;
	struct tm tm1, tm2;
	int size1, size2;

	if (stat(file1, &statbuf1) < 0) { //파일정보 가져오기
		fprintf(stderr, "stat error for %s\n", file1);
		return FALSE;
	}
	if (stat(file2, &statbuf2) < 0) { //파일정보 가져오기
		fprintf(stderr, "stat error for %s\n",file2);
		return FALSE;
	}
	tm1 = *localtime(&statbuf1.st_mtime);
	tm2 = *localtime(&statbuf2.st_mtime);
	if (memcmp(&tm1, &tm2, sizeof(struct tm))) { //수정시간 확인
		return TRUE;	//수정시간 다름
	}

	if ((statbuf1.st_mode & S_IFMT) == S_IFREG) { //일반 파일인 경우 바로 크기 저장
		size1 = statbuf1.st_size;
	}
	else if ((statbuf1.st_mode & S_IFMT) == S_IFDIR) { //디렉토리인 경우 함수로 구해서 크기 저장
		size1 = getDirSize(file1);
	}

	if ((statbuf2.st_mode & S_IFMT) == S_IFREG) { //일반 파일인 경우 바로 크기 저장
		size2 = statbuf2.st_size;
	}
	else if ((statbuf2.st_mode & S_IFMT) == S_IFDIR) { //디렉토리인 경우 함수로 구해서 크기 저장
		size2 = getDirSize(file2);
	}
	if (size1 == size2) { //파일 사이즈 동일한 경우
		return FALSE;
	}
	else //파일 사이즈 다른 경우
		return TRUE;
}

int except_tmp_file(const struct dirent *info) { //scandir()에 사용할 함수 (.으로 시작하는 파일 포함x)
	char *name;

	strcpy(name, info->d_name);

	if (info->d_name[0] != '.') //'.'으로 시작하는 swp파일 등은 포함시키지 않음
		return 1;
	else
		return 0;
}

int saveDirInfo(char *path, char(*flist)[BUFLEN], int idx) {
	//path(디렉토리)에 있는 파일 이름 flist에 저장 후 총 파일 개수 리턴
	int cnt = 0, allcnt = 0, filecnt = 0;
	int i, j;
	struct stat statbuf;
	char nxtpath[BUFLEN];
	char buf[BUFLEN];
	struct dirent **filelist;
	
	if ((cnt = scandir(path, &filelist, except_tmp_file, NULL)) == -1) { //모든 파일 가져옴
		fprintf(stderr, "scandir error for %s\n", path);
		return FALSE;
	}
	allcnt = cnt;
	for (i = idx, j = 0; j < cnt; j++) {
		if(rOption && (i!=j)) { //r옵션 때문에 재귀호출 된 경우, 부모 디렉토리까지 같이 출력
			sprintf(buf, "%s/%s", path, filelist[j]->d_name);
		}
		else
			strcpy(buf, filelist[j]->d_name); //해당 파일 이름을 저장

		if (stat(buf, &statbuf) < 0) {
			fprintf(stderr, "saved dir info stat error for %s\n", buf);
			return FALSE;
		}
		
		if ((statbuf.st_mode & S_IFMT) == S_IFREG) { //일반 파일인 경우 바로 저장
			strcpy(flist[i], buf);
			i++;
			filecnt++;
		}
		else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { //디렉토리인 경우
			if (rOption) { //r옵션 설정 되어있으면 재귀호출
				if (i==j) //아직 재귀호출 안된경우
					sprintf(nxtpath, "%s", filelist[j]->d_name);
				else //재귀호출 된 경우
					sprintf(nxtpath, "%s/%s", path, filelist[j]->d_name);
				allcnt += saveDirInfo(nxtpath, flist, (idx + allcnt)); //하위 파일 정보도 저장
				i++;
			}
			else { //r옵션 설정 안되있는 경우 일반파일과 동일하게 저장
				strcpy(flist[i], buf);
				i++;
				filecnt++;
			}
		}
	}
	//메모리 해제 후 파일개수 리턴
	for (i = 0; i < cnt; i++)
		free(filelist[i]);
	free(filelist);
	if (rOption) //rOption 설정된 경우 총 파일 개수
		return allcnt;
	else //rOption 없는경우 파일 개수만 리턴
		return filecnt;
	
}

int mySync(char *originFile, char *syncFile, int modify) {
	//originFile 파일을 이름은 유지한 채 synFile파일으로 동기화 시켜줌
	//modify인자가 TRUE인경우, 파일이 수정되어 동기화, FALSE인 경우, 새로운 파일 생성 동기화
	char cpcommand[BUFLEN];
	if(modify == TRUE)
		if (remove(originFile) < 0) { //기존에 있던 파일 삭제
			fprintf(stderr, "remove error for %s\n", originFile);
			return FALSE;
		}
	sprintf(cpcommand, "cp -r -p %s %s", syncFile, originFile);
	system(cpcommand);

	return TRUE;
}


int syncDir() { //디렉토리 파일을 동기화
	//동기화된 파일의 이름, 크기 출력해야 함
	//파일 이름은 상대경로로 출력해야함, 파일이 삭제된 경우 delete를 기록해야 함
	
	struct stat statbuf;
	char srcflist[BUFLEN][BUFLEN], dstflist[BUFLEN][BUFLEN];
	char srcbuf[BUFLEN], dstbuf[BUFLEN], newName[BUFLEN], timebuf[BUFLEN], filebuf[BUFLEN], logbuf[BUFLEN], tmpbuf[BUFLEN];
	int srccnt, dstcnt, fsize;
	int i, j;
	int isfound, ismodified;
	int res;
	time_t cur_time;
	FILE *fp;

	is_changed = TRUE;
	is_started = TRUE;
	memset(logbuf, 0, BUFLEN);

	chdir(srcPath);
	srccnt = saveDirInfo(srcPath, srcflist,  0); //SRC파일 정보 가져옴
	chdir(dstPath);
	dstcnt = saveDirInfo(dstPath, dstflist,  0); //DST파일 정보 가져옴
	chdir(workPath);

	for (i = 0; i < srccnt; i++) {
		isfound = FALSE;
		ismodified = FALSE;
		sprintf(srcbuf, "%s/%s", srcPath, srcflist[i]);
		for(j=0; j < dstcnt; j++) { //파일존재 여부 확인
			if (!strcmp(srcflist[i], dstflist[j])) {
				isfound = TRUE; //파일은 존재함, 변경되었는지 확인 필요
				sprintf(dstbuf, "%s/%s", dstPath, dstflist[j]);
				if (compareFile(srcbuf, dstbuf)) { //파일 비교해 크기나 수정시간 변경된 경우
					ismodified = TRUE;
					break;
				}
				break;
			}
		}
		if (isfound == FALSE) { //src에 있는데 dst에 없는 건 생성된 것이므로
			sprintf(newName, "%s/%s", dstPath, srcflist[i]); //복사 될 이름 (dst아래 src파일이름)
			//printf("newname:%s\nsrcbuf:%s\n",newName,srcbuf);
			mySync(newName, srcbuf, FALSE); //dst디렉토리에 새로 생성해줌

			//생성된 파일 정보 동기화하여 로그파일에 작성
			if (stat(srcbuf, &statbuf) <0) {
					fprintf(stderr, "stat error for %s\n", srcbuf);
					return FALSE;
			}
			if (S_ISREG(statbuf.st_mode)) { //생성된 파일 정보 로그에 쓰기
				fsize = statbuf.st_size;
				//로그파일에 해당 정보 쓰기 위해 내용 추가
				sprintf(tmpbuf, "\t%s %ldbytes\n", srcflist[i], statbuf.st_size);
				strcat(logbuf, tmpbuf);	
			}
		}
		else if (ismodified == TRUE) { //파일이 변경된 경우
			//기존파일 삭제 후, 동기화 시켜주기
			if (stat(srcbuf, &statbuf) <0) {
					fprintf(stderr, "stat error for %s\n", srcbuf);
					return FALSE;
			}
			if (S_ISREG(statbuf.st_mode)) { //생성된 파일 정보 로그에 쓰기
				mySync(dstbuf, srcbuf, TRUE);
			}
			else if (S_ISDIR(statbuf.st_mode)) //디렉토리면 skip
				continue;

			//수정된 파일 정보 동기화하여 로그파일에 작성
			if (stat(srcbuf, &statbuf) <0) {
					fprintf(stderr, "stat error for %s\n", srcbuf);
					return FALSE;
			}
			if (S_ISREG(statbuf.st_mode)) { //생성된 파일 정보 로그에 쓰기
				fsize = statbuf.st_size;
				//로그파일에 해당 정보 쓰기 위해 내용 추가
				sprintf(tmpbuf, "\t%s %ldbytes\n", srcflist[i], statbuf.st_size);
				strcat(logbuf, tmpbuf);	
			}
		}
	}
	for ( i= 0; i < dstcnt; i++) {
		isfound = FALSE;
		sprintf(dstbuf, "%s/%s", dstPath, dstflist[i]);
		for(j=0; j<srccnt; j++) {
			if (!strcmp(dstflist[i], srcflist[j])) { //파일 이름 비교
				isfound = TRUE; //같은 이름 찾은 경우 빠져나감
				break;
			}	
		}
		if ((isfound == FALSE) && mOption) { //dst에 있는데 src에 없는 파일인 경우 => m옵션 설정시에 삭제 해줌
			sprintf(tmpbuf, "\t%s delete\n", dstflist[i]);	//삭제된 파일 정보 로그에 쓰기 위해
			strcat(logbuf, tmpbuf);	 //로그 작성 버퍼에 추가
			if (stat(dstbuf, &statbuf) < 0) { //파일 정보 가져오기
					fprintf(stderr, "stat error for %s\n", dstbuf);
					return FALSE;
			}
			if (S_ISDIR(statbuf.st_mode)) { //디렉토리인 경우 디렉토리 내의 파일 모두 삭제
				if (!rmvDir(dstbuf)) { 
					fprintf(stderr, "rmvDir error for %s\n", dstflist[i]);
					return FALSE;
				}
			}
			if (remove(dstbuf) < 0) { //해당 파일 자체 삭제
				fprintf(stderr, "remove error for %s\n", dstbuf);
				return FALSE;
			}
		}
	}

	//백업파일들 삭제


	is_finished = TRUE; //동기화 완료

	is_logchanged = TRUE;	
	time(&cur_time);
	ctime_r(&cur_time, timebuf);
	timebuf[19] = 0; //요일 월 일 시간 까지만 출력할것임
	sprintf(filebuf, "[%s] %s\n", timebuf, usrCommand);
	if ((fp = fopen(logFile, "a")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", logFile);
		return FALSE;
	}
	if (fputs(filebuf, fp) < 0) { //로그 파일 작성
		fprintf(stderr, "write error\n");
		return FALSE;
	}
	if(strlen(logbuf) > 0) {
		if (fputs(logbuf, fp) < 0) { //로그 파일 작성
		fprintf(stderr, "write error\n");
		return FALSE;
		}
	}
	fclose(fp);
	return TRUE;
}

int syncFile() { //일반 파일을 동기화
	is_changed = TRUE;
	is_started = TRUE;
	char pathbuf[BUFLEN], newName[BUFLEN], timebuf[BUFLEN], logbuf[BUFLEN], tmpbuf[BUFLEN], rmvbuf[BUFLEN], dstbuf[BUFLEN], buf[BUFLEN];
	char cpcommand[BUFLEN];
	long fsize;
	FILE *fp;
	struct stat statbuf;
	struct stat statbuf2;
	struct dirent **filelist;
	time_t cur_time;
	int i, cnt;

	memset(logbuf, 0, BUFLEN);
	memset(rmvbuf, 0, BUFLEN);
	if (stat(srcPath, &statbuf) < 0) { //src파일정보 가져오기
		fprintf(stderr, "stat error for %s\n",srcPath);
		return FALSE;
	}

	if (!isExist(dstPath, srcName, pathbuf)) { //dst에 src 없는 경우 -> dst에 복사
		sprintf(newName, "%s/%s", dstPath, srcName); //복사 될 이름 (dst아래 src파일이름)
		mySync(newName, srcPath, FALSE); //dst안에 src 복사

		//로그파일에 쓸 내용 저장
		sprintf(tmpbuf, "\t%s %ldbytes\n", srcName, statbuf.st_size);	
		strcat(logbuf, tmpbuf);
	}
	else { //dst에 src가 있는 경우 -> 동기화 해야함
		if (stat(pathbuf, &statbuf2) < 0) { //src파일정보 가져오기
			fprintf(stderr, "stat error for %s\n", pathbuf);
			return FALSE;
		}
		//src와 dst의 파일이 다른지 비교
		if(compareFile(srcPath, pathbuf)) { //파일이 달라서(수정) 동기화 하는 경우
			mySync(pathbuf, srcPath, TRUE);
			
			//로그에 작성할 내용 저장
			sprintf(tmpbuf, "\t%s %ldbytes\n", srcName, statbuf.st_size);	
			strcat(logbuf, tmpbuf);
		}
	}
	if (mOption) { //m옵션 사용된 경우, src 파일 제외 dst 내의 파일 모두 삭제
		chdir(dstPath);
		if ((cnt = scandir(".", &filelist, except_tmp_file, NULL)) == -1) { //모든 파일 가져옴
			fprintf(stderr, "scandir error for %s\n", dstPath);
			return FALSE;
		}
		for (i = 0; i < cnt; i++) {
			sprintf(buf, "%s", filelist[i]->d_name); //파일 이름 저장
			if(strstr(buf, srcName) !=NULL) //src파일은 삭제 안함
				continue;
			if (stat(buf, &statbuf) < 0) {
				fprintf(stderr, "saved dir info stat error for %s\n", buf);
				return FALSE;
			}
			if ((statbuf.st_mode & S_IFMT) == S_IFREG) { //일반 파일인 경우 바로 저장
				sprintf(tmpbuf, "\t%s delete\n", buf);	//삭제된 파일 정보 로그에 쓰기 위해
				strcat(rmvbuf, tmpbuf);	 //로그 작성 버퍼에 추가
				if (remove(buf) < 0) {
					fprintf(stderr, "remove error for %s\n", buf);
					return FALSE;
				}
			}
			else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { //디렉토리인 경우
				sprintf(tmpbuf, "\t%s delete\n", buf);	//삭제된 파일 정보 로그에 쓰기 위해
				strcat(rmvbuf, tmpbuf);	 //로그 작성 버퍼에 추가
				if (!rmvDir(buf)) {
					fprintf(stderr, "rmvDir error for %s\n", buf);
					return FALSE;
				}
				if (remove(buf) < 0) {
					fprintf(stderr, "remove error for %s\n", buf);
					return FALSE;
				}
			}
		}
		//메모리 해제 후 파일개수 리턴
		for (i = 0; i < cnt; i++)
			free(filelist[i]);
		free(filelist);
		chdir(workPath);
	}
	

	is_finished = TRUE; //동기화 완료
	is_logchanged = TRUE;
	time(&cur_time);
	ctime_r(&cur_time, timebuf);
	timebuf[19] = 0; //요일 월 일 시간 까지만 출력할것임
	sprintf(tmpbuf, "[%s] %s\n", timebuf, usrCommand);	

	if ((fp = fopen(logFile, "a")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", logFile);
		return FALSE;
	}
	if (fputs(tmpbuf, fp) < 0) { //로그 파일 작성
		fprintf(stderr, "write error\n");
		return FALSE;
	}
	if(strlen(logbuf) > 0) {
		if (fputs(logbuf, fp) < 0) { //로그 파일 작성
			fprintf(stderr, "write error\n");
			return FALSE;
		}
	}
	if (mOption && (strlen(rmvbuf) > 0)) {
		if (fputs(rmvbuf, fp) < 0) { //로그 파일 작성
			fprintf(stderr, "write error\n");
			return FALSE;
		}
	}
	fclose(fp);
	
	is_finished = TRUE;
	return TRUE;
}

int isExist(char *dirName, char *fname, char *pathname) {
	//dirName 디렉토리에 fname파일 존재시 true(1)리턴, 없으면 false(0)리턴, 파일경로 pathname에 저장
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmpname[BUFLEN], dname[BUFLEN];
	char nxtdir[BUFLEN];
	int result = FALSE;

	if ((dp = opendir(dirName)) == NULL) {
		fprintf(stderr, "opendir error for %s\n", dirName);
		return FALSE;
	}
	while ((dirp = readdir(dp)) != NULL) { //모든 파일을 읽어서 찾는 파일인지 비교
		if (!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmpname, "%s/%s", dirName, dirp->d_name);
		if (stat(tmpname, &statbuf) < 0) {
			fprintf(stderr, "stat error for %s\n", tmpname);
			return FALSE;
		}

		if ((statbuf.st_mode & S_IFMT) == S_IFREG) { //일반파일인 경우
			strcpy(dname, dirp->d_name);
			if (!strcmp(fname, dname)) {  //delimeter 제거하고 이름 비교하기
				strcpy(pathname, tmpname); //내가찾는 파일이면 경로까지 합친 이름 저장
				return TRUE;
			}
		}
		else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { //디렉토리인 경우
				if (!strcmp(dirp->d_name, fname)) { //내가 찾는 파일이름과 동일한경우
					strcpy(pathname, tmpname); //해당 파일의 절대경로를 pathname에저장해 리턴
					return TRUE;
				}
				else { //내가 찾는 이름이 아닌경우
					strcpy(nxtdir, tmpname); //해당 디렉토리의 경로 저장
					result = isExist(nxtdir, fname, pathname); //해당 디렉토리 하위를 탐색하기위해서 재귀호출
					if (result)
						return TRUE;
				}
			}
	}
	if (result)
		return TRUE;
	return FALSE;
}

char *rmvpath(char *str) { //앞의 경로는 제외하고 이름만 뽑아내는 함수
	char *start;
	char tmp[BUFLEN];

	memset(tmp, (char)0, BUFLEN);
	strcpy(tmp, str);

	start = tmp + strlen(tmp) - 1;
	while (*start != '/') {
		--start;
	}
	if (start == str) //경로가 없는 경우, 원본 그대로 리턴해줌
		return str;
	else {
		str = start + 1;
		return str;
	}
}

static void cancel_sync_handler(int signo) {
	struct stat statbuf;
	int isDir = FALSE;
	char cpcommand[BUFLEN];

	if (stat(srcPath, &statbuf) < 0) { //src파일 정보 가져오기
		fprintf(stderr, "stat error for %s\n", srcPath);
		exit(1);
	}
	if ((statbuf.st_mode & S_IFMT) == S_IFDIR) {
		isDir = TRUE;
	}
	if (is_changed == TRUE) { //파일 변경이 시작된 경우
		if (is_started || is_finished) { //이미 시작되었으면 파일 동기화 전으로 되돌림
			if (isDir) { //SRC가 디렉토리인 경우 - 디렉토리 내 파일 삭제 후 디렉토리 삭제
				if (!rmvDir(srcPath)) { //src내의 내용 삭제
					fprintf(stderr, "rmvDir error for %s\n", srcPath);
					return ;
				}
				if (rmdir(srcPath) < 0) { //src파일 자체 삭제
					fprintf(stderr, "remove error for %s\n", srcPath);
					return ;
				}
			}
			else {
				if (remove(srcPath) < 0) { //src파일 자체 삭제
					fprintf(stderr, "remove error for %s\n", srcPath);
					return ;
				}
			}
			
			if (rename(srcBackup, srcPath) < 0) { //src파일 되돌리기
				fprintf(stderr, "rename error\n");
				exit(1);
			}

			if (!rmvDir(dstPath)) { //dst 내의 내용 삭제
				fprintf(stderr, "rmvDir error for %s\n", dstPath);
				return ;
			}
			if (rmdir(dstPath) < 0) { //dst파일 자체 삭제
				fprintf(stderr, "remove error for %s\n", dstPath);
				return ;
			}
			if (rename(dstBackup, dstPath) < 0) { //dst파일 되돌리기
				fprintf(stderr, "rename error\n");
				exit(1);
			}

			if (!is_logchanged) { //로그 파일도 변경 된 경우
				//로그 파일도 동기화 전으로 되돌림
				if (remove(logFile) < 0) { //로그파일 삭제
					fprintf(stderr, "remove error for %s\n", logFile);
					return ;
				}
				if (rename(logBackup, logFile) < 0) { //로그 파일 되돌리기
					fprintf(stderr, "rename error\n");
					exit(1);
				}
			}
		}
	}
	else {
		//아직 변경 안된 경우 => 백업 파일 삭제 후 종료
		if (isDir) { //SRC가 디렉토리인 경우
			if (!rmvDir(srcBackup)) { //src백업 내의 내용 삭제
				fprintf(stderr, "rmvDir error for %s\n", srcBackup);
				return;
			}
			if (rmdir(srcBackup) < 0) { //백업src파일 자체 삭제
				fprintf(stderr, "remove error for %s\n", srcBackup);
				return;
			}
		}
		else {
			if (remove(srcBackup) < 0) { //백업src파일 자체 삭제
				fprintf(stderr, "remove error for %s\n", srcBackup);
				return;
			}
		}
			
		if (remove(logBackup) < 0) { //백업로그파일 삭제
			fprintf(stderr, "remove error for %s\n", srcBackup);
			return ;
		}
		if (!rmvDir(dstBackup)) { //dst백업 내의 내용 삭제
			fprintf(stderr, "rmvDir error for %s\n", dstBackup);
			return ;
		}
		if (rmdir(dstBackup) < 0) { //dst파일 자체 삭제
			fprintf(stderr, "remove error for %s\n", dstBackup);
			return ;
		}
	}
	//파일 되돌리기 완료 or 
	exit(0);
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

void printUsage() { //프로그램 사용법 출력 함수
	printf("Uage : ssu_rsync [OPTION] <SRC> <DST>\n");
	printf("Option : \n");
	printf(" -r 		SRC가 디렉토리인 경우 서브 디렉토리 내의 파일도 함께 동기화\n");
	printf(" -t			필요한 대상들을 묶어 한번에 동기화 작업 수행\n");
	printf(" -m			DST 디렉토리에 SRC파일(또는 디렉토리)이 없는 경우 해당 파일을 DST내에서 삭제\n");
}