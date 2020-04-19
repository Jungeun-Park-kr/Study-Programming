#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ssu_score.h"
#include "blank.h"

extern struct ssu_scoreTable score_table[QNUM];
extern char id_table[SNUM][10];

struct ssu_scoreTable score_table[QNUM];
struct ssu_scoreTable score_mod[QNUM]; //수정할 문제정보를 저장할 구조체배열
char id_table[SNUM][10];

char stuDir[BUFLEN];
char ansDir[BUFLEN];
char errorDir[BUFLEN];
char threadFiles[ARGNUM][FILELEN];
char iIDs[ARGNUM][FILELEN]; //i옵션 사용할 학번 저장

int eOption = false;
int tOption = false;
int mOption = false; 
int iOption = false;

void ssu_score(int argc, char *argv[])
{
	char saved_path[BUFLEN];
	int i;

	for(i = 0; i < argc; i++){
		if(!strcmp(argv[i], "-h")){
			print_usage();
			return;
		}
	}

	memset(saved_path, 0, BUFLEN);
	if(argc >= 3 && strcmp(argv[1], "-i") != 0){
		strcpy(stuDir, argv[1]);
		strcpy(ansDir, argv[2]);
	}

	if(!check_option(argc, argv))
		exit(1);

	if(!eOption && !tOption && !mOption && iOption){
		do_iOption(iIDs);
		return;
	}

	getcwd(saved_path, BUFLEN);

	if(chdir(stuDir) < 0){
		fprintf(stderr, "%s doesn't exist\n", stuDir);
		return;
	}
	getcwd(stuDir, BUFLEN);

	chdir(saved_path);
	if(chdir(ansDir) < 0){
		fprintf(stderr, "%s doesn't exist\n", ansDir);
		return;
	}
	getcwd(ansDir, BUFLEN);

	chdir(saved_path);

	set_scoreTable(ansDir);
	set_idTable(stuDir);

	printf("grading student's test papers..\n");
	score_students();

	if(iOption)
		do_iOption(iIDs);

	return;
}

int check_option(int argc, char *argv[])
{
	int i, j;
	int c;

	while((c = getopt(argc, argv, "e:thmi")) != -1)
	{
		switch(c){
			case 'e':
				eOption = true;
				strcpy(errorDir, optarg);

				if(access(errorDir, F_OK) < 0)
					mkdir(errorDir, 0755);
				else{
					rmdirs(errorDir);
					mkdir(errorDir, 0755);
				}
				break;
			case 't':
				tOption = true;
				i = optind;
				j = 0;

				while(i < argc && argv[i][0] != '-'){

					if(j >= ARGNUM)
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else
						strcpy(threadFiles[j], argv[i]);
					i++; 
					j++;
				}
				break;
			case 'm':
				mOption = true;
				break;
			case 'i':
				iOption = true;
				i = optind;
				j = 0;

				while(i < argc && argv[i][0] != '-'){

					if(j >= ARGNUM)
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else
						strcpy(iIDs[j], argv[i]);
					i++; 
					j++;
				}
				break;
			case '?':
				printf("Unkown option %c\n", optopt);
				return false;
		}
	}

	return true;
}


