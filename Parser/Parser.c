#include "../Lexical Analyzer/Token/Token.h"
#include "../Lexical Analyzer/Source Code/win.lex.yy.c"
#include "Parser.h"

#include <string.h>

Token* current_token;
eTOKENS* current_follow;
int current_follow_size;
eTOKENS* expected_token_types;
int expected_token_types_size;
FILE* parser_output_file;


int match(eTOKENS expected_token) {
	eTOKENS expected_token_array[] = { expected_token };
	current_token = next_token();
	if (current_token->kind != expected_token)
	{
		expected_token_types = expected_token_array;
		expected_token_types_size = 1;
		error();
		return 0;
	}
	return 1;
}
int parse_Follow()
{
	int flag = 0;
	for (int i = 0; i < current_follow_size; i++)
	{
		flag += current_token->kind == current_follow[i];
	}
	return flag;
}
void error() {

	char* tokens_names;
	get_tokens_names(&tokens_names);
	fprintf(parser_output_file, "Expected token of type '{%s}' at line: {%d},  Actual token of type '{%d}', lexeme: '{%s}'.", tokens_names, current_token->lineNumber, current_token->kind, current_token->lexeme);
	do
	{
		current_token = next_token();
	} while (parse_Follow() == 0 && current_token -> kind != EOF_tok);
	back_token();
	free(tokens_names);
}
void get_tokens_names(char** tokens)
{
	*tokens = (char**)malloc(sizeof(char*));
	(*tokens)[0] = '\0';
	for (int i = 0; i < expected_token_types_size; i++)
	{
		strncat(*tokens, eTokensStrings[expected_token_types[i]],
			strlen(eTokensStrings[expected_token_types[i]]));
		strncat(*tokens, ", ", strlen(", "));
	}
	strncpy(*tokens, *tokens, strlen(*tokens) - 2);
	(*tokens)[strlen(*tokens) - 2] = 0;
}
