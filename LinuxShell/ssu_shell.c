#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define TRUE 1
#define FALSE 0

int isPiped = FALSE;

char **tokenize(char *line);
int execute_pps(char **tokens);
int execute_ttop(char **tokens);
int execute_built_in(char **tokens);
int check_complete_process(int status); //비정상 프로세스 종료 함수
static void exit_ssushell_handler(int signo);

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize_pipe(char *line) {
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for (i = 0; i < strlen(line); i++)
	{
		char readChar = line[i];

		if (readChar == '|') {
			token[tokenIndex++] = '\0';
			if (tokenIndex != 0)
			{
				tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0;
			}
			token[tokenIndex] = readChar; //pipe 따로 넣기
			tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
			tokens[tokenNo++] = "|";
			tokenIndex = 0;
		}
		else if ( readChar == '\n') {
			token[tokenIndex] = '\0';
			if (tokenIndex != 0)
			{
				tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0;
			}
		}
		else
		{
			token[tokenIndex++] = readChar;
		}

	}

	free(token);
	tokens[tokenNo] = NULL; //마지막에 널문자!!
	return tokens;
}

char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for (i = 0; i < strlen(line); i++)
	{
		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t')
		{
			token[tokenIndex] = '\0';
			if (tokenIndex != 0)
			{
				tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0;
			}
		}
		else
		{
			// if (readChar == '|')
			// 	isPiped = TRUE;
			token[tokenIndex++] = readChar;
		}
	}

	free(token);
	tokens[tokenNo] = NULL; //마지막에 널문자!!
	return tokens;
}

int execute_pps(char **tokens) {
	return TRUE;
}

int execute_ttop(char **tokens) {
	return TRUE;
}

int execute_piped(char **tokens) { //모르겠음 다시해야함...
	int i, j, idx = 0;
	int pipe_fd[2];
	pid_t pid1, pid2;
	

	for (i = 0; tokens[i+1] != NULL; i++) {
		char **piped_tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));

		for (j = 0; tokens[j] != NULL; j++)
		{
			tokens[j] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
			if (tokens[j][0] == '|')
			{
				piped_tokens[j] = NULL;
			}
			else
			{
				piped_tokens[j] = tokens[j];
			}
		}

		if (pipe(pipe_fd) == -1) { //파이프 파일 생성
			fprintf(stderr, "pipe error\n");
			return FALSE;
		}
		pid1 = fork(); //명령어 수행할 자식 프로세스 생성 (읽기 전용)
		if(pid1 < 0) { //fork 실패한 경우
			fprintf(stderr, "fork error\n");
			return FALSE;
		}
		else if (pid1 == 0) { //생성된 읽기전용 자식 프로세스
			close(pipe_fd[0]); //읽기 파일 닫기
			dup2(pipe_fd[1], STDOUT_FILENO);
			close(pipe_fd[1]);
			printf("execvp시작 전\n");
			if (execvp(tokens[i], tokens) < 0)
			{ //execvp로 명령어 실행
				fprintf(stderr, "execvp error\n");
				exit(1);
			}
		}
		else { //부모 프로세스
			pid2 = fork(); //명령어 수행할 자식 프로세스 생성 (쓰기 전용)
			if (pid2 < 0) { //fork 실패한 경우
				fprintf(stderr, "fork error\n");
				return FALSE;
			}
			else if (pid2 == 0) { //생성된 쓰기 전용 자식 프로세스
				close (pipe_fd[1]);
				dup2(pipe_fd[0], STDIN_FILENO);
				close(pipe_fd[0]);

				printf("execvp시작 전\n");
				if (execvp(tokens[i], tokens) < 0)
				{ //execvp로 명령어 실행
					fprintf(stderr, "execvp error\n");
					exit(1);
				}
			}
			else { //부모 프로세스

			}
		}
	}
}

int execute_built_in(char **tokens) {
	pid_t pid;
	int status, respid;
	int res = FALSE;

	pid = fork(); //부모 프로세스가 자식 프로세스 생성!

	if (pid < 0) { //fork()실패한 경우
		fprintf(stderr, "fork error\n"); 
		return FALSE;
	}
	else if (pid == 0) { //생성된 자식 프로세스
		//파이프 없는 일반 명령어인 경우
		//execvp()으로 명령어 실행
		printf("execvp시작 전\n");
		if (execvp(tokens[0], tokens) < 0) { //execvp로 명령어 실행
			fprintf(stderr, "execvp error\n");
			exit(1);
		}
		exit(0); //명령어 수행 끝난 후 프로세스 종료 시키기
	}
	else { 	//생성을 완료한 부모 프로세스는 제어 여기로 옴
		respid = wait(&status); //자식 프로세스의 종료 기다린 후 pid 얻기
		if (respid == pid) //종료된 프로세스가 자식 프로세스가 맞는 경우 return TRUE
			return TRUE;
	}
	
	if (res == FALSE) { //자식 프로세스가 정상종료 안된경우
		if (check_complete_process(status) == FALSE) //해당 자식 프로세스의 종료 상태를 확인
			kill(pid, SIGKILL); //해당 pid 종료시키기
		return FALSE;
	}
	return FALSE;
}

