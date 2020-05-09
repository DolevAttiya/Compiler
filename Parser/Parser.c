#include "../Lexical Analyzer/Token/Token.h"
#include "Parser.h"

Token* current_token;
eTOKENS* current_follow;
int current_follow_size;
eTOKENS expected_token_type;
FILE* parser_output_file;

int error(eTOKENS expected_token_type) {
	fprintf(parser_output_file, "Expected token of type '{%s}' at line: {%d},  Actual token of type '{%s}', lexeme: '{%s}'.", expected_token_type, current_token->lineNumber, current_token->kind, current_token->lexeme);
	do
	{
		current_token = next_token();
	} while (parse_Follow() != 0 || current_token == EOF_tok);
	back_token();

}