void do_iOption(char (*ids)[FILELEN])
{
	FILE *fp;
	char tmp[BUFLEN];
	char tmp2[BUFLEN];
	char questions[QNUM][BUFLEN];
	char wrong_quest[BUFLEN]; //틀린 문제번호들을 저장할 문자열
	char check_score[BUFLEN]; //점수를 읽어 저장하는 문자열
	float score;
	int i;
	int size;
	char *p, *saved;

	if((fp = fopen("score.csv", "r")) == NULL){
		//채점결과 파일을 열수 없는경우, 에러메시지 출력 후 종료
		fprintf(stderr, "file open error for score.csv\n");
		return;
	}
        	
	fscanf(fp, "%s\n", tmp); //문제 저장을 위해 첫 줄 읽어 tmp에 저장
	i=0;
	p = strtok(tmp, ","); //첫 글자 검색해 저장
	strcpy(questions[i++], p); //해당 문제 저장
	while ((p = strtok(NULL, ",")) != NULL) {
		//한 번 strtok()에 tmp를 사용했으므로, 이후는 NULL로 반복문 가능
		strcpy(questions[i++], p); //문제를 읽어서 questions[]배열에 저장
	}
	size = i - 1; //총 문제 개수 저장 (-1한 이유는 sum을 제외하기 위함)
	strcpy(questions[size], ""); //sum 제거하기 위해 마지막 내용 없앰

	
	while(fscanf(fp, "%s\n", tmp) != EOF)
	{	//학생들의 정보를 읽어오기 위한 반복문
		p = strtok(tmp, ","); //첫 번째 셀은 학번 정보가 저장되어있음
		if(!is_exist(ids, tmp)) { //-i옵션 사용시 가변인자로 입력받은 학번인지 검사하는 함수
			continue; //아닐경우 다음 학번 검색
		}
		printf("%s's wrong answer : \n", tmp);
		memset(wrong_quest, 0, sizeof(char)*BUFLEN); //학생이 틀린 문제 저장할 배열 초기화
		i=-1; //문제이름을 가져올 인덱스
		while((p = strtok(NULL, ",")) != NULL) {
			//문제 점수를 하나 읽어오기
			saved = p;
			strcpy(check_score, saved);
			score = atof(saved);
			i++; //인덱스 증가
			if (score != 0.00) {
				//0점이 아닐경우 다음 문제 검사
			    continue;
			}
			//0점인 문제를 발견한 경우
			sprintf(tmp2, "%s, ", questions[i]); //해당 인덱스를 가진 문제이름을 tmp2에 저장 
			strcat(wrong_quest, tmp2); //해당 문제를 학생의 wrong_quest[]배열에 추가
		}
		//학생이 틀린 문제 출력 후 마지막 문자의 ", "를 제거하기 위한 코드
		wrong_quest[strlen(wrong_quest)-2]='\0'; //','와 ' '를 제거하기 위해 널문자 대임
		//그러나 널문자를 대입하면 마지막 문자까지 같이 사라져버림
		if((p = strpbrk(tmp2, "c")) != NULL) {
			//마지막으로 읽었던 문제 이름에 "c"가 포함되는지 확인(프로그램 문제인지)
		    wrong_quest[strlen(wrong_quest)-2] = '.'; //제거된 문자 추가
		      wrong_quest[strlen(wrong_quest)-1] = 'c'; //제거된 문자 추가
		      wrong_quest[strlen(wrong_quest)] = '\0'; //문자열의 끝에 널문자 추가
		}
		else if((p = strpbrk(tmp2, "t")) != NULL) {
			//마지막으로 읽었던 문제 이름에 "t"가 포함되는지 확인(빈칸 문제인지)
		    wrong_quest[strlen(wrong_quest)-2] = 'x'; //제거된 문자 추가
		      wrong_quest[strlen(wrong_quest)-1] = 't'; //제거된 문자 추가
		      wrong_quest[strlen(wrong_quest)] = '\0'; //문자열의 끝에 널문자 추가
		}
		printf("%s\n", wrong_quest); //해당 학생의 틀린 문제 출력 (완성된 wrong_quest자열 출력)
	}
	fclose(fp);
}

int is_exist(char (*src)[FILELEN], char *target)
{
	int i = 0;

	while(1)
	{
		if(i >= ARGNUM)
			return false;
		else if(!strcmp(src[i], ""))
			return false;
		else if(!strcmp(src[i++], target))
			return true;
	}
	return false;
}

void set_scoreTable(char *ansDir)
{
	char filename[FILELEN];

	sprintf(filename, "%s", "./score_table.csv"); //파일 생성 위치 변경(현재 실행위치)

	if(access(filename, F_OK) == 0) {
		//점수 테이블 파일(score_table.csv)이 존재하는 경우
		if (mOption) //m옵션이 사용된 경우
			modify_scoreTable(filename); //문제 점수 변경 함수 호출
		read_scoreTable(filename); //m옵션 유무와 상관없이 점수테이블 읽기함수 호출(읽어온 정보를 현재 프로그램 내 구조체에 저장)
	}
	else{
		make_scoreTable(ansDir);
		write_scoreTable(filename);
	}
}

