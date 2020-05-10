#include "../Lexical Analyzer/Token/Token.h"
#include "Parser.h"

Token* current_token;
eTOKENS* current_follow;
eTOKENS expected_token_type;
FILE* parser_output_file;

int match(eTOKENS expected_token) {
	current_token = next_token();
	if (current_token->kind != expected_token)
	{
		error(expected_token);
		return 0;
	}
	return 1;
}
