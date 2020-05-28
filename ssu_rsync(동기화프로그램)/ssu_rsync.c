/*  파일 동기화 중 SIGINT발생시 어떻게 백업할건지 생각해야 함
	link()하면 기존 파일 변경되면 상대방도 변경되는듯.....
	근데 copy하면 또 다른 파일 되자나..
	애초에 link() 디렉토리에는 안먹힘 ㅜㅜㅜㅜ (왜이러지)
 */
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

FILE *logfp;

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

void ssu_rsync(int argc, char *argv[]) {
	char saved_path[BUFLEN];
	char cpcommand[BUFLEN];
	struct stat statbuf;

	//SIGINT시그널을 등록, cancel_sync_handler()함수 핸들러 등록
	if (signal(SIGINT, cancel_sync_handler) == SIG_ERR) {
		fprintf(stderr, "cannot handle SIGINT\n");
		exit(EXIT_FAILURE);
	}
	//명령어 복사
	sprintf(usrCommand, "ssu_rsync %s %s", argv[1], rtrim(argv[2]));
	
	getcwd(saved_path, BUFLEN);
	sprintf(logFile, "%s/ssu_rync_log", saved_path);
	if (access(logFile, F_OK)) { //로그파일 없으면 생성
		logfp = fopen(logFile, "w");
		fclose(logfp);
	}
	
	if (realpath(argv[1], srcPath) == NULL) { //src이름 절대경로로 저장
		fprintf(stderr, "realpath error for %s\n", argv[1]);
		exit(1);
	}
	strcpy(srcName, rmvpath(srcPath)); //src경로 제외 이름만 저장
	printf("srcName : %s\n", srcName);
	if (realpath(argv[2], dstPath) == NULL) {  //dst 이름  절대경로로 저장
		fprintf(stderr, "realpath error for %s\n", argv[2]);
		exit(1);
	}
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
	
	compareFile(0, argv[1], argv[2]);
	exit(0);
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
		fprintf(stderr, "stat error\n");
		return FALSE;
	}
	
	//동기화 하는 파일 못 열게 함
	if (fd = open(srcPath, O_RDWR) < 0) {
		fprintf(stderr, "open error for %s\n", srcPath);
		return FALSE;
	}
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
		if (!syncFile()) {
			fprintf(stderr, "syncFile error\n");
			return FALSE;
		}
		
	}
	else if ((statbuf.st_mode & S_IFMT) == S_IFDIR) { //src파일이 디렉토리인 경우
		if (!syncDir()) {
			fprintf(stderr, "syncDir error\n");
			return FALSE;
		}
	}
	lock.l_type = F_UNLCK; //동기화 후 락 해제
	fcntl(fd, F_SETLK, &lock);
	return TRUE;
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
		fprintf(stderr, "scandir error\n");
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
			deleteDir(buf); //재귀호출로 해당디렉토리 내의 파일 모두 삭제
			remove(buf); //그후 빈 디렉토리인 자기자신도 삭제
		}
	}
	return TRUE;
}

int compareFile(int isDir, char *file1, char *file2) {
	//두 파일을 비교해서 동일하면 0, 다르면 1 리턴
	struct stat statbuf1, statbuf2;
	struct tm tm1, tm2;
	if (stat(file1, &statbuf1) < 0) { //src파일정보 가져오기
		fprintf(stderr, "stat error\n");
		exit(1);
	}
	if (stat(file2, &statbuf2) < 0) { //src파일정보 가져오기
		fprintf(stderr, "stat error\n");
		exit(1);
	}
	tm1 = *localtime(&statbuf1.st_mtime);
	tm2 = *localtime(&statbuf2.st_mtime);
	if (memcmp(&tm1, &tm2, sizeof(struct tm))) { //수정시간 확인
		printf("수정시간이 다름\n");
		return TRUE;	//수정시간 다름
	}
	else
	{
		printf("수정시간 같음\n");
	}
	
	if (isDir) { //디렉토리인 경우 크기 비교
		if (getDirSize(file1) == getDirSize(file2)) {//사이즈 같으면 0 리턴
			printf("사이즈 같음\n");
			return FALSE;
		}
		else { //사이즈 다르면 1 리턴 
			printf("사이즈가 다름\n");
			return TRUE;
		}
	}
	else { //디렉토리 아닌경우
		if (memcmp(&statbuf1.st_size, &statbuf2.st_size, sizeof(struct stat))) { //사이즈 비교
				printf("사이즈 같음\n");
			return FALSE; //크기 같으면 0리턴
		}
		else {
			printf("사이즈가 다름\n");
			return TRUE; //크기 다른 경우 1 리턴	
		}
	}
}

int syncDir() { //디렉토리 파일을 동기화
	is_changed = TRUE;
	is_started = TRUE;



	is_finished = TRUE;
	return TRUE;
}

