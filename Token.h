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
	PARENTHESES_tok,
	BRACKETS_tok,
	COMMENT_tok,
	INT_NUM_tok,
	FLOAT_NUM_tok,
	AR_OP_tok,
	REL_OP_tok,
	ASSIGNMENT_OP_tok,
	CURLY_BRACKETS_tok
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