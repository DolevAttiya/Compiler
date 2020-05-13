#include "../Lexical Analyzer/Token/Token.h"
#include "Parser.h"

Token* current_token;
eTOKENS* current_follow;
int current_follow_size;
eTOKENS* expected_token_types;
int expected_token_types_size;
FILE* parser_output_file;

void parser()
{
	parse_PROG();
}

void parse_PROG()
{
	eTOKENS follow[] = { EOF_tok };
	current_follow = follow;
	current_follow_size = 1;
	fprintf(parser_output_file, "Rule {PROG -> GLOBAL_VARS FUNC_PREDEFS FUNC_FULL_DEFS}");
	fprintf(parser_output_file, "Rule {GLOBAL_VARS -> VAR_DEC GLOBAL_VARS'}");
	parse_GLOBAL_VARS();
	do {
		for(int i=0;i<3;i++) 
			current_token=next_token();
		if (current_token->kind != PARENTHESIS_OPEN_tok)
		{
			for (int i = 0; i < 3; i++)
				back_token();
			fprintf(parser_output_file, "Rule {GLOBAL_VARS' -> VAR_DEC GLOBAL_VARS'}");
			parse_GLOBAL_VARS();
		}
	} while (current_token->kind != PARENTHESIS_OPEN_tok);

	fprintf(parser_output_file, "Rule {GLOBAL_VARS' -> epsilon}");

	for (int i = 0; i < 3; i++)
		back_token();

	fprintf(parser_output_file, "Rule {FUNC_PREDEFS -> FUNC_PROTYTYPE; FUNC_PREDEFS'}");
	do {		
		parse_FUNC_PROTOTYPE();
		current_token = next_token();
		if (current_token->kind == SEMICOLON_tok)
			fprintf(parser_output_file, "Rule {FUNC_PREDEFS' -> FUNC_PROTYTYPE; FUNC_PREDEFS'}");
	} while (current_token->kind == SEMICOLON_tok);

	fprintf(parser_output_file, "Rule {FUNC_PREDEFS' -> epsilon}");

	fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS -> FUNC_WITH_BODY FUNC_FULL_DEFS'}");
	fprintf(parser_output_file, "Rule {FUNC_WITH_BODY -> FUNC_PROTOTYPE COMP_STMT}");
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

	fprintf(parser_output_file, "Rule {VAR_DEC -> TYPE id VAR_DEC'}");
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
		fprintf(parser_output_file, "Rule {VAR_DEC' -> ;}");
		break;
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_DEC' -> [DIM_SIZES] ;}");
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
		fprintf(parser_output_file, "Rule {TYPE -> int}");
		break;
	case FLOAT_tok:
		fprintf(parser_output_file, "Rule {TYPE -> float}");
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
	fprintf(parser_output_file, "Rule {DIM_SIZES -> int_num DIM_SIZES'}");
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
		fprintf(parser_output_file, "Rule {DIM_SIZES' -> , DIM_SIZES}");
		parse_DIM_SIZES();
		break;
	case BRACKET_CLOSE_tok:
		fprintf(parser_output_file, "Rule {DIM_SIZES' -> epsilon}");
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
	fprintf(parser_output_file, "Rule {FUNC_PROTOTYPE -> RETURN_TYPE id (PARAMS)}");
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
	fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS -> FUNC_WITH_BODY FUNC_FULL_DEFS'}");
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
	expected_token_types_size = 3;

	switch (current_token->kind)
	{
	case INT_tok:
	case FLOAT_tok:
	case VOID_tok:
		fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS' -> FUNC_FULL_DEFS}");
		back_token();
		parse_FUNC_FULL_DEFS();
		break;
	case EOF_tok:
		fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS' -> epsilon}");
		back_token();
		break;
	default:
		error();
		break;
	}
}

void parse_FUNC_WITH_BODY()
{
	fprintf(parser_output_file, "Rule {FUNC_WITH_BODY -> FUNC_PROTOTYPE COMP_STMT}");
	parse_FUNC_PROTOTYPE();
	parse_COMP_STMT();
}
