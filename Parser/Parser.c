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


void parse_RETURN_TYPE() {
	eTOKENS follow[] = { ID_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS tokens[] = {/*Firsts*/ INT_tok, FLOAT_tok, VOID_tok};
	expected_token_types = tokens;
	expected_token_types_size = 3;
	current_token = next_token();
	switch (current_token->kind)
	{
	case INT_tok:
	case FLOAT_tok:
		fprintf(parser_output_file, "Rule {RETURN_TYPE -> TYPE}");
		back_token();
		parse_TYPE();
		break;
	case VOID_tok:
		fprintf(parser_output_file, "Rule {RETURN_TYPE -> void}");
		back_token();
		if (!match(VOID_tok))
			return;
		break;
	default:
		error();
		break;
	}
}

void parse_PARAMS() {
	eTOKENS follow[] = { PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS tokens[] = {/*Firsts*/ INT_tok, FLOAT_tok,/*Follows*/ PARENTHESIS_CLOSE_tok };
	expected_token_types = tokens;
	expected_token_types_size = 3;
	current_token = next_token();
	switch (current_token->kind)
	{
	case INT_tok:
	case FLOAT_tok:
		fprintf(parser_output_file, "Rule {PARAMS -> PARAMS_LIST}");
		back_token();
		parse_PARAM_LIST();
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {PARAMS -> epsilon}");
			back_token();
			break;
		}
		error();
		break;
	}
}

void parse_PARAM_LIST() {
	fprintf(parser_output_file, "Rule {PARAMS_LIST -> PARAM PARAMS_LIST'}");
	parse_PARAM();
	parse_PARAM_LIST_TAG();
}

void parse_PARAM_LIST_TAG() {
	eTOKENS follow[] = { PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS tokens[] = {/*Firsts*/ COMMA_tok,/*Follows*/ PARENTHESIS_CLOSE_tok};
	expected_token_types = tokens;
	expected_token_types_size = 2;
	current_token = next_token();
	switch (current_token->kind)
	{
	case COMMA_tok:
		fprintf(parser_output_file, "Rule {PARAMS_LIST' -> , PARAM PARAMS_LIST'}");
		parse_PARAM();
		parse_PARAM_LIST_TAG();
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {PARAMS_LIST' -> epsilon}");
			back_token();
			break;
		}
		error();
		break;
	}
}

void parse_PARAM() {
	eTOKENS follow[] = { COMMA_tok, PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	fprintf(parser_output_file, "Rule {PARAM -> TYPE id PARAM'}");
	parse_TYPE();
	if (!match(ID_tok))
		return;
	parse_PARAM_TAG();
}

void parse_PARAM_TAG() {
	eTOKENS follow[] = { COMMA_tok, PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	eTOKENS tokens[] = {/*Firsts*/ BRACKET_OPEN_tok,/*Follows*/ COMMA_tok, PARENTHESIS_CLOSE_tok };
	expected_token_types = tokens;
	expected_token_types_size = 3;
	current_token = next_token();
	switch (current_token->kind)
	{
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {PARAM' -> [DIM_SIZES]}");
		parse_DIM_SIZES();
		if (!match(BRACKET_CLOSE_tok))
			return;
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {PARAM' -> epsilon}");
			back_token();
			break;
		}
		error();
		break;
	}
}

void parse_COMP_STMT() {
	eTOKENS follow[] = { INT_tok, FLOAT_tok, VOID_tok, EOF_tok, SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 6;
	fprintf(parser_output_file, "Rule {COMP_STMT -> { VAR_DEC_LIST STMT_LIST }}");
	if (!match(CURLY_BRACKET_OPEN_tok))
		return;
	parse_VAR_DEC_LIST();
	parse_STMT_LIST();
	if (!match(CURLY_BRACKET_CLOSE_tok))
		return;
}

void parse_VAR_DEC_LIST() {
	fprintf(parser_output_file, "Rule {VAR_DEC_LIST -> VAR_DEC_LIST'}");
	parse_VAR_DEC_LIST_TAG();
}
void parse_VAR_DEC_LIST_TAG() {
	eTOKENS follow[] = { ID_tok, CURLY_BRACKET_OPEN_tok, IF_tok, RETURN_tok };
	current_follow = follow;
	current_follow_size = 4;
	eTOKENS tokens[] = { INT_tok, FLOAT_tok };
	expected_token_types = tokens;
	expected_token_types_size = 2;
	current_token = next_token();
	switch (current_token->kind)
	{
	case INT_tok:
	case FLOAT_tok:
		fprintf(parser_output_file, "Rule {VAR_DEC_LIST' -> VAR_DEC VAR_DEC_LIST'}");
		back_token();
		parse_VAR_DEC();
		parse_VAR_DEC_LIST_TAG();
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {VAR_DEC_LIST' -> epsilon}");
			back_token();
			break;
		}
		error();
		break;
	}
}
void parse_STMT_LIST() {
	fprintf(parser_output_file, "Rule {STMT_LIST -> STMT STMT_LIST'}");
	parse_STMT();
	parse_STMT_LIST_TAG();
}
void parse_STMT_LIST_TAG() {
	eTOKENS follow[] = { CURLY_BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS tokens[] = {/*Firsts*/SEMICOLON_tok ,/*Follow*/CURLY_BRACKET_CLOSE_tok};
	expected_token_types = tokens;
	expected_token_types_size = 2;
	current_token = next_token();
	switch (current_token->kind)
	{
	case SEMICOLON_tok:
		fprintf(parser_output_file, "Rule {STMT_LIST' -> ; STMT STMT_LIST'}");
		parse_STMT();
		parse_STMT_LIST_TAG();
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {STMT_LIST' -> epsilon}");
			back_token();
			break;
		}
		error();
		break;
	}
}
void parse_STMT() {
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	eTOKENS tokens[] = { ID_tok, CURLY_BRACKET_OPEN_tok, IF_tok, RETURN_tok };
	expected_token_types = tokens;
	expected_token_types_size = 4;
	current_token = next_token();
	switch (current_token->kind)
	{
	case ID_tok:
		fprintf(parser_output_file, "Rule {STMT -> id VAR_OR_CALL}");
		parse_VAR_OR_CALL();
		break;
	case CURLY_BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {STMT -> COMP_STMT}");
		back_token();
		parse_COMP_STMT();
		break;
	case IF_tok:
		fprintf(parser_output_file, "Rule {STMT -> IF_STMT}");
		back_token();
		parse_IF_STMT();
		break;
	case RETURN_tok:
		fprintf(parser_output_file, "Rule {STMT -> RETURN_STMT}");
		back_token();
		parse_RETURN_STMT();
		break;
	default:
		error();
		break;
	}
}
void parse_VAR_OR_CALL() {
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	eTOKENS tokens[] = { PARENTHESIS_OPEN_tok, BRACKET_OPEN_tok,ASSIGNMENT_OP_tok };
	expected_token_types = tokens;
	expected_token_types_size = 3;
	current_token = next_token();
	switch (current_token->kind)
	{
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL -> (ARGS)}");
		parse_ARGS();
		if (!match(PARENTHESIS_CLOSE_tok))
			return;
		break;
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL -> VAR' = EXPR}");
		back_token();
		parse_VAR_TAG();
		if (!match(ASSIGNMENT_OP_tok))
			return;
		parse_EXPR();
		break;
	case ASSIGNMENT_OP_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL -> VAR' = EXPR}");
		parse_EXPR();
		break;
	default:
		error();
		break;
	}
}
void parse_IF_STMT() {
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	fprintf(parser_output_file, "Rule {IF_STMT -> if (CONDITION) STMT}");
	if (!match(IF_tok))
		return;
	if (!match(PARENTHESIS_OPEN_tok))
		return;
	parse_CONDITION();
	if (!match(PARENTHESIS_CLOSE_tok))
		return;
	parse_STMT();
}
void parse_ARGS() {
	eTOKENS follow[] = { PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS tokens[] = {/*Firsts*/ ID_tok, INT_NUM_tok, FLOAT_NUM_tok, PARENTHESIS_OPEN_tok,/*Follows*/ PARENTHESIS_CLOSE_tok};
	expected_token_types = tokens;
	expected_token_types_size = 5;
	current_token = next_token();
	switch (current_token->kind)
	{
	case ID_tok:
	case INT_NUM_tok:
	case FLOAT_NUM_tok:
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {ARGS -> ARG_LIST}");
		back_token();
		parse_ARG_LIST();
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {ARGS -> epsilon}");
			back_token();
			break;
		}
		error();
		break;
	}
}

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
	} while (parse_Follow() == 0 && current_token->kind != EOF_tok);
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