int syncFile() { //일반 파일을 동기화
	is_changed = TRUE;
	is_started = TRUE;
	char pathbuf[BUFLEN], newName[BUFLEN], timebuf[BUFLEN], filebuf[BUFLEN];
	char cpcommand[BUFLEN];
	long fsize;
	FILE *fp;
	//struct flock lock;
	struct stat statbuf;
	struct stat statbuf2;
	time_t cur_time;


	if (stat(srcPath, &statbuf) < 0) { //src파일정보 가져오기
		fprintf(stderr, "stat error\n");
		return FALSE;
	}

	// //동기화 하는 파일 못 열게 함
	// if (fd1 = open(srcPath, O_RDWR) < 0) {
	// 	fprintf(stderr, "open error for %s\n", srcPath);
	// 	return FALSE;
	// }
	// lock.l_type = F_WRLCK;
	// lock.l_whence = 0;
	// lock.l_start = 0L;
	// lock.l_len = 0L;
	// if (fcntl(fd1, F_SETLK, &lock) == -1) { //동기화 할 파일 락
	// 	fprintf(stderr, "fcntl error\n");
	// 	return FALSE;
	// }

	if (!isExist(dstPath, srcName, pathbuf)) { //dst에 src 없는 경우 -> dst에 복사
		//dst안에 src 복사(link 이용)
		printf("없는 파일 동기화\n");
		sprintf(newName, "%s/%s", dstPath, srcName); //복사 될 이름 (dst아래 src파일이름)
		/* link() 이용한 것 말고 cp 명령어로 파일 복사 해보려고 주석 처리
		if (link(srcPath, newName) == -1) { //얘는 없는 파일이니까 새 이름 지정
			fprintf(stderr, "link error for %s\n", srcPath);
			return FALSE;
		} */
		sprintf(cpcommand, "cp -r -p %s %s", srcPath, newName);
		system(cpcommand);

		//로그파일에 쓸 내용 저장
		time(&cur_time);
		ctime_r(&cur_time, timebuf);
		timebuf[19] = 0; //요일 월 일 시간 까지만 출력할것임
		sprintf(filebuf, "[%s] %s\n\t%s %ldbytes\n", timebuf, usrCommand, srcName, statbuf.st_size);	
	}
	else {	//dst에 src가 있는 경우 -> 동기화 해야함
		printf("파일 있는거 동기화\n");
		if (stat(pathbuf, &statbuf2) < 0) { //src파일정보 가져오기
			fprintf(stderr, "stat error\n");
			return FALSE;
		}
		//src와 dst의 파일이 다른지 비교
		int res = compareFile(FALSE, srcPath, pathbuf);
		if (!res) { //같은 경우 -> 동기화 필요 x
			is_finished = TRUE; //동기화 완료(필요 없는 동기화)
			printf("동기화 필요 없음\n");
			time(&cur_time);
			ctime_r(&cur_time, timebuf);
			timebuf[19] = 0; //요일 월 일 시간 까지만 출력할것임
			sprintf(filebuf, "[%s] %s\n", timebuf, usrCommand);
			
		}
		else { //파일이 달라서 동기화 하는 경우
			printf("동기화 필요함\n");
			if (remove(pathbuf) < 0) { //기존에 있던 dst의 파일 삭제
				fprintf(stderr, "remove error for %s\n", pathbuf);
				return FALSE;
			}
			sprintf(cpcommand, "cp -r -p %s %s", srcPath, pathbuf);
			system(cpcommand);
			/* cp명령어로 파일 복사로 변경했음
			if (link(srcPath, pathbuf) == -1) { //src 파일을 dst에 복사
				fprintf(stderr, "link error for %s\n", srcPath);
				return FALSE;
			} */
			is_finished = TRUE; //동기화 완료
			
			//로그에 작성할 내용 저장
			time(&cur_time);
			ctime_r(&cur_time, timebuf);
			timebuf[19] = 0; //요일 월 일 시간 까지만 출력할것임
			sprintf(filebuf, "[%s] %s\n\t%s %ldbytes\n", timebuf, usrCommand, srcName, statbuf.st_size);	
		}
	}
	is_logchanged = TRUE;
	
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
	

	if ((fp = fopen(logFile, "a")) == NULL) {
		fprintf(stderr, "fopen error for %s\n", logFile);
		return FALSE;
	}
	if (fputs(filebuf, fp) < 0) { //로그 파일 작성
		fprintf(stderr, "write error\n");
		return FALSE;
	}
	fclose(fp);
	// lock.l_type = F_UNLCK; //동기화 후 락 해제
	// fcntl(fd1, F_SETLK, &lock);
	
	


	
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
			fprintf(stderr, "stat error\n");
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
	if (is_changed == FALSE) { //아직 변경 안되었으므로 바로 종료
		exit(0);	
	}
	if (is_started || is_finished) { //이미 시작되었으면
		//dst 파일 동기화 전으로 되돌림
		//in here
		if (is_logchanged) { //로그 파일도 변경 된 경우
			exit(0);
		}
		//로그 파일도 동기화 전으로 되돌림
		//in here
		
		exit(0);
	}
}

void printUsage() {
	printf("Uage : ssu_rsync [OPTION] <SRC> <DST>\n");
	printf("Option : \n");
	printf(" -r 		SRC가 디렉토리인 경우 서브 디렉토리 내의 파일도 함께 동기화\n");
	printf(" -t			필요한 대상들을 묶어 한번에 동기화 작업 수행\n");
	printf(" -m			DST 디렉토리에 SRC파일(또는 디렉토리)이 없는 경우 해당 파일을 DST내에서 삭제\n");
}
int main(int argc, char *argv[]) {
	
	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);	

	ssu_rsync(argc, argv);

	gettimeofday(&end_time, NULL);

	end_time.tv_sec -= start_time.tv_sec;
	
	if (end_time.tv_usec < start_time.tv_usec) {
		end_time.tv_sec--;
		end_time.tv_usec += SECOND_TO_MICRO;
	}
	end_time.tv_usec -= start_time.tv_usec;
	printf("Runtime: %ld:%06ld(sec:usec)\n", end_time.tv_sec, end_time.tv_usec);
}