int get_qnametype(char *qname) {
    ////파라미터로 받은 문제이름 검색해서 어떤 파일타입인지 리턴
	//파라미터 : 사용자가 입력한 수정할 문제 이름
	//리턴값 : 성공시 입력받은 문제의 파일타입(TEXTFILE(3) or CFILE(4)), 실패시 false
    FILE *fp;
    char qtmp[BUFLEN];
    char stmp[BUFLEN];
    int filetype;
    char *p;

    if((fp = fopen("score_table.csv", "r")) == NULL) {	//점수 테이블 파일 오픈 
		//실패시 에러메시지 출력 후 false 리턴
        fprintf(stderr, "file open error for score_table.csv\n");
        return false;
    }

    while(fscanf(fp, "%[^,],%s\n", qtmp, stmp) != EOF) { //파일의 끝까지 내용을 한 줄씩(한 문제씩)읽는 반복문
		//파일의 한줄을 읽어 문제이름은 qtmp, 점수는 stmp에 저장
		if ((p = strstr(qtmp, qname)) != NULL) {
			//파라미터로 전달받은(사용자가 입력한 문제이름)과 동일한 문제를 찾은경우
			filetype = get_file_type(qtmp); //get_file_type()호출하여 얻은 값 filetype 변수에 저장
			return filetype; //filetype 리턴
		}
    }
    return false; //일치하는 문제가 없는 경우 false 리턴
}
double get_score(char *qname) {
	//파라미터 : 사용자로부터 입력받은 변경할 문제의 이름
    //리턴값 : 성공시 점수 테이블 파일에 있는 파라미터 문제의 현재 점수, 실패시 false(0)
    FILE *fp;
    char qtmp[BUFLEN];
    char stmp[BUFLEN];
    double score;
    char *p;

    if((fp = fopen("score_table.csv", "r")) == NULL) {
		//점수 테이블 파일 열기
        fprintf(stderr, "file open error for score_table.csv\n");
        return false;
    }

    while(fscanf(fp, "%[^,],%s\n", qtmp, stmp) != EOF) {
		//한 줄씩(한 문제씩)읽어 입력받은 문제에 해당하는 문제가 있는지 확인하여 점수를 리턴하는 반복문
        if((p = strstr(qtmp, qname)) != NULL) {
			//동일한 문제이름을 찾은 경우
	       score = atof(stmp); //그 문제의 현재점수를 실수화하여 score변수에 저장
	       return score; //score 리턴
        }
    }
	//해당하는 문제와 동일한 문제를 못찾은 경우 false 리턴
    return false;    
    
}

