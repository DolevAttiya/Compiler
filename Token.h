#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE* yyin, * yyout;

typedef enum eTOKENS
{
	INT_tok,
	FLOAT_tok,
	VOID_tok,
	IF_tok,
	RETURN_tok,
	NUM_tok,
	OP_tok,
	ID_tok,
	COMMA_tok,
	COLON_tok,
	SEMICOLON_tok,
	PARENTHES_OPEN_tok,
	PARENTHES_CLOSE_tok,
	BRACKET_OPEN_tok,
	BRACKET_CLOSE_tok,
	INT_NUM_tok,
	FLOAT_NUM_tok,
	ADD_OP_tok,
	MUL_OP_tok,	
	LESS_tok,
	LESS_EQUAL_tok,
	EQUAL_tok,
	GREATER_tok,
	GREATER_EQUAL_tok,
	NOT_EQUAL_tok,
	ASSIGNMENT_OP_tok,
	CURLY_BRACKET_OPEN_tok,
	CURLY_BRACKET_CLOSE_tok,
	EOF_tok
}eTOKENS;

typedef struct Token
{
	eTOKENS kind;
	char* lexeme;
	int lineNumber;
}Token;

typedef struct Node
{
	Token* tokensArray;
	struct Node* prev;
	struct Node* next;
} Node;

void create_and_store_token(eTOKENS kind, char* lexeme, int numOfLine);
Token* next_token();
Token* back_token();

#endif