/* front.c - a lexical analyzer system for simple
 * arithmetic expressions */
#include <stdio.h> 
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
//#pragma warning(disable:4996)

/* Global declarations */
/* Variables */
int charClass; //LETTER, DIGIT, UNKNOWN
char lexeme[100]; //토큰
char nextChar; //다음 문자 가져와서 lexeme에 추가해줌
int lexLen;
int token; //토큰 코드
int nextToken;
FILE *in_fp;

/* Function declarations */
void addChar();
void getChar();
void getNonBlank();
int lex();

/* Character classes */
#define LETTER 0 
#define DIGIT 1 
#define UNKNOWN 99

/* Token codes */
#define INT_LIT 10 
#define IDENT 11 
#define ASSIGN_OP 20 
#define ADD_OP 21 
#define SUB_OP 22 
#define MULT_OP 23 
#define DIV_OP 24 
#define LEFT_PAREN 25 
#define RIGHT_PAREN 26
//token code 추가
#define LEFT_BRACE 27 // { 왼 중괄호
#define RIGHT_BRACE 28 // } 오른 중괄호
#define LESSER 29 // <
#define GREATER 30 // >
#define EQUAL_SIGN 31 // =
#define SEMICOLON 40 // ;
#define WHILE_LOOP 50 // While
#define FOR_LOOP 51	//for
#define IF 55 //if
#define ELSE 56 //else
#define SWITCH 60 //switch
#define CASE 61 //case



char WHILE[6] = "while";

#define TOK_TABLE_SIZE (sizeof(LookupTable)/sizeof(*LookupTable))
#define RES_TABLE_SIZE (sizeof(ReservedTable)/sizeof(*ReservedTable))
struct {
	char token;
	int code;
} LookupTable[] = {
	{'(',LEFT_PAREN}, {')',RIGHT_PAREN}, {'+',ADD_OP}, {'-',SUB_OP},
	{'*', MULT_OP}, {'/', DIV_OP}, {'{',LEFT_BRACE}, {'}',RIGHT_BRACE},
	{'<', LESSER}, {'>', GREATER}, {'=',EQUAL_SIGN}, {';',SEMICOLON}
};

struct {
	char token[10];
	int code;
} ReservedTable[] = {
	{"while",WHILE_LOOP}, {"if", IF}, {"else", ELSE}, 
	{"switch", SWITCH}, {"case",CASE}, {"for", FOR_LOOP}
	//,{"int", }, {"char", }, {"double", }, {"float", }
};

/******************************************************/
/* main driver */
int main() {
	/* Open the input data file and process its contents */
	if ((in_fp = fopen("test.c", "r")) == NULL)
		printf("ERROR - cannot open test.c \n");
	else {
		getChar();
		do { 
			lex(); 
		} while (nextToken != EOF);
	}
}
/*****************************************************/ 

/* lookup - a function to lookup operators and parentheses            
and return the token */ 
int lookup(char ch) { 
	int i;
	for (i = 0; i < TOK_TABLE_SIZE; i++) { //lookupTable을 이용하여 해당 토큰을 확인
		if (ch == LookupTable[i].token) { //동일한 토큰이 있는경우, 토큰 코드 저장
			addChar();
			nextToken = LookupTable[i].code;
			return nextToken; //nextToken 리턴
		}
	}
	//테이블에 없는 경우 - EOF임
	addChar();
	nextToken = EOF;
	return nextToken;
}
/*****************************************************/
/* addChar - a function to add nextChar to lexeme */
void addChar() {
	if (lexLen <= 98) {
		lexeme[lexLen++] = nextChar;
		lexeme[lexLen] = 0;
	}
	else
		printf("Error - lexeme is too long \n");
}

/*****************************************************/
/* getChar - a function to get the next character of
input and determine its character class */
void getChar() {
	if ((nextChar = getc(in_fp)) != EOF)
	{
		if (isalpha(nextChar)) {
			//if (nextChar == 'w')
			charClass = LETTER;
		}
		else if (isdigit(nextChar))
			charClass = DIGIT;
		else charClass = UNKNOWN;
	}
	else		
		charClass = EOF;
}

/*****************************************************/
/* getNonBlank - a function to call getChar until it
returns a non-whitespace character */
void getNonBlank() {
	while (isspace(nextChar))
		getChar();
}

/*****************************************************/
/* lex - a simple lexical analyzer for arithmetic expressions */
int lex() {
	lexLen = 0;
	int i = 0;
	getNonBlank();
	switch (charClass) {
	/* Parse identifiers */
	case LETTER:
		addChar();
		getChar();
		while (charClass == LETTER || charClass == DIGIT) {
			addChar();
			getChar();
		}
		for(i=0; i<RES_TABLE_SIZE; i++) {
			if (!strcmp(lexeme, ReservedTable[i].token)) { //해당 lexeme이 while인지 확인
				nextToken = ReservedTable[i].code;
				break;
			}
		}
		nextToken = IDENT;
		break;
		
	/* Parse integer literals */
	case DIGIT:
		addChar();
		getChar();
		while (charClass == DIGIT) {
			addChar();
			getChar();
		}
		nextToken = INT_LIT;
		break;
	/* Parentheses, operators and reserved words*/
	case UNKNOWN:
		lookup(nextChar);
		getChar();
		break;
	/* EOF */
	case EOF:
		nextToken = EOF;
		lexeme[0] = 'E';
		lexeme[1] = 'O';
		lexeme[2] = 'F';
		lexeme[3] = 0;
		break;
	} /* End of switch */
	printf("Next token is: %d, Next lexeme is %s\n", nextToken, lexeme);
	return nextToken;
}  /* End of function lex */