int modify_scoreTable(char *path)
{ 
	//파라미터 : 점수 테이블파일(score_table.csv)의 경로
	//리턴 값 : 성공시 true, 실패시 false
    FILE *fp;
    FILE *fp2;
    char tmp[FILELEN];
    char tmp2[FILELEN];
    char qtmp[FILELEN];
    double stmp;
    double scur;
    int num;
    int i=0;
    int filetype;
    char *p;
    int is_found=false;
   
    if((fp = fopen(path, "r")) == NULL) {
		//점수 테이블 파일을 읽을 파일 포인터
        fprintf(stderr, "file open error for %s\n", path);
        return false;
    }
    
    if((fp2 = fopen(path, "r+")) == NULL) {
		//점수 테이블 파일을 작성할 파일 포인터
        fprintf(stderr, "file open error for %s\n", path);
        return false;
    }


    while(1) {
		//사용자가 문제 이름에 "no"를 입력할 때 까지 문제 이름을 입력받은 후 수정할 점수를 입력받아 scroe_mod 배열에 저장하는 반복문

		printf("Input question's number to modify >> ");
		scanf("%s", qtmp); //변경할 문제 번호 입력
		if (!strcmp(qtmp, "no")) //no이면 반복 종료
			break;
		scur = get_score(qtmp); //변경할 문제의 현재 점수를 리턴하는 함수 호출하여 scur에 저장
		if (scur == 0) { //사용자가 입력한 문제가 점수테이블 파일에 없는 경우
			printf("%s question not found\n", path); 
			return false; //return false
		}
        printf("Current score : %.2lf\n",scur); //scur에 저장된 현재(수정전)점수 출력
        printf("New score : ");
        scanf("%lf", &stmp); //사용자로부터 변경할 점수 입력받아 stmp에 저장
        filetype = get_qnametype(qtmp); //입력받은 문제의 파일 종류를 리턴하는 함수 호출해 filetype에 저장 
        if(filetype == TEXTFILE) //.txt파일인 경우 
	    strcat(qtmp,".txt"); //완전한 문제이름으로 만들기
        else if(filetype == CFILE) //.c파일인 경우
	    strcat(qtmp, ".c");	  //완전한 문제이름으로 만들기

        strcpy(score_mod[i].qname, qtmp); //수정될 문제의 이름 저장
        score_mod[i].score = stmp; //수정될 점수 저장
        i++; //수정될 문제의 개수를 세기 위한 인덱스 증가
        
    }
    num = i; //수정될 문제의 개수 num에 저장
   
    while(fscanf(fp, "%s\n", tmp) != EOF) { //변경된 점수를 점수 테이블 파일에 작성하는 반복문
		//점수 테이블 파일의 한 줄 읽어오기
		strcpy(tmp2, tmp); //읽어온 내용 tmp2에 복사
		is_found = false; //수정되어야 할 문제가 있는지 확인하는 변수 false로 초기화
		p = strtok(tmp2, ","); //문제이름 읽어 p가 가리키게 함
		for (i = 0; i < num; i++) {
			//수정될 문제와 동일한 이름인지 확인하는 반복문
			if (strstr(score_mod[i].qname, p) != NULL) {
				//수정될 문제와 동일한 문제일 경우
				is_found = true; //is_found변수에 true 대입
				fprintf(fp2, "%s,%.2f\n", score_mod[i].qname, score_mod[i].score);
				//해당하는 파일의 위치에 fp2로 변경될 점수로 수정
			}
		}
		if (!is_found) //수정될 문제가 아닌경우
			fprintf(fp2, "%s\n", tmp); //fp2의 포인터 이동을 위해 한줄 읽어 tmp에 저장(해당 tmp는 의미없음)

	}
	fclose(fp);
	fclose(fp2);
	return true; //성공적으로 파일을 수정한 경우 true 리턴
}
void read_scoreTable(char *path)
{
	FILE *fp;
	char qname[FILELEN];
	char score[BUFLEN];
	int idx = 0;

	if((fp = fopen(path, "r")) == NULL){
		fprintf(stderr, "file open error for %s\n", path);
		return ;
	}

	while(fscanf(fp, "%[^,],%s\n", qname, score) != EOF){
		strcpy(score_table[idx].qname, qname);
		score_table[idx++].score = atof(score);
	}

	fclose(fp);
}

void make_scoreTable(char *ansDir)
{
	int type, num;
	double score, bscore, pscore;
	struct dirent *dirp;
	DIR *dp;
	char tmp[BUFLEN];
	int idx = 0;
	int i;

	num = get_create_type();

	if(num == 1)
	{
		printf("Input value of blank question : ");
		scanf("%lf", &bscore);
		printf("Input value of program question : ");
		scanf("%lf", &pscore);
	}

	if((dp = opendir(ansDir)) == NULL){
		fprintf(stderr, "open dir error for %s\n", ansDir);
		return;
	}	

	while((dirp = readdir(dp)) != NULL)
	{   //ansDir디렉토리 내에 바로 정답 파일들이 존재하기 때문에, 또 디렉토리를 검색할 필요가 없음
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		if((type = get_file_type(dirp->d_name)) < 0)
			continue;

		strcpy(score_table[idx++].qname, dirp->d_name);
	}

	closedir(dp);
	sort_scoreTable(idx);

	for(i = 0; i < idx; i++)
	{
		type = get_file_type(score_table[i].qname);

		if(num == 1)
		{
			if(type == TEXTFILE)
				score = bscore;
			else if(type == CFILE)
				score = pscore;
		}
		else if(num == 2)
		{
			printf("Input of %s: ", score_table[i].qname);
			scanf("%lf", &score);
		}

		score_table[i].score = score;
	}
}

