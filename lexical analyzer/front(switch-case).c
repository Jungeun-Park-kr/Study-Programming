/* front.c - a lexical analyzer system for simple
 * arithmetic expressions */
#include <stdio.h> 
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#pragma warning(disable:4996)

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
#define WHILE_LOOP 40 // While
#define SEMICOLON 50 // ;

char WHILE[6] = "while";


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
	switch (ch) {
	case '(':
		addChar();
		nextToken = LEFT_PAREN;
		break;
	case ')':
		addChar();
		nextToken = RIGHT_PAREN;
		break;
	case '+':
		addChar();
		nextToken = ADD_OP;
		break;
	case '-':
		addChar();
		nextToken = SUB_OP;
		break;
	case '*':
		addChar();
		nextToken = MULT_OP;
		break;
	case '/':
		addChar();
		nextToken = DIV_OP;
		break;
	case '{' :
		addChar();
		nextToken = LEFT_BRACE;
		break;
	case '}' :
		addChar();
		nextToken = RIGHT_BRACE;
		break;
	case '<' :
		addChar();
		nextToken = LESSER;
		break;
	case '>' :
		addChar();
		nextToken = GREATER;
		break;
	case '=' :
		addChar();
		nextToken = EQUAL_SIGN;
		break;
	case ';' :
		addChar();
		nextToken = SEMICOLON;
		break;
	default:
		addChar();
		nextToken = EOF;
		break;
	}
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

/***************************************************** /
/* lex - a simple lexical analyzer for arithmetic expressions */
int lex() {
	lexLen = 0;
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
		if (!strcmp(lexeme, WHILE)) { //해당 lexeme이 while인지 확인
			nextToken = WHILE_LOOP;
			break;
		}
		else {
			nextToken = IDENT;
			break;
		}
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
