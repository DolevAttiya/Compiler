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
	parse_GLOBAL_VARS();
	do {
		for(int i=0;i<3;i++) 
			current_token=next_token();
		if (current_token->kind != PARENTHESIS_OPEN_tok)
			parse_GLOBAL_VARS();
		for (int i = 0; i < 3; i++)
			back_token();
	} while (current_token->kind != PARENTHESIS_OPEN_tok);
	parse_FUNC_PREDEFS();
	parse_FUNC_FULL_DEFS();
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
	if (match(ID_tok))
		parse_VAR_DEC_TAG();
	else
		return 0;
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
		match(BRACKET_CLOSE_tok);
		match(SEMICOLON_tok);
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
	match(INT_NUM_tok);
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
		back_token();
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

int parse_FUNC_PREDEFS()
{
	eTOKENS follow[] = { INT_tok, FLOAT_tok, VOID_tok };
	current_follow = follow;
	parse_FUNC_PROTOTYPE();
	match(SEMICOLON_tok);
	parse_FUNC_FULL_DEFS_TAG();
}

int parse_FUNC_PREDEFS_TAG()
{

	parse_FUNC_PROTOTYPE();
	match(SEMICOLON_tok);

}

int parse_FUNC_PROTOTYPE()
{

}

int parse_FUNC_FULL_DEFS()
{

}

int parse_FUNC_FULL_DEFS_TAG()
{

}

int parse_FUNC_WITH_BODY()
{

}

int match(eTOKENS token_type) 
{

}

int error(eTOKENS expected_token_type)
{

}