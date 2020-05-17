#include "../Lexical Analyzer/Token/Token.h"
char* eTokensStrings[];
#include "Parser.h"
#include <string.h>

Token* current_token;
eTOKENS* current_follow;
int current_follow_size;
eTOKENS* expected_token_types;
int expected_token_types_size;

FILE* parser_output_file;

void parser()
{
	parse_PROG();
	clean_token_storage();
}

void parse_PROG()
{
	eTOKENS follow[] = { EOF_tok };
	current_follow = follow;
	current_follow_size = 1;
	fprintf(parser_output_file, "Rule {PROG -> GLOBAL_VARS FUNC_PREDEFS FUNC_FULL_DEFS}\n");
	fprintf(parser_output_file, "Rule {GLOBAL_VARS -> VAR_DEC GLOBAL_VARS'}\n");
	parse_GLOBAL_VARS();
	do {
		for(int i=0;i<3;i++) 
			current_token=next_token();
		if (current_token->kind != PARENTHESIS_OPEN_tok)
		{
			for (int i = 0; i < 3; i++)
				back_token();
			fprintf(parser_output_file, "Rule {GLOBAL_VARS' -> VAR_DEC GLOBAL_VARS'}\n");
			parse_GLOBAL_VARS();
		}
	} while (current_token->kind != PARENTHESIS_OPEN_tok);

	fprintf(parser_output_file, "Rule {GLOBAL_VARS' -> epsilon}\n");

	for (int i = 0; i < 3; i++)
		back_token();

	fprintf(parser_output_file, "Rule {FUNC_PREDEFS -> FUNC_PROTYTYPE; FUNC_PREDEFS'}\n");
	do {		
		parse_FUNC_PROTOTYPE();
		current_token = next_token();
		if (current_token->kind == SEMICOLON_tok)
			fprintf(parser_output_file, "Rule {FUNC_PREDEFS' -> FUNC_PROTYTYPE; FUNC_PREDEFS'}\n");
	} while (current_token->kind == SEMICOLON_tok);

	fprintf(parser_output_file, "Rule {FUNC_PREDEFS' -> epsilon}\n");

	fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS -> FUNC_WITH_BODY FUNC_FULL_DEFS'}\n");
	fprintf(parser_output_file, "Rule {FUNC_WITH_BODY -> FUNC_PROTOTYPE COMP_STMT}\n");
	back_token();       
	parse_COMP_STMT();
	current_token = next_token();
	if (current_token->kind != EOF_tok)
	{
		back_token();    
		parse_FUNC_FULL_DEFS();
	}
}

void parse_GLOBAL_VARS()
{
	current_token = next_token();
	eTOKENS follow[] = { INT_tok, FLOAT_tok, VOID_tok};
	current_follow = follow;
	current_follow_size = 3;
	eTOKENS tokens[] = { INT_tok, FLOAT_tok };
	expected_token_types = tokens;
	expected_token_types_size = 2;

	switch (current_token->kind)
	{
	case INT_tok:
	case FLOAT_tok:
		back_token();
		parse_VAR_DEC();
		break;
	default:
		error();
		break;
	}
}

void parse_VAR_DEC()
{
	eTOKENS follow[] = { INT_tok, FLOAT_tok, VOID_tok, ID_tok, CURLY_BRACKET_OPEN_tok, IF_tok, RETURN_tok };
	current_follow = follow;
	current_follow_size = 7;

	fprintf(parser_output_file, "Rule {VAR_DEC -> TYPE id VAR_DEC'}\n");
	parse_TYPE();
	if (!match(ID_tok))
		return;
	parse_VAR_DEC_TAG();	
}

void parse_VAR_DEC_TAG()
{
	eTOKENS follow[] = { INT_tok, FLOAT_tok, VOID_tok, ID_tok, CURLY_BRACKET_OPEN_tok, IF_tok, RETURN_tok };
	current_follow = follow;
	current_follow_size = 7;
	current_token = next_token();
	eTOKENS tokens[] = { SEMICOLON_tok, BRACKET_OPEN_tok };
	expected_token_types = tokens;
	expected_token_types_size = 2;

	switch (current_token->kind)
	{
	case SEMICOLON_tok: 
		fprintf(parser_output_file, "Rule {VAR_DEC' -> ;}\n");
		break;
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_DEC' -> [DIM_SIZES] ;}\n");
		parse_DIM_SIZES();
		if (!match(BRACKET_CLOSE_tok))
			return;
		if (!match(SEMICOLON_tok))
			return;
		break;
	default:
		error();
		break;
	}
}

