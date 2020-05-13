#pragma once
#include "../Lexical Analyzer/Token/Token.h"

FILE* parser_output_file;
int match(eTOKENS token_type);
void error();
int parse_Follow();

void parser();
void parse_PROG();
void parse_GLOBAL_VARS();
//void parse_GLOBAL_VARS_TAG();
void parse_VAR_DEC();
void parse_VAR_DEC_TAG();
void parse_TYPE();
void parse_DIM_SIZES();
void parse_DIM_SIZES_TAG();
//void parse_FUNC_PREDEFS();
//void parse_FUNC_PREDEFS_TAG();
void parse_FUNC_PROTOTYPE();
void parse_FUNC_FULL_DEFS();
void parse_FUNC_FULL_DEFS_TAG();
void parse_FUNC_WITH_BODY();

void parse_COMP_STMT(); //Didn't implement
void parse_RETURN_TYPE(); //Didn't implement
void parse_PARAMS(); //Didn't implement
