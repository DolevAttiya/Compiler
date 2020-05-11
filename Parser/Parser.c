#include "../Lexical Analyzer/Token/Token.h"
#include "Parser.h"

Token* current_token;
eTOKENS* current_follow;
int current_follow_size;
eTOKENS expected_token_type;
FILE* parser_output_file;


int parse_Follow()
{
	int flag = 0;
	for (int i = 0; i < current_follow_size; i++)
	{
		flag += current_token->kind == current_follow[i];
	}
	return flag;
}
void error(eTOKENS expected_token_type) {
	fprintf(parser_output_file, "Expected token of type '{%d}' at line: {%d},  Actual token of type '{%d}', lexeme: '{%c}'.", expected_token_type, current_token->lineNumber, current_token->kind, current_token->lexeme);
	do
	{
		current_token = next_token();
	} while (parse_Follow() == 0 && current_token -> kind != EOF_tok);
	back_token();

}