#include "../Lexical Analyzer/Token/Token.h"
#include "Parser.h"

Token* current_token;
eTOKENS* current_follow;
eTOKENS expected_token_type;
FILE* parser_output_file;