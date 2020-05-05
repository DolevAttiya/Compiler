#include "../Lexical Analyzer/Token/Token.h"
#include "Parser.h"

Token* current_token;
eTOKENS* current_follow;
eTOKENS expected_token_type;

int parse()
{
	parse_PROG();
}

int parse_PROG()
{
	eTOKENS follow[] = { EOF_tok };
	current_follow = follow;
	parse_GLOBAL_VARS();
	do {
		for(int i=0;i<3;i++) 
			current_token=next_token();
		if (current_token->kind != PARENTHESIS_OPEN_tok)
			for (int i = 0; i < 3; i++)
				back_token();
			parse_GLOBAL_VARS();
	} while (current_token->kind != PARENTHESIS_OPEN_tok);
	for (int i = 0; i < 3; i++)
		back_token();

	do {
		parse_FUNC_PROTOTYPE();
		current_token = next_token();
	} while (current_token->kind == SEMICOLON_tok);

	if (!match(BRACKET_OPEN_tok))
		return 0;

	back_token();       
	parse_COMP_STMT();
	current_token = next_token();
	if (current_token->kind != EOF_tok)
	{
		back_token();    
		parse_FUNC_FULL_DEFS();
	}
}

int parse_GLOBAL_VARS()
{
	current_token = next_token();
	eTOKENS follow[] = { INT_tok, FLOAT_tok, VOID_tok};
	current_follow = follow;
	switch (current_token->kind)
	{
	case INT_tok:
	case FLOAT_tok:
		back_token();
		parse_VAR_DEC();
		break;
	default:
		error(expected_token_type);
		break;
	}
}

int parse_VAR_DEC()
{
	eTOKENS follow[] = { INT_tok, FLOAT_tok, VOID_tok, ID_tok, CURLY_BRACKET_OPEN_tok, IF_tok, RETURN_tok };
	current_follow = follow;
	parse_TYPE();
	if (!match(ID_tok))
		return 0;
	parse_VAR_DEC_TAG();	
}

int parse_VAR_DEC_TAG()
{
	eTOKENS follow[] = { INT_tok, FLOAT_tok, VOID_tok, ID_tok, CURLY_BRACKET_OPEN_tok, IF_tok, RETURN_tok };
	current_follow = follow;
	current_token = next_token();
	switch (current_token->kind)
	{
	case SEMICOLON_tok: 
		break;
	case BRACKET_OPEN_tok:
		parse_DIM_SIZES();
		if (!match(BRACKET_CLOSE_tok))
			return 0;
		if (!match(SEMICOLON_tok))
			return 0;
		break;
	default:
		error(expected_token_type);
		break;
	}
}

int parse_TYPE()
{
	eTOKENS follow[] = { ID_tok };
	current_follow = follow;
	current_token = next_token();
	switch (current_token->kind)
	{
	case INT_tok:
		break;
	case FLOAT_tok:
		break;
	default:
		error(expected_token_type);
		break;
	}
}

int parse_DIM_SIZES()
{
	eTOKENS follow[] = { BRACKET_CLOSE_tok };
	current_follow = follow;
	if (!match(INT_NUM_tok))
		return 0;
	parse_DIM_SIZES_TAG();
}

int parse_DIM_SIZES_TAG()
{
	eTOKENS follow[] = { BRACKET_CLOSE_tok };
	current_follow = follow;
	current_token = next_token();
	switch (current_token->kind)
	{
	case COMMA_tok:
		parse_DIM_SIZES();
		break;
	case BRACKET_CLOSE_tok:
		back_token();
		break;
	default:
		error(expected_token_type);
		break;
	}
}

int parse_FUNC_PROTOTYPE()
{
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_OPEN_tok };
	current_follow = follow;
	parse_RETURN_TYPE();
	if (!match(ID_tok))
		return 0;
	if (!match(PARENTHESIS_OPEN_tok))
		return 0;
	parse_PARAMS();
	if (!match(PARENTHESIS_CLOSE_tok))
		return 0;
}

int parse_FUNC_FULL_DEFS()
{
	parse_FUNC_WITH_BODY();
	parse_FUNC_FULL_DEFS_TAG();
}

int parse_FUNC_FULL_DEFS_TAG()
{
	eTOKENS follow[] = { EOF_tok };
	current_follow = follow;
	current_token = next_token();
	switch (current_token->kind)
	{
	case INT_tok:
	case FLOAT_tok:
	case VOID_tok:
		back_token();
		parse_FUNC_FULL_DEFS();
		break;
	case EOF_tok:
		back_token();
		break;
	default:
		error(expected_token_type);
		break;
	}
}

int parse_FUNC_WITH_BODY()
{
	parse_FUNC_PROTOTYPE();
	parse_COMP_STMT();
}