void write_scoreTable(char *filename)
{
	int fd;
	char tmp[BUFLEN];
	int i;
	int num = sizeof(score_table) / sizeof(score_table[0]);

	if ((fd = creat(filename, 0666)) < 0) {
		fprintf(stderr, "creat error for %s\n", filename);
		return;
	}

	for(i = 0; i < num; i++)
	{
		if(score_table[i].score == 0)
			break;

		sprintf(tmp, "%s,%.2f\n", score_table[i].qname, score_table[i].score);
		write(fd, tmp, strlen(tmp));
	}

	close(fd);
}


void set_idTable(char *stuDir)
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	char tmp[BUFLEN];
	int num = 0;

	if((dp = opendir(stuDir)) == NULL){
		fprintf(stderr, "opendir error for %s\n", stuDir);
		exit(1);
	}

	while((dirp = readdir(dp)) != NULL){
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmp, "%s/%s", stuDir, dirp->d_name);
		stat(tmp, &statbuf);

		if(S_ISDIR(statbuf.st_mode))
			strcpy(id_table[num++], dirp->d_name);
		else
			continue;
	}

	sort_idTable(num);
}

void sort_idTable(int size)
{
	int i, j;
	char tmp[10];

	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 -i; j++){
			if(strcmp(id_table[j], id_table[j+1]) > 0){
				strcpy(tmp, id_table[j]);
				strcpy(id_table[j], id_table[j+1]);
				strcpy(id_table[j+1], tmp);
			}
		}
	}
}

void sort_scoreTable(int size)
{
	int i, j;
	struct ssu_scoreTable tmp;
	int num1_1, num1_2;
	int num2_1, num2_2;

	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 - i; j++){

			get_qname_number(score_table[j].qname, &num1_1, &num1_2);
			get_qname_number(score_table[j+1].qname, &num2_1, &num2_2);


			if((num1_1 > num2_1) || ((num1_1 == num2_1) && (num1_2 > num2_2))){

				memcpy(&tmp, &score_table[j], sizeof(score_table[0]));
				memcpy(&score_table[j], &score_table[j+1], sizeof(score_table[0]));
				memcpy(&score_table[j+1], &tmp, sizeof(score_table[0]));
			}
		}
	}
}

void get_qname_number(char *qname, int *num1, int *num2)
{
	char *p;
	char dup[FILELEN];

	strncpy(dup, qname, strlen(qname));
	*num1 = atoi(strtok(dup, "-."));
	
	p = strtok(NULL, "-.");
	if(p == NULL)
		*num2 = 0;
	else
		*num2 = atoi(p);
}

int get_create_type()
{
	int num;

	while(1)
	{
		printf("score_table.csv file doesn't exist in TREUDIR!\n");
		printf("1. input blank question and program question's score. ex) 0.5 1\n");
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n");
		printf("select type >> ");
		scanf("%d", &num);

		if(num != 1 && num != 2)
			printf("not correct number!\n");
		else
			break;
	}

	return num;
}

void score_students()
{
	double score = 0;
	int num;
	int fd;
	char tmp[BUFLEN];
	int size = sizeof(id_table) / sizeof(id_table[0]);

	if((fd = creat("score.csv", 0666)) < 0){
		fprintf(stderr, "creat error for score.csv");
		return;
	}
	write_first_row(fd);

	for(num = 0; num < size; num++)
	{
		if(!strcmp(id_table[num], ""))
			break;

		sprintf(tmp, "%s,", id_table[num]);
		write(fd, tmp, strlen(tmp)); 

		score += score_student(fd, id_table[num]);
	}

        printf("Total average : %.2f\n", score / num);

	close(fd);
}

