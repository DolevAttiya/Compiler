#pragma once
#include "../Lexical Analyzer/Token/Token.h"
#include "Semantic functions.h"

FILE* parser_output_file;
int match(eTOKENS expected_token);
void error();

int parse_Follow();
char* get_tokens_names();

void parser();
void parse_PROG();
void parse_GLOBAL_VARS();
//void parse_GLOBAL_VARS_TAG();
void parse_VAR_DEC();
void parse_VAR_DEC_TAG(Type* type, ListNode** DimensionsList);
Type parse_TYPE();
void parse_DIM_SIZES(ListNode** DimensionsList);
void parse_DIM_SIZES_TAG(ListNode** DimensionsList);
//void parse_FUNC_PREDEFS();
//void parse_FUNC_PREDEFS_TAG();
void parse_FUNC_PROTOTYPE(char** function_name, Type* function_type, ListNode** ParametersList, Role role_for_parameters_parser);
void parse_FUNC_FULL_DEFS();
void parse_FUNC_FULL_DEFS_TAG();
void parse_FUNC_WITH_BODY();

Type parse_RETURN_TYPE();
ListNode* parse_PARAMS(Role role_for_parameters_parser);
ListNode* parse_PARAM_LIST();
void parse_PARAM_LIST_TAG(ListNode* Head);
Type parse_PARAM();
void parse_PARAM_TAG(Type* param_type, ListNode** dimList);
void parse_COMP_STMT();
void parse_VAR_DEC_LIST();
void parse_VAR_DEC_LIST_TAG();
void parse_STMT_LIST();
void parse_STMT_LIST_TAG();
void parse_STMT();
void parse_VAR_OR_CALL();
void parse_IF_STMT();
void parse_ARGS();

void parse_ARG_LIST();
void parse_ARG_LIST_TAG();
void parse_RETURN_STMT();
void parse_RETURN_STMT_TAG();
void parse_VAR_TAG();
void parse_EXPR_LIST();
void parse_EXPR_LIST_TAG();
void parse_CONDITION();
void parse_EXPR();
void parse_EXPR_TAG();
void parse_TERM();
void parse_TERM_TAG();
void parse_FACTOR();
void parse_VAR_OR_CALL_TAG();

void parse_BB();
void parse_FB();