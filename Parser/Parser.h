#pragma once
#include "../Lexical Analyzer/Token/Token.h"

FILE* parser_output_file;
int match(eTOKENS token_type);
int error(eTOKENS expected_token_type);
int parse_Follow();

int parse();
int parse_PROG();
int parse_GLOBAL_VARS();
//int parse_GLOBAL_VARS_TAG();
int parse_VAR_DEC();
int parse_VAR_DEC_TAG();
int parse_TYPE();
int parse_DIM_SIZES();
int parse_DIM_SIZES_TAG();
//int parse_FUNC_PREDEFS();
//int parse_FUNC_PREDEFS_TAG();
int parse_FUNC_PROTOTYPE();
int parse_FUNC_FULL_DEFS();
int parse_FUNC_FULL_DEFS_TAG();
int parse_FUNC_WITH_BODY();

int parse_RETURN_TYPE();
int parse_PARAMS();
int parse_PARAM_LIST();
int parse_PARAM_LIST_TAG();
int parse_PARAM();
int parse_PARAM_TAG();
int parse_COMP_STMT();
int parse_VAR_DEC_LIST();
int parse_VAR_DEC_LIST_TAG();
int parse_STMT_LIST();
int parse_STMT_LIST_TAG();
int parse_STMT();
int parse_VAR_OR_CALL();
int parse_IF_STMT();
int parse_ARGS();

int parse_RETURN_STMT();
int parse_VAR_TAG();
int parse_EXPR();
int parse_CONDITION();
int parse_ARG_LIST();