int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i, res;

	if (signal(SIGINT, exit_ssushell_handler) == SIG_ERR) { //SIGINT (Ctrl+C) 발생시 프로그램 종료할 exit_ssushell_handler 등록
		fprintf(stderr, "cannot handle SIGINT\n");
		exit(EXIT_FAILURE);
	}

	FILE* fp;
	if(argc == 2) { //배치모드인 경우 인자인 파일 이름으로 파일 존재여부 확인
		fp = fopen(argv[1],"r");
		if(fp < 0) { //파일 없으면 프로그램 종료
			printf("File doesn't exists.");
			return -1;
		}
	}

	
	while(1) {
		res = isPiped = FALSE; //변수 초기화

		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		if(argc == 2) { // batch mode (배치식 모드인 경우 - 파일 이름이 인자)
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;	//파일 내에 더이상 읽을 명령어가 없는 경우 break 후 종료
			}
			line[strlen(line) - 1] = '\0';
		} else { // interactive mode (대화식 모드인 경우 - 인자 없이 실행)
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
			if (strlen(line) == 0) //엔터만 입력한 경우 프롬프트 다시 출력
				continue;
			// printf("strlen(len):%ld\n",strlen(line));
			// for (i = 0; i < strlen(line); i++) {
			// 	if (!isspace(line[i])) {
			// 		break;
			// 	}
			// }
			// if (i == strlen(line)-1) { //공백만 있는 경우
			// 	continue;
			// }
		}
		
		printf("Command entered: %s (test)\n", line); //입력된 명령어 확인용
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		for(int i= 0; i<strlen(line); i++) {
			if (line[i] == '|') {
				isPiped = TRUE;
				break;
			}
		}
		if (isPiped)
			tokens = tokenize_pipe(line);
		else
			tokens = tokenize(line); //line에 있는 명령어 한 줄을 token으로 만듦

		//do whatever you want with the commands, here we just print them

		for(i = 0; tokens[i] != NULL; i++){
			printf("found token %s (test:%d번째 토큰)\n", tokens[i], i); //생성된 토큰 하나씩 확인
		}
		printf("tokens[0] : %s\n",tokens[0]);
		//printf("isPiped : %d\n", isPiped);
		if (strcmp(tokens[0], "pps") == 0) { //pps명령어인지 확인 후 실행
			printf("pps 명령어 실행\n");
			res = execute_pps(tokens);
		}
		else if (strcmp(tokens[0], "ttop") ==0) { //ttop명령어인지 확인 후 실행
			printf("ttop 명령어 실행\n");
			res = execute_ttop(tokens);
		}
		else if (isPiped) {
			printf("파이프 명령어 실행\n");
			res = execute_piped(tokens);
		}
		else { //나머지 명령어 (리눅스 내장 명령어) 실행
			printf("내장 명령어 실행\n");
			res = execute_built_in(tokens);
		}

		if (res == FALSE) { //명령어 수행 실패한 경우
			printf("SSUShell : Incorrect command\n");
		}
       
		// Freeing the allocated memory	(토큰 메모리 해제)
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);

	}
	return 0;
}

static void exit_ssushell_handler(int signo) {
	//SIGINT 시그널 들어오면, 프로그램을 종료함

	//현재 생성된 자식 프로세스가 있는지 확인 후 종료
	//in here
	//

	//현재 좀비 상태인 프로세스가 있는지 확인 후 종료
	//in here
	//

	//모든 종료 작업 마친 후 프로그램 종료
	exit;
}

int check_complete_process(int status) {
	//현재 비정상적인 프로세스 (좀비상태나 올바르게 삭제되지 못한 자식 프로세스)를 확인한 후 결과값을 리턴하는 함수
	//정상 종료된 경우 TRUE(1), 문제 있는 경우 FALSE(0)리턴

	if (WIFEXITED(status)) { //정상 종료된 경우
		return TRUE;
	}
	else if (WIFSIGNALED(status)) { //자식프로세스가 시그널을 받았지만, 처리하지 않아 비정상 적으로 출력된 경우
		return FALSE;
	}
// #ifdef WCOREDUMP //코어파일 생성ㅇ여부 확인하는 매크로 재정의
// WCOREDUMP(status) ? " (core file generated)" : "");
// #else("");
// #endif
	else if (WIFSTOPPED(status)) { // 현재 자식프로세스가 중지 상태인 경우
		return FALSE;
	}
}
