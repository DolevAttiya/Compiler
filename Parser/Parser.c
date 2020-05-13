#include "../Lexical Analyzer/Token/Token.h"
#include "Parser.h"

Token* current_token;
eTOKENS* current_follow;
int current_follow_size;
eTOKENS* expected_token_types;
int expected_token_types_size;
FILE* parser_output_file;


int match(eTOKENS expected_token) {
	current_token = next_token();
	if (current_token->kind != expected_token)
	{
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
	fprintf(parser_output_file, "Expected token of type '{%d}' at line: {%d},  Actual token of type '{%d}', lexeme: '{%c}'.", expected_token_type, current_token->lineNumber, current_token->kind, current_token->lexeme);
	do
	{
		current_token = next_token();
	} while (parse_Follow() == 0 && current_token->kind != EOF_tok);
	back_token();
}


void parse_ARG_LIST() {
	fprintf(parser_output_file, "Rule {ARG_LIST -> EXPR ARG_LIST'}");
	parse_EXPR();
	parse_ARG_LIST_TAG();
}
void parse_ARG_LIST_TAG() {
	eTOKENS follow[] = { PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	current_token = next_token();
	switch (current_token->kind)
	{
	case COMMA_tok:
		fprintf(parser_output_file, "Rule {ARG_LIST' -> , EXPR ARG_LIST'}");
		if (!match(COMMA_tok))
			return;
		parse_EXPR();
		parse_ARG_LIST_TAG();
		break;
	case PARENTHESIS_CLOSE_tok:
		fprintf(parser_output_file, "Rule {DIM_SIZES' -> Epsilon}");
		back_token();
		break;
	default:
		error();
		break;
	}
}
void parse_RETURN_STMT() {
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	current_token = next_token();
	fprintf(parser_output_file, "Rule {RETURN_STMT -> return RETURN_STMT'}");
	if (!match(current_token))
		return;
	parse_RETURN_STMT_TAG();
}
void parse_RETURN_STMT_TAG() {
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok };
	eTOKENS expected_tokens[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 2;
	current_token = next_token();
	switch (current_token->kind)
	{
	case ID_tok:
	case INT_tok:
	case FLOAT_tok:
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {RETURN_STMT' -> EXPR}");
		current_token = back_token();
		parse_EXPR();
		break;
	case SEMICOLON_tok:
	case CURLY_BRACKET_CLOSE_tok:
		fprintf(parser_output_file, "Rule {RETURN_STMT' -> Epsilon}");
		current_token = back_token();
		break;
	default:
		error();
		break;
	}
}
void parse_VAR_TAG() {
	fprintf(parser_output_file, "Rule {VAR' -> [EXPR_LIST]}");
	fprintf(parser_output_file, "Rule {VAR' -> Epsilon}");
}
void parse_EXPR_LIST() {
	fprintf(parser_output_file, "Rule {EXPR_LIST -> EXPR EXPR_LIST'}");
	parse_EXPR();
	parse_EXPR_LIST_TAG();
}
void parse_EXPR_LIST_TAG() {
	eTOKENS follow[] = { BRACKET_CLOSE_tok };
	eTOKENS expected_tokens[] = { BRACKET_CLOSE_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 1;
	current_token = next_token();
	switch (current_token->kind)
	{
	case COMMA_tok:
		fprintf(parser_output_file, "Rule {EXPR_LIST' -> , EXPR EXPR_LIST'}");
		if (!match(COMMA_tok))
			return;
		parse_EXPR();
		parse_EXPR_LIST_TAG();
		break;
	case BRACKET_CLOSE_tok:
		fprintf(parser_output_file, "Rule {DIM_SIZES' -> Epsilon}");
		back_token();
		break;
	default:
		error();
		break;
	}
	
	fprintf(parser_output_file, "Rule {EXPR_LIST' -> Epsilon}");
}
void parse_CONDITION() {
	fprintf(parser_output_file, "Rule {CONDITION -> EXPR rel_op EXPR}");
	parse_EXPR();
	current_token = next_token();
	if ((current_token->kind == LESS_tok) || (current_token->kind == LESS_EQUAL_tok) || (current_token->kind == EQUAL_tok) || (current_token == GREATER_tok) || (current_token == GREATER_EQUAL_tok) || (current_token->kind == NOT_EQUAL_tok))
		parse_EXPR();
	else
	{
		eTOKENS expected_token_types[] = { LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok };
		expected_token_types_size = 6;
		error();
	}
}
void parse_EXPR() {
	fprintf(parser_output_file, "Rule {EXPR -> TERM EXPR'}");
	parse_TERM();
	parse_EXPR_TAG();
}
void parse_EXPR_TAG() {
	fprintf(parser_output_file, "Rule {EXPR' -> + TERM EXPR'}");
	fprintf(parser_output_file, "Rule {EXPR' -> Epsilon}");
}
void parse_TERM() {
	fprintf(parser_output_file, "Rule {TERM -> FACTOR TERM'}");
	parse_FACTOR();
	parse_TERM_TAG();
}
void parse_TERM_TAG() {
	fprintf(parser_output_file, "Rule {TERM' ->  * FACTOR TERM'}");
	fprintf(parser_output_file, "Rule {TERM' ->  Epsilon}");
}
void parse_FACTOR() {
	fprintf(parser_output_file, "Rule {FACTOR -> id VAR_OR_CALL'}");
	fprintf(parser_output_file, "Rule {FACTOR -> int_num}");
	fprintf(parser_output_file, "Rule {FACTOR -> float_num}");
	fprintf(parser_output_file, "Rule {FACTOR -> (EXPR)}");
}
void parse_VAR_OR_CALL_TAG() {
	fprintf(parser_output_file, "Rule {VAR_OR_CALL' -> VAR'}");
	fprintf(parser_output_file, "Rule {VAR_OR_CALL' -> (ARGS)}");
}