void parse_TYPE()
{
	eTOKENS follow[] = { ID_tok };
	current_follow = follow;
	current_follow_size = 1;
	current_token = next_token();
	eTOKENS tokens[] = { INT_tok, FLOAT_tok };
	expected_token_types = tokens;
	expected_token_types_size = 2;

	switch (current_token->kind)
	{
	case INT_tok:
		fprintf(parser_output_file, "Rule {TYPE -> int}\n");
		break;
	case FLOAT_tok:
		fprintf(parser_output_file, "Rule {TYPE -> float}\n");
		break;
	default:
		error();
		break;
	}
}

void parse_DIM_SIZES()
{
	eTOKENS follow[] = { BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	fprintf(parser_output_file, "Rule {DIM_SIZES -> int_num DIM_SIZES'}\n");
	if (!match(INT_NUM_tok))
		return;
	parse_DIM_SIZES_TAG();
}

void parse_DIM_SIZES_TAG()
{
	eTOKENS follow[] = { BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	current_token = next_token();
	eTOKENS tokens[] = { COMMA_tok, BRACKET_CLOSE_tok };
	expected_token_types = tokens;
	expected_token_types_size = 2;

	switch (current_token->kind)
	{
	case COMMA_tok:
		fprintf(parser_output_file, "Rule {DIM_SIZES' -> , DIM_SIZES}\n");
		parse_DIM_SIZES();
		break;
	case BRACKET_CLOSE_tok:
		fprintf(parser_output_file, "Rule {DIM_SIZES' -> epsilon}\n");
		back_token();
		break;
	default:
		error();
		break;
	}
}

void parse_FUNC_PROTOTYPE()
{
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_OPEN_tok };
	current_follow = follow;
	current_follow_size = 2;
	fprintf(parser_output_file, "Rule {FUNC_PROTOTYPE -> RETURN_TYPE id (PARAMS)}\n");
	parse_RETURN_TYPE();
	if (!match(ID_tok))
		return;
	if (!match(PARENTHESIS_OPEN_tok))
		return;
	parse_PARAMS();
	if (!match(PARENTHESIS_CLOSE_tok))
		return;
}

void parse_FUNC_FULL_DEFS()
{
	fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS -> FUNC_WITH_BODY FUNC_FULL_DEFS'}\n");
	parse_FUNC_WITH_BODY();
	parse_FUNC_FULL_DEFS_TAG();
}

void parse_FUNC_FULL_DEFS_TAG()
{
	eTOKENS follow[] = { EOF_tok };
	current_follow = follow;
	current_follow_size = 1;
	current_token = next_token();
	eTOKENS tokens[] = { INT_tok, FLOAT_tok, VOID_tok, EOF_tok };
	expected_token_types = tokens;
	expected_token_types_size = 4;

	switch (current_token->kind)
	{
	case INT_tok:
	case FLOAT_tok:
	case VOID_tok:
		fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS' -> FUNC_FULL_DEFS}\n");
		back_token();
		parse_FUNC_FULL_DEFS();
		break;
	case EOF_tok:
		fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS' -> epsilon}\n");
		back_token();
		break;
	default:
		error();
		break;
	}
}

void parse_FUNC_WITH_BODY()
{
	fprintf(parser_output_file, "Rule {FUNC_WITH_BODY -> FUNC_PROTOTYPE COMP_STMT}\n");
	parse_FUNC_PROTOTYPE();
	parse_COMP_STMT();
}


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
		fprintf(parser_output_file, "Rule {RETURN_TYPE -> TYPE}\n");
		back_token();
		parse_TYPE();
		break;
	case VOID_tok:
		fprintf(parser_output_file, "Rule {RETURN_TYPE -> void}\n");
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
		fprintf(parser_output_file, "Rule {PARAMS -> PARAMS_LIST}\n");
		back_token();
		parse_PARAM_LIST();
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {PARAMS -> epsilon}\n");
			back_token();
			break;
		}
		error();
		break;
	}
}