double score_student(int fd, char *id)
{
	int type;
	double result;
	double score = 0;
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]);

	for(i = 0; i < size ; i++)
	{
		if(score_table[i].score == 0)
			break;

		sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname);

		if(access(tmp, F_OK) < 0)
			result = false;
		else
		{
			if((type = get_file_type(score_table[i].qname)) < 0)
				continue;
			
			if(type == TEXTFILE)
				result = score_blank(id, score_table[i].qname);
			else if(type == CFILE)
				result = score_program(id, score_table[i].qname);
		}

		if(result == false)
			write(fd, "0,", 2);
		else{
			if(result == true){
				score += score_table[i].score;
				sprintf(tmp, "%.2f,", score_table[i].score);
			}
			else if(result < 0){
				score = score + score_table[i].score + result;
				sprintf(tmp, "%.2f,", score_table[i].score + result);
			}
			write(fd, tmp, strlen(tmp));
		}
	}

	printf("%s is finished.. score : %.2f\n", id, score); 

	sprintf(tmp, "%.2f\n", score);
	write(fd, tmp, strlen(tmp));

	return score;
}

void write_first_row(int fd)
{
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]);

	write(fd, ",", 1);

	for(i = 0; i < size; i++){
		if(score_table[i].score == 0)
			break;
		
		sprintf(tmp, "%s,", score_table[i].qname);
		write(fd, tmp, strlen(tmp));
	}
	write(fd, "sum\n", 4);
}

char *get_answer(int fd, char *result)
{
	char c;
	int idx = 0;

	memset(result, 0, BUFLEN);
	while(read(fd, &c, 1) > 0)
	{
		if(c == ':')
			break;
		
		result[idx++] = c;
	}
	if(result[strlen(result) - 1] == '\n')
		result[strlen(result) - 1] = '\0';

	return result;
}

int score_blank(char *id, char *filename)
{
	char tokens[TOKEN_CNT][MINLEN];
	node *std_root = NULL, *ans_root = NULL;
	int idx, start;
	char tmp[BUFLEN];
	char s_answer[BUFLEN], a_answer[BUFLEN];
	char qname[FILELEN];
	int fd_std, fd_ans;
	int result = true;
	int has_semicolon = false;

	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));

	sprintf(tmp, "%s/%s/%s", stuDir, id, filename);
	fd_std = open(tmp, O_RDONLY);
	strcpy(s_answer, get_answer(fd_std, s_answer));

	if(!strcmp(s_answer, "")){
		close(fd_std);
		return false;
	}

	if(!check_brackets(s_answer)){
		close(fd_std);
		return false;
	}

	strcpy(s_answer, ltrim(rtrim(s_answer)));

	if(s_answer[strlen(s_answer) - 1] == ';'){
		has_semicolon = true;
		s_answer[strlen(s_answer) - 1] = '\0';
	}

	if(!make_tokens(s_answer, tokens)){
		close(fd_std);
		return false;
	}

	idx = 0;
	std_root = make_tree(std_root, tokens, &idx, 0);

	sprintf(tmp, "%s/%s", ansDir, filename);
	fd_ans = open(tmp, O_RDONLY);

	while(1)
	{
		ans_root = NULL;
		result = true;

		for(idx = 0; idx < TOKEN_CNT; idx++)
			memset(tokens[idx], 0, sizeof(tokens[idx]));

		strcpy(a_answer, get_answer(fd_ans, a_answer));

		if(!strcmp(a_answer, ""))
			break;

		strcpy(a_answer, ltrim(rtrim(a_answer)));

		if(has_semicolon == false){
			if(a_answer[strlen(a_answer) -1] == ';')
				continue;
		}

		else if(has_semicolon == true)
		{
			if(a_answer[strlen(a_answer) - 1] != ';')
				continue;
			else
				a_answer[strlen(a_answer) - 1] = '\0';
		}

		if(!make_tokens(a_answer, tokens))
			continue;

		idx = 0;
		ans_root = make_tree(ans_root, tokens, &idx, 0);

		compare_tree(std_root, ans_root, &result);

		if(result == true){
			close(fd_std);
			close(fd_ans);

			if(std_root != NULL)
				free_node(std_root);
			if(ans_root != NULL)
				free_node(ans_root);
			return true;

		}
	}
	
	close(fd_std);
	close(fd_ans);

	if(std_root != NULL)
		free_node(std_root);
	if(ans_root != NULL)
		free_node(ans_root);

	return false;
}

