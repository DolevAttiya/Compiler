#pragma once
#include "../Lexical Analyzer/Token/Token.h"
int match(eTOKENS token_type);
int error(eTOKENS expected_token_type);
int parse();
int parse_PROG();
int parse_GLOBAL_VARS();
int parse_VAR_DEC();
int parse_VAR_DEC_TAG();
int parse_TYPE();
int parse_DIM_SIZES();
int parse_DIM_SIZES_TAG();
int parse_FUNC_PREDEFS();
int parse_FUNC_PREDEFS_TAG();
int parse_FUNC_PROTOTYPE();
int parse_FUNC_FULL_DEFS();
int parse_FUNC_FULL_DEFS_TAG();
int parse_FUNC_WITH_BODY();