void parse_PARAM_LIST() {
	fprintf(parser_output_file, "Rule {PARAMS_LIST -> PARAM PARAMS_LIST'}\n");
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
		fprintf(parser_output_file, "Rule {PARAMS_LIST' -> , PARAM PARAMS_LIST'}\n");
		parse_PARAM();
		parse_PARAM_LIST_TAG();
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {PARAMS_LIST' -> epsilon}\n");
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
	fprintf(parser_output_file, "Rule {PARAM -> TYPE id PARAM'}\n");
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
		fprintf(parser_output_file, "Rule {PARAM' -> [DIM_SIZES]}\n");
		parse_DIM_SIZES();
		if (!match(BRACKET_CLOSE_tok))
			return;
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {PARAM' -> epsilon}\n");
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
	fprintf(parser_output_file, "Rule {COMP_STMT -> { VAR_DEC_LIST STMT_LIST }}\n");
	if (!match(CURLY_BRACKET_OPEN_tok))
		return;
	parse_VAR_DEC_LIST();
	parse_STMT_LIST();
	if (!match(CURLY_BRACKET_CLOSE_tok))
		return;
}

void parse_VAR_DEC_LIST() {
	fprintf(parser_output_file, "Rule {VAR_DEC_LIST -> VAR_DEC_LIST'}\n");
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
		fprintf(parser_output_file, "Rule {VAR_DEC_LIST' -> VAR_DEC VAR_DEC_LIST'}\n");
		back_token();
		parse_VAR_DEC();
		parse_VAR_DEC_LIST_TAG();
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {VAR_DEC_LIST' -> epsilon}\n");
			back_token();
			break;
		}
		error();
		break;
	}
}
void parse_STMT_LIST() {
	fprintf(parser_output_file, "Rule {STMT_LIST -> STMT STMT_LIST'}\n");
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
		fprintf(parser_output_file, "Rule {STMT_LIST' -> ; STMT STMT_LIST'}\n");
		parse_STMT();
		parse_STMT_LIST_TAG();
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {STMT_LIST' -> epsilon}\n");
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
		fprintf(parser_output_file, "Rule {STMT -> id VAR_OR_CALL}\n");
		parse_VAR_OR_CALL();
		break;
	case CURLY_BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {STMT -> COMP_STMT}\n");
		back_token();
		parse_COMP_STMT();
		break;
	case IF_tok:
		fprintf(parser_output_file, "Rule {STMT -> IF_STMT}\n");
		back_token();
		parse_IF_STMT();
		break;
	case RETURN_tok:
		fprintf(parser_output_file, "Rule {STMT -> RETURN_STMT}\n");
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
		fprintf(parser_output_file, "Rule {VAR_OR_CALL -> (ARGS)}\n");
		parse_ARGS();
		if (!match(PARENTHESIS_CLOSE_tok))
			return;
		break;
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL -> VAR' = EXPR}\n");
		back_token();
		parse_VAR_TAG();
		if (!match(ASSIGNMENT_OP_tok))
			return;
		parse_EXPR();
		break;
	case ASSIGNMENT_OP_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL -> VAR' = EXPR}\n");
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
	fprintf(parser_output_file, "Rule {IF_STMT -> if (CONDITION) STMT}\n");
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
		fprintf(parser_output_file, "Rule {ARGS -> ARG_LIST}\n");
		back_token();
		parse_ARG_LIST();
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {ARGS -> epsilon}\n");
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
	fprintf(parser_output_file, "Expected token of type '{%s}' at line: {%d},  Actual token of type '{%d}', lexeme: '{%s}'.\n", tokens_names, current_token->lineNumber, current_token->kind, current_token->lexeme);
	do
	{
		current_token = next_token();
	} while (parse_Follow() == 0 && current_token->kind != EOF_tok);
	back_token();
	free(tokens_names);
}

void get_tokens_names(char** tokens)
{
	*tokens = (char*)malloc(sizeof(char*));
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

void parse_ARG_LIST() {
	fprintf(parser_output_file, "Rule {ARG_LIST -> EXPR ARG_LIST'}\n");
	parse_EXPR();
	parse_ARG_LIST_TAG();
}
void parse_ARG_LIST_TAG() {
	eTOKENS follow[] = { PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS expected_tokens[] = { COMMA_tok, PARENTHESIS_CLOSE_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 2;
	current_token = next_token();
	switch (current_token->kind)
	{
	case COMMA_tok:
		fprintf(parser_output_file, "Rule {ARG_LIST' -> , EXPR ARG_LIST'}\n");
		parse_EXPR();
		parse_ARG_LIST_TAG();
		break;
	case PARENTHESIS_CLOSE_tok:
		fprintf(parser_output_file, "Rule {DIM_SIZES' -> Epsilon}\n");
		back_token();
		break;
	default:
		error();
		break;
	}
}
void parse_RETURN_STMT() {
	fprintf(parser_output_file, "Rule {RETURN_STMT -> return RETURN_STMT'}\n");
	if(!match(RETURN_tok))
		return;
	parse_RETURN_STMT_TAG();
}
void parse_RETURN_STMT_TAG() {
	// First of EXPR - id int_num float_num (
	// Follow of RETURN_STMT' - ; }
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	eTOKENS expected_tokens[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, ID_tok, INT_NUM_tok, FLOAT_NUM_tok, PARENTHESIS_OPEN_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 6;
	current_token = next_token();
	switch (current_token->kind)
	{
	case ID_tok:
	case INT_NUM_tok:
	case FLOAT_NUM_tok:
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {RETURN_STMT' -> EXPR}\n");
		back_token();
		parse_EXPR();
		break;
	case SEMICOLON_tok:
	case CURLY_BRACKET_CLOSE_tok:
		fprintf(parser_output_file, "Rule {RETURN_STMT' -> Epsilon}\n");
		back_token();
		break;
	default:
		error();
		break;
	}
}
void parse_VAR_TAG() {
	// Follow of VAR' -  ; } , ) ] rel_op + * =
	// rel_op -  <  <=  ==  >=  >  != 
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok, 
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok,
						 ADD_OP_tok, MUL_OP_tok, ASSIGNMENT_OP_tok };
	current_follow = follow;
	current_follow_size = 14;
	eTOKENS expected_tokens[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok,
						 ADD_OP_tok, MUL_OP_tok, ASSIGNMENT_OP_tok, BRACKET_OPEN_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 15;
	current_token = next_token();
	switch (current_token->kind) {
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR' -> [EXPR_LIST]}\n");
		parse_EXPR_LIST();
		if(!match(BRACKET_CLOSE_tok))
			return;
		break;
	case SEMICOLON_tok:
	case CURLY_BRACKET_CLOSE_tok:
	case COMMA_tok:
	case PARENTHESIS_CLOSE_tok:
	case BRACKET_CLOSE_tok:
	case LESS_tok:
	case LESS_EQUAL_tok:
	case EQUAL_tok:
	case GREATER_tok:
	case GREATER_EQUAL_tok:
	case NOT_EQUAL_tok:
	case ADD_OP_tok:
	case MUL_OP_tok:
	case ASSIGNMENT_OP_tok:
		fprintf(parser_output_file, "Rule {VAR' -> Epsilon}\n");
		back_token();
		break;
	 default:
		error();
		break;
	}
}
void parse_EXPR_LIST() {
	fprintf(parser_output_file, "Rule {EXPR_LIST -> EXPR EXPR_LIST'}\n");
	parse_EXPR();
	parse_EXPR_LIST_TAG();
}
void parse_EXPR_LIST_TAG() {
	// Follow of EXPR_LIST' - ]
	eTOKENS follow[] = { BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS expected_tokens[] = { BRACKET_CLOSE_tok, COMMA_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 2;
	current_token = next_token();
	switch (current_token->kind)
	{
	case COMMA_tok:
		fprintf(parser_output_file, "Rule {EXPR_LIST' -> , EXPR EXPR_LIST'}\n");
		if(!match(COMMA_tok))
			return;
		parse_EXPR();
		parse_EXPR_LIST_TAG();
		break;
	case BRACKET_CLOSE_tok:
		fprintf(parser_output_file, "Rule {EXPR_LIST' -> Epsilon}\n");
		back_token();
		break;
	default:
		error();
		break;
	}
}
void parse_CONDITION() {
	eTOKENS expected_tokens[] = { LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok,
											GREATER_EQUAL_tok, NOT_EQUAL_tok };
	fprintf(parser_output_file, "Rule {CONDITION -> EXPR rel_op EXPR}\n");
	parse_EXPR();
	current_token = next_token();
	if ((current_token->kind == LESS_tok) || (current_token->kind == LESS_EQUAL_tok) ||
		(current_token->kind == EQUAL_tok) || (current_token == GREATER_tok) ||
		(current_token == GREATER_EQUAL_tok) || (current_token->kind == NOT_EQUAL_tok))
		parse_EXPR();
	else
	{
		expected_token_types = expected_tokens;
		expected_token_types_size = 6;
		error();
	}
}
void parse_EXPR() {
	fprintf(parser_output_file, "Rule {EXPR -> TERM EXPR'}\n");
	parse_TERM();
	parse_EXPR_TAG();
}
void parse_EXPR_TAG() {
	// Follow of EXPR' - ; } , ) ] rel_op
	// rel_op -  <  <=  ==  >=  >  != 
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok };
	current_follow = follow;
	current_follow_size = 11;
	eTOKENS expected_tokens[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok, ADD_OP_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 12;
	current_token = next_token();
	switch (current_token->kind) {
	case ADD_OP_tok:
		fprintf(parser_output_file, "Rule {EXPR' -> + TERM EXPR'}\n");
		parse_TERM();
		parse_EXPR_TAG();
		break;
	case SEMICOLON_tok:
	case CURLY_BRACKET_CLOSE_tok:
	case COMMA_tok:
	case PARENTHESIS_CLOSE_tok:
	case BRACKET_CLOSE_tok:
	case LESS_tok:
	case LESS_EQUAL_tok:
	case EQUAL_tok:
	case GREATER_tok:
	case GREATER_EQUAL_tok:
	case NOT_EQUAL_tok:
		fprintf(parser_output_file, "Rule {EXPR' -> Epsilon}\n");
		back_token();
		break;
	 default:
		error();
		break;
	}
}
void parse_TERM() {
	fprintf(parser_output_file, "Rule {TERM -> FACTOR TERM'}\n");
	parse_FACTOR();
	parse_TERM_TAG();
}
void parse_TERM_TAG() {
	// Follow of TERM' - ; } , ) ] rel_op +
	// rel_op -  <  <=  ==  >=  >  != 
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok, ADD_OP_tok };
	current_follow = follow;
	current_follow_size = 12;
	eTOKENS expected_tokens[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok, ADD_OP_tok,
						 MUL_OP_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 13;
	current_token = next_token();
	switch (current_token->kind) {
	case MUL_OP_tok:
		fprintf(parser_output_file, "Rule {TERM' ->  * FACTOR TERM'}\n");
		parse_FACTOR();
		parse_TERM_TAG();
		break;
	case SEMICOLON_tok:
	case CURLY_BRACKET_CLOSE_tok:
	case COMMA_tok:
	case PARENTHESIS_CLOSE_tok:
	case BRACKET_CLOSE_tok:
	case LESS_tok:
	case LESS_EQUAL_tok:
	case EQUAL_tok:
	case GREATER_tok:
	case GREATER_EQUAL_tok:
	case NOT_EQUAL_tok:
	case ADD_OP_tok:
		fprintf(parser_output_file, "Rule {TERM' ->  Epsilon}\n");
		back_token();
		break;
	default:
		error();
		break;
	}
}
void parse_FACTOR() {
	// First of FACTOR - id int_num float_num (
	// Follow of FACTOR - ; } , ) ] rel_op + *
	// rel_op -  <  <=  ==  >=  >  != 
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok, ADD_OP_tok, MUL_OP_tok};
	current_follow = follow;
	current_follow_size = 13;
	eTOKENS expected_tokens[] = { ID_tok, INT_NUM_tok, FLOAT_NUM_tok, PARENTHESIS_OPEN_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 4;
	current_token = next_token();
	switch (current_token->kind) {
	case ID_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> id VAR_OR_CALL'}\n");
		parse_VAR_OR_CALL_TAG();
		break;
	case INT_NUM_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> int_num}\n");
		break;
	case FLOAT_NUM_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> float_num}\n");
		break;
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> (EXPR)}\n");
		parse_EXPR();
		if(!match(PARENTHESIS_CLOSE_tok))
			return;
		break;
	default:
		error();
		break;
	}
}
void parse_VAR_OR_CALL_TAG() {
	// Follow of VAR_OR_CALL' - ; } , ) ] rel_op + *
	// rel_op -  <  <=  ==  >=  >  != 
	// First of VAR' - [
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok, ADD_OP_tok,
						 MUL_OP_tok };
	current_follow = follow;
	current_follow_size = 13;
	eTOKENS expected_tokens[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok, ADD_OP_tok,
						 MUL_OP_tok, PARENTHESIS_OPEN_tok, BRACKET_OPEN_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 15;
	current_token = next_token();
	switch (current_token->kind) {
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL' -> (ARGS)}\n");
		parse_ARGS();
		if(!match(PARENTHESIS_CLOSE_tok))
			return;
		break;
	case SEMICOLON_tok:
	case CURLY_BRACKET_CLOSE_tok:
	case COMMA_tok:
	case PARENTHESIS_CLOSE_tok:
	case BRACKET_CLOSE_tok:
	case LESS_tok:
	case LESS_EQUAL_tok:
	case EQUAL_tok:
	case GREATER_tok:
	case GREATER_EQUAL_tok:
	case NOT_EQUAL_tok:
	case ADD_OP_tok:
	case MUL_OP_tok:
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL' -> VAR'}\n");
		back_token();
		parse_VAR_TAG();
		break;
	default:
		error();
		break;
	}
}