double score_program(char *id, char *filename)
{
	double compile;
	int result;

	compile = compile_program(id, filename);

	if(compile == ERROR || compile == false)
		return false;
	
	result = execute_program(id, filename);

	if(!result)
		return false;

	if(compile < 0)
		return compile;

	return true;
}

int is_thread(char *qname)
{
	int i;
	int size = sizeof(threadFiles) / sizeof(threadFiles[0]);

	for(i = 0; i < size; i++){
		if(!strcmp(threadFiles[i], qname))
			return true;
	}
	return false;
}

double compile_program(char *id, char *filename)
{
	int fd;
	char tmp_f[BUFLEN], tmp_e[BUFLEN];
	char command[BUFLEN];
	char qname[FILELEN];
	int isthread;
	off_t size;
	double result;

	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));
	
	isthread = is_thread(qname);
	sprintf(tmp_f, "%s/%s", ansDir, filename);
	sprintf(tmp_e, "%s/%s.exe", ansDir, qname);

	if(tOption && isthread)
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);

	sprintf(tmp_e, "%s/%s_error.txt", ansDir, qname);
	fd = creat(tmp_e, 0666);

	redirection(command, fd, STDERR);
	size = lseek(fd, 0, SEEK_END);
	close(fd);
	unlink(tmp_e);

	if(size > 0)
		return false;

	sprintf(tmp_f, "%s/%s/%s", stuDir, id, filename);
	sprintf(tmp_e, "%s/%s/%s.stdexe", stuDir, id, qname);

	if(tOption && isthread)
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);

	sprintf(tmp_f, "%s/%s/%s_error.txt", stuDir, id, qname);
	fd = creat(tmp_f, 0666);

	redirection(command, fd, STDERR);
	size = lseek(fd, 0, SEEK_END);
	close(fd);

	if(size > 0){
		if(eOption)
		{
			sprintf(tmp_e, "%s/%s", errorDir, id);
			if(access(tmp_e, F_OK) < 0)
				mkdir(tmp_e, 0755);

			sprintf(tmp_e, "%s/%s/%s_error.txt", errorDir, id, qname);
			rename(tmp_f, tmp_e);

			result = check_error_warning(tmp_e);
		}
		else{ 
			result = check_error_warning(tmp_f);
			unlink(tmp_f);
		}

		return result;
	}

	unlink(tmp_f);
	return true;
}

double check_error_warning(char *filename)
{
	FILE *fp;
	char tmp[BUFLEN];
	double warning = 0;

	if((fp = fopen(filename, "r")) == NULL){
		fprintf(stderr, "fopen error for %s\n", filename);
		return false;
	}

	while(fscanf(fp, "%s", tmp) > 0){
		if(!strcmp(tmp, "error:"))
			return ERROR;
		else if(!strcmp(tmp, "warning:"))
			warning += WARNING;
	}

	return warning;
}

int execute_program(char *id, char *filename)
{
	char std_fname[BUFLEN], ans_fname[BUFLEN];
	char tmp[BUFLEN];
	char qname[FILELEN];
	time_t start, end;
	pid_t pid;
	int fd;

	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));

	sprintf(ans_fname, "%s/%s.stdout", ansDir, qname);
	fd = creat(ans_fname, 0666);

	sprintf(tmp, "%s/%s.exe", ansDir, qname);
	redirection(tmp, fd, STDOUT);
	close(fd);

	sprintf(std_fname, "%s/%s/%s.stdout", stuDir, id, qname);
	fd = creat(std_fname, 0666);

	sprintf(tmp, "%s/%s/%s.stdexe &", stuDir, id, qname);

	start = time(NULL);
	redirection(tmp, fd, STDOUT);
	
	sprintf(tmp, "%s.stdexe", qname);
	while((pid = inBackground(tmp)) > 0){
		end = time(NULL);

		if(difftime(end, start) > OVER){
			kill(pid, SIGKILL);
			close(fd);
			return false;
		}
	}

	close(fd);

	return compare_resultfile(std_fname, ans_fname);
}

pid_t inBackground(char *name)
{
	pid_t pid;
	char command[64];
	char tmp[64];
	int fd;
	off_t size;
	
	memset(tmp, 0, sizeof(tmp));
	fd = open("background.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);

	sprintf(command, "ps | grep %s", name);
	redirection(command, fd, STDOUT);

	lseek(fd, 0, SEEK_SET);
	read(fd, tmp, sizeof(tmp));

	if(!strcmp(tmp, "")){
		unlink("background.txt");
		close(fd);
		return 0;
	}

	pid = atoi(strtok(tmp, " "));
	close(fd);

	unlink("background.txt");
	return pid;
}

int compare_resultfile(char *file1, char *file2)
{
	int fd1, fd2;
	char c1, c2;
	int len1, len2;

	fd1 = open(file1, O_RDONLY);
	fd2 = open(file2, O_RDONLY);

	while(1)
	{
		while((len1 = read(fd1, &c1, 1)) > 0){
			if(c1 == ' ') 
				continue;
			else 
				break;
		}
		while((len2 = read(fd2, &c2, 1)) > 0){
			if(c2 == ' ') 
				continue;
			else 
				break;
		}
		
		if(len1 == 0 && len2 == 0)
			break;

		to_lower_case(&c1);
		to_lower_case(&c2);

		if(c1 != c2){
			close(fd1);
			close(fd2);
			return false;
		}
	}
	close(fd1);
	close(fd2);
	return true;
}

void redirection(char *command, int new, int old)
{
	int saved;

	saved = dup(old);
	dup2(new, old);

	system(command);

	dup2(saved, old);
	close(saved);
}

int get_file_type(char *filename)
{
	char *extension = strrchr(filename, '.');

	if(!strcmp(extension, ".txt"))
		return TEXTFILE;
	else if (!strcmp(extension, ".c"))
		return CFILE;
	else
		return -1;
}

void rmdirs(const char *path)
{
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmp[50];
	char dtmp[50];
	
	if((dp = opendir(path)) == NULL)
		return;

	while((dirp = readdir(dp)) != NULL)
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		strcpy(dtmp, dirp->d_name); //strcpy()로 먼저 디렉토리 이름 저장 후, 아래 코드 실행
		sprintf(tmp, "%s/%s", path, dtmp); //이부분에서 warning 발생해서 코드 고침

		if(lstat(tmp, &statbuf) == -1)
			continue;

		if(S_ISDIR(statbuf.st_mode))
			rmdirs(tmp);
		else
			unlink(tmp);
	}

	closedir(dp);
	rmdir(path);
}

void to_lower_case(char *c)
{
	if(*c >= 'A' && *c <= 'Z')
		*c = *c + 32;
}

void print_usage()
{
	printf("Usage : ssu_score <STUDENTDIR> <TRUEDIR> [OPTION]\n");
	printf("Option : \n");
	printf(" -m		modify question's score\n");
	printf(" -e <DIRNAME>      print error on 'DIRNAME/ID/qname_error.txt' file \n");
	printf(" -t <QNAMES>       compile QNAME.C with -lpthread option\n");
	printf(" -i <IDS>		print ID's wrong questions\n");
	printf(" -h                print usage\n");
}
