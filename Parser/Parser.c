#include "../Lexical Analyzer/Token/Token.h"
char* eTokensStrings[];
#include "Parser.h"

Token* current_token;
eTOKENS* current_follow;
int current_follow_size;
eTOKENS* expected_token_types;
int expected_token_types_size;

int buffer_size=0;
char* temp_buffer=NULL;
int parser_output_file_last_position;
FILE* parser_output_file;

void parser()
{
	//AssafTest();
	parse_PROG();
	clean_token_storage();
}

void parse_PROG()
{
	int steps = 0;
	eTOKENS follow[] = { EOF_tok };
	current_follow = follow;
	current_follow_size = 1;
	fprintf(parser_output_file, "Rule {PROG -> GLOBAL_VARS FUNC_PREDEFS FUNC_FULL_DEFS}\n");

	fprintf(parser_output_file, "Rule {GLOBAL_VARS -> VAR_DEC GLOBAL_VARS'}\n");
	parse_GLOBAL_VARS();
	do {
		fprintf(parser_output_file, "Rule {GLOBAL_VARS' -> VAR_DEC GLOBAL_VARS' | epsilon}\n");
	
		current_token = next_token(); //Checks if first token is 'void'
		if (current_token->kind == VOID_tok || current_token->kind == EOF_tok)
		{
			fprintf(parser_output_file, "Rule {GLOBAL_VARS' -> epsilon}\n");
			back_token();
			break;
		}
		back_token();

		for (int i = 0; i < 3 && current_token->kind != EOF_tok; i++, steps++) // Lookahead for '(' (Meaning it's a function predefinition)
			current_token = next_token();
		if (current_token->kind == PARENTHESIS_OPEN_tok || current_token->kind == EOF_tok)
		{
			fprintf(parser_output_file, "Rule {GLOBAL_VARS' -> epsilon}\n");
			for (int i = 0; i < steps; i++)
				back_token();
			break;
		}
		for (int i = 0; i < steps; i++)
			back_token();
		steps = 0;

		fprintf(parser_output_file, "Rule {GLOBAL_VARS' -> VAR_DEC GLOBAL_VARS'}\n"); //Continues regular parsing
		eTOKENS follow[] = { INT_tok, FLOAT_tok, VOID_tok };
		current_follow = follow;
		current_follow_size = 3;
		eTOKENS tokens[] = { INT_tok, FLOAT_tok };
		expected_token_types = tokens;
		expected_token_types_size = 2;
		current_token = next_token();
		if (current_token->kind == INT_tok || current_token->kind == FLOAT_tok)
		{
			back_token();
			parse_VAR_DEC();
		}
		else
		{
			error();
			//break
		}

	} while(1);

	eTOKENS follow2[] = { INT_tok, FLOAT_tok, VOID_tok };
	current_follow = follow2;
	current_follow_size = 3;
	fprintf(parser_output_file, "Rule {FUNC_PREDEFS -> FUNC_PROTYTYPE; FUNC_PREDEFS'}\n");
 	parse_FUNC_PROTOTYPE();
	current_follow = follow2;
	current_follow_size = 3;
	if (match(SEMICOLON_tok))
	{
		do {
			fprintf(parser_output_file, "Rule {FUNC_PREDEFS' -> FUNC_PROTYTYPE; FUNC_PREDEFS' | epsilon}\n");
			parser_output_file_last_position = ftell(parser_output_file);//save file seeker location
			parse_FUNC_PROTOTYPE();
			current_token = next_token();
		} while (current_token->kind == SEMICOLON_tok);

		fseek(parser_output_file, parser_output_file_last_position, SEEK_SET); //Returns file position to before the last parse_FUNC_PROTOTYPE
		while (fgetc(parser_output_file) != EOF)
			buffer_size++;
		temp_buffer = (char*)malloc(buffer_size + 1);
		fseek(parser_output_file, parser_output_file_last_position, SEEK_SET); //Returns file position to before the last parse_FUNC_PROTOTYPE
		fread(temp_buffer, 1, buffer_size, parser_output_file); //read from file seek location and save to a defined buffer
		temp_buffer[buffer_size] = 0;
		fseek(parser_output_file, parser_output_file_last_position, SEEK_SET); //Reset file position to the last position
		buffer_size = 0;
		back_token();
		fprintf(parser_output_file, "Rule {FUNC_PREDEFS' -> epsilon}\n");
	}
	else
	{
		parser_output_file_last_position = ftell(parser_output_file);//save file seeker location
		parse_FUNC_PROTOTYPE();
		fseek(parser_output_file, parser_output_file_last_position, SEEK_SET); //Returns file position to before the last parse_FUNC_PROTOTYPE
		while (fgetc(parser_output_file) != EOF)
			buffer_size++;
		temp_buffer = (char*)malloc(buffer_size + 1);
		fseek(parser_output_file, parser_output_file_last_position, SEEK_SET); //Returns file position to before the last parse_FUNC_PROTOTYPE
		fread(temp_buffer, 1, buffer_size, parser_output_file); //read from file seek location and save to a defined buffer
		temp_buffer[buffer_size] = 0;
		fseek(parser_output_file, parser_output_file_last_position, SEEK_SET);
		buffer_size = 0;
	}

	fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS -> FUNC_WITH_BODY FUNC_FULL_DEFS'}\n");
	fprintf(parser_output_file, "Rule {FUNC_WITH_BODY -> FUNC_PROTOTYPE COMP_STMT}\n");  
	if(temp_buffer!=NULL)
		fprintf(parser_output_file, temp_buffer); //Append to output file
		free(temp_buffer);
	temp_buffer = NULL;
	parse_COMP_STMT();
	fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS' -> FUNC_FULL_DEFS | epsilon}\n");
	current_token = next_token();
	if (current_token->kind != EOF_tok)
	{
		fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS' -> FUNC_FULL_DEFS}\n");
		back_token();    
		parse_FUNC_FULL_DEFS();
	}
	else
	{
		fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS' -> epsilon}\n");
	}
}

void parse_GLOBAL_VARS()
{
	current_token = next_token();
	eTOKENS follow[] = { INT_tok, FLOAT_tok, VOID_tok};
	current_follow = follow;
	current_follow_size = 3;
	eTOKENS tokens[] = { INT_tok, FLOAT_tok };
	expected_token_types = tokens;
	expected_token_types_size = 2;
	switch (current_token->kind)
	{
	case INT_tok:
	case FLOAT_tok:
		back_token();
		parse_VAR_DEC();
		break;
	default:
		error();
		break;
	}
}

void parse_VAR_DEC()
{
	eTOKENS follow[] = { INT_tok, FLOAT_tok, VOID_tok, ID_tok, CURLY_BRACKET_OPEN_tok, IF_tok, RETURN_tok };
	current_follow = follow;
	current_follow_size = 7;

	fprintf(parser_output_file, "Rule {VAR_DEC -> TYPE id VAR_DEC'}\n");
	parse_TYPE();
	current_follow = follow;
	current_follow_size = 7;
	if (!match(ID_tok))
		return;
	parse_VAR_DEC_TAG();	
}

void parse_VAR_DEC_TAG()
{
	eTOKENS follow[] = { INT_tok, FLOAT_tok, VOID_tok, ID_tok, CURLY_BRACKET_OPEN_tok, IF_tok, RETURN_tok };
	current_follow = follow;
	current_follow_size = 7;
	current_token = next_token();
	eTOKENS tokens[] = { SEMICOLON_tok, BRACKET_OPEN_tok };
	expected_token_types = tokens;
	expected_token_types_size = 2;

	fprintf(parser_output_file, "Rule {VAR_DEC' -> ; | [DIM_SIZES] ;}\n");

	switch (current_token->kind)
	{
	case SEMICOLON_tok: 
		fprintf(parser_output_file, "Rule {VAR_DEC' -> ;}\n");
		break;
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_DEC' -> [DIM_SIZES] ;}\n");
		parse_DIM_SIZES();
		current_follow = follow;
		current_follow_size = 7;
		if (!match(BRACKET_CLOSE_tok))
			return;
		if (!match(SEMICOLON_tok))
			return;
		break;
	default:
		error();
		break;
	}
}

Type parse_TYPE()
{
	eTOKENS follow[] = { ID_tok };
	current_follow = follow;
	current_follow_size = 1;
	current_token = next_token();
	eTOKENS tokens[] = { INT_tok, FLOAT_tok };
	expected_token_types = tokens;
	expected_token_types_size = 2;

	fprintf(parser_output_file, "Rule {TYPE -> int | float}\n");

	switch (current_token->kind)
	{
	case INT_tok:
		fprintf(parser_output_file, "Rule {TYPE -> int}\n");
		break;
	case FLOAT_tok:
		fprintf(parser_output_file, "Rule {TYPE -> float}\n");
		break;
	default:
		error();
		break;
	}
}

ListNode* parse_DIM_SIZES()
{
	eTOKENS follow[] = { BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	fprintf(parser_output_file, "Rule {DIM_SIZES -> int_num DIM_SIZES'}\n");
	if (!match(INT_NUM_tok))
		return;
	parse_DIM_SIZES_TAG();
}

void parse_DIM_SIZES_TAG()
{
	eTOKENS follow[] = { BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	current_token = next_token();
	eTOKENS tokens[] = { COMMA_tok, BRACKET_CLOSE_tok };
	expected_token_types = tokens;
	expected_token_types_size = 2;

	fprintf(parser_output_file, "Rule {DIM_SIZES' -> , DIM_SIZES | epsilon}\n");

	switch (current_token->kind)
	{
	case COMMA_tok:
		fprintf(parser_output_file, "Rule {DIM_SIZES' -> , DIM_SIZES}\n");
		parse_DIM_SIZES();
		break;
	case BRACKET_CLOSE_tok:
		fprintf(parser_output_file, "Rule {DIM_SIZES' -> epsilon}\n");
		back_token();
		break;
	default:
		error();
		break;
	}
}

void parse_FUNC_PROTOTYPE()
{
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_OPEN_tok };
	current_follow = follow;
	current_follow_size = 2;
	fprintf(parser_output_file, "Rule {FUNC_PROTOTYPE -> RETURN_TYPE id (PARAMS)}\n");
	parse_RETURN_TYPE();
	current_follow = follow;
	current_follow_size = 2;
	if (!match(ID_tok))
		return;
	if (!match(PARENTHESIS_OPEN_tok))
		return;
	parse_PARAMS();
	current_follow = follow;
	current_follow_size = 2;
	if (!match(PARENTHESIS_CLOSE_tok))
		return;
}

void parse_FUNC_FULL_DEFS()
{
	fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS -> FUNC_WITH_BODY FUNC_FULL_DEFS'}\n");
	parse_FUNC_WITH_BODY();
	parse_FUNC_FULL_DEFS_TAG();
}

void parse_FUNC_FULL_DEFS_TAG()
{
	eTOKENS follow[] = { EOF_tok };
	current_follow = follow;
	current_follow_size = 1;
	current_token = next_token();
	eTOKENS tokens[] = { INT_tok, FLOAT_tok, VOID_tok, EOF_tok };
	expected_token_types = tokens;
	expected_token_types_size = 4;

	fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS' -> FUNC_FULL_DEFS | epsilon}\n");

	switch (current_token->kind)
	{
	case INT_tok:
	case FLOAT_tok:
	case VOID_tok:
		fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS' -> FUNC_FULL_DEFS}\n");
		back_token();
		parse_FUNC_FULL_DEFS();
		break;
	case EOF_tok:
		fprintf(parser_output_file, "Rule {FUNC_FULL_DEFS' -> epsilon}\n");
		back_token();
		break;
	default:
		error();
		break;
	}
}

void parse_FUNC_WITH_BODY()
{
	fprintf(parser_output_file, "Rule {FUNC_WITH_BODY -> FUNC_PROTOTYPE COMP_STMT}\n");
	parse_FUNC_PROTOTYPE();
	parse_COMP_STMT();
}


Type parse_RETURN_TYPE() {
	eTOKENS follow[] = { ID_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS tokens[] = {/*Firsts*/ INT_tok, FLOAT_tok, VOID_tok};
	expected_token_types = tokens;
	expected_token_types_size = 3;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {RETURN_TYPE -> TYPE | void}\n");

	switch (current_token->kind)
	{
	case INT_tok:
	case FLOAT_tok:
		fprintf(parser_output_file, "Rule {RETURN_TYPE -> TYPE}\n");
		back_token();
		return parse_TYPE();
	case VOID_tok:
		fprintf(parser_output_file, "Rule {RETURN_TYPE -> void}\n");
		back_token();
		if (!match(VOID_tok))
			return ErrorType;
		return Void;
	default:
		error();
		return ErrorType;
	}
}
/*return List of Params types */
ListNode* parse_PARAMS() {
	
	eTOKENS follow[] = { PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS tokens[] = {/*Firsts*/ INT_tok, FLOAT_tok,/*Follows*/ PARENTHESIS_CLOSE_tok };
	expected_token_types = tokens;
	expected_token_types_size = 3;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {PARAMS -> PARAMS_LIST | epsilon}\n");

	switch (current_token->kind)
	{
	case INT_tok:
	case FLOAT_tok:
		fprintf(parser_output_file, "Rule {PARAMS -> PARAMS_LIST}\n");
		back_token();
		/*Semantic*/
		ListNode* to_check = parse_PARAM_LIST();
		if (!search_type_error(to_check))
			return to_check;
		else
			return NULL;
		/*Semantic*/
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {PARAMS -> epsilon}\n");
			back_token();
			/*Semantic*/
			return (ListNode*) calloc(1,sizeof(ListNode));
			/*Semantic*/
		}
		error();
		/*Semantic*/
		return NULL;
		/*Semantic*/
	}
}

ListNode* parse_PARAM_LIST() {
	fprintf(parser_output_file, "Rule {PARAMS_LIST -> PARAM PARAMS_LIST'}\n");
	/*Semantic*/
	ListNode* Head = NULL;
	add_type_to_list_node(Head, parse_PARAM());
	parse_PARAM_LIST_TAG(Head);
	return Head;
	/*Semantic*/
}

void parse_PARAM_LIST_TAG(ListNode* Head) {
	eTOKENS follow[] = { PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS tokens[] = {/*Firsts*/ COMMA_tok,/*Follows*/ PARENTHESIS_CLOSE_tok};
	expected_token_types = tokens;
	expected_token_types_size = 2;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {PARAMS_LIST' -> , PARAM PARAMS_LIST' | epsilon}\n");

	switch (current_token->kind)
	{
	case COMMA_tok:
		fprintf(parser_output_file, "Rule {PARAMS_LIST' -> , PARAM PARAMS_LIST'}\n");
		/*Semantic*/
		add_type_to_list_node(Head, parse_PARAM());
		/*Semantic*/
		parse_PARAM_LIST_TAG(Head);
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {PARAMS_LIST' -> epsilon}\n");
			back_token();
			break;
		}
		error();
		break;
	}
}

Type parse_PARAM() {
	eTOKENS follow[] = { COMMA_tok, PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	fprintf(parser_output_file, "Rule {PARAM -> TYPE id PARAM'}\n");
	/*Semantic*/
	Type param_type = parse_TYPE();
	ListNode* dimList = NULL;
	/*Semantic*/
	current_follow = follow;
	current_follow_size = 2;
	if (!match(ID_tok))//need to add to the symbol table
		return NULL;// if match didn't work does the type is error_type ?
	parse_PARAM_TAG(&param_type,&dimList);
	/*Semantic*/
	if (param_type != ErrorType && dimList!= NULL)
	{
		table_entry id = insert(current_token->lexeme);
		set_id_role(id, Variable);
		set_list(id, dimList);
		set_id_type(id, param_type);
		return get_id_type(id);
	}
	else if (dimList!=NULL)
	{
		free_list(&dimList);
	}
	return ErrorType;
	/*Semantic*/
}

void parse_PARAM_TAG(Type* param_type, ListNode** dimList) {
	eTOKENS follow[] = { COMMA_tok, PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	eTOKENS tokens[] = {/*Firsts*/ BRACKET_OPEN_tok,/*Follows*/ COMMA_tok, PARENTHESIS_CLOSE_tok };
	expected_token_types = tokens;
	expected_token_types_size = 3;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {PARAM' -> [DIM_SIZES] | epsilon}\n");

	switch (current_token->kind)
	{
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {PARAM' -> [DIM_SIZES]}\n");

		/*Semantic*/
		parse_DIM_SIZES(dimList);
		if (*param_type == Integer)
			*param_type = IntArray;
		else
		{
			if (*param_type != ErrorType)
				*param_type = FloatArray;
			else
				return; // TODO so do i delete the ID if he is set to error type ?
		}
		/*Semantic*/
		current_follow = follow;
		current_follow_size = 2;
		if (!match(BRACKET_CLOSE_tok))
			return;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {PARAM' -> epsilon}\n");
			/*Semantic*/
			*dimList = (ListNode*)calloc(1, sizeof(ListNode));
			/*Semantic*/
			back_token();
			break;
		}
		error();
		/*Semantic*/
		*param_type = ErrorType;
		*dimList = NULL;
		/*Semantic*/
		break;
	}
}

void parse_COMP_STMT() {
	eTOKENS follow[] = { INT_tok, FLOAT_tok, VOID_tok, EOF_tok, SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 6;
	fprintf(parser_output_file, "Rule {COMP_STMT -> { VAR_DEC_LIST STMT_LIST }}\n");
	if (!match(CURLY_BRACKET_OPEN_tok))
		return;
	/*Semantic*/
	make_table();
	/*Semantic*/
	parse_VAR_DEC_LIST();
	parse_STMT_LIST();
	current_follow = follow;
	current_follow_size = 6;
	if (!match(CURLY_BRACKET_CLOSE_tok))
		return;
	/*Semantic*/
	pop_table();
	/*Semantic*/
}

void parse_VAR_DEC_LIST() {
	fprintf(parser_output_file, "Rule {VAR_DEC_LIST -> VAR_DEC_LIST'}\n");
	parse_VAR_DEC_LIST_TAG();
}

void parse_VAR_DEC_LIST_TAG() {
	eTOKENS follow[] = { ID_tok, CURLY_BRACKET_OPEN_tok, IF_tok, RETURN_tok };
	current_follow = follow;
	current_follow_size = 4;
	eTOKENS tokens[] = { INT_tok, FLOAT_tok };
	expected_token_types = tokens;
	expected_token_types_size = 2;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {VAR_DEC_LIST' -> VAR_DEC VAR_DEC_LIST' | epsilon}\n");

	switch (current_token->kind)
	{
	case INT_tok:
	case FLOAT_tok:
		fprintf(parser_output_file, "Rule {VAR_DEC_LIST' -> VAR_DEC VAR_DEC_LIST'}\n");
		back_token();
		parse_VAR_DEC();
		parse_VAR_DEC_LIST_TAG();
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {VAR_DEC_LIST' -> epsilon}\n");
			back_token();
			break;
		}
		error();
		break;
	}
}

void parse_STMT_LIST() {
	fprintf(parser_output_file, "Rule {STMT_LIST -> STMT STMT_LIST'}\n");
	parse_STMT();
	parse_STMT_LIST_TAG();
}

void parse_STMT_LIST_TAG() {
	eTOKENS follow[] = { CURLY_BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS tokens[] = {/*Firsts*/SEMICOLON_tok ,/*Follow*/CURLY_BRACKET_CLOSE_tok};
	expected_token_types = tokens;
	expected_token_types_size = 2;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {STMT_LIST' -> ; STMT STMT_LIST' | epsilon}\n");

	switch (current_token->kind)
	{
	case SEMICOLON_tok:
		fprintf(parser_output_file, "Rule {STMT_LIST' -> ; STMT STMT_LIST'}\n");
		parse_STMT();
		parse_STMT_LIST_TAG();
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {STMT_LIST' -> epsilon}\n");
			back_token();
			break;
		}
		error();
		break;
	}
}

void parse_STMT() {
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	eTOKENS tokens[] = { ID_tok, CURLY_BRACKET_OPEN_tok, IF_tok, RETURN_tok };
	expected_token_types = tokens;
	expected_token_types_size = 4;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {STMT -> id VAR_OR_CALL | COMP_STMT | IF_STMT | RETURN_STMT}\n");

	switch (current_token->kind)
	{
	case ID_tok:
		fprintf(parser_output_file, "Rule {STMT -> id VAR_OR_CALL}\n");
		table_entry id= lookup(current_token->lexeme);
		parse_VAR_OR_CALL(id);
		break;
	case CURLY_BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {STMT -> COMP_STMT}\n");
		back_token();
		parse_COMP_STMT();
		break;
	case IF_tok:
		fprintf(parser_output_file, "Rule {STMT -> IF_STMT}\n");
		back_token();
		parse_IF_STMT();
		break;
	case RETURN_tok:
		fprintf(parser_output_file, "Rule {STMT -> RETURN_STMT}\n");
		back_token();
		parse_RETURN_STMT();
		break;
	default:
		error();
		break;
	}
}

void parse_VAR_OR_CALL(table_entry id) {
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	eTOKENS tokens[] = { PARENTHESIS_OPEN_tok, BRACKET_OPEN_tok,ASSIGNMENT_OP_tok };
	expected_token_types = tokens;
	expected_token_types_size = 3;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {VAR_OR_CALL -> (ARGS) | VAR' = EXPR}\n");

	switch (current_token->kind)
	{
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL -> (ARGS)}\n");
		/*Semantic*/
		if (id->Role != FullDefinition)
		{
			semantic_error("Calling args on not a Function var");
		}
		ListNode* args = parse_ARGS();
		int result = check_types_equality(id->ListOfParameterTypes, args);
		/*Semantic*/
		current_follow = follow;
		current_follow_size = 2;
		if (!match(PARENTHESIS_CLOSE_tok))
			return;
		break;
	case BRACKET_OPEN_tok:
	case ASSIGNMENT_OP_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL -> VAR' = EXPR}\n");
		back_token();
		/*Semantic*/
		ListNode* var_tag_result = parse_VAR_TAG();
		int result = check_dim_equality(id->ListOfArrayDimensions, var_tag_result);
		/*Semantic*/
		current_follow = follow;
		current_follow_size = 2;
		if (!match(ASSIGNMENT_OP_tok))
			return;
		/*Semantic*/
		Type rightSide = parse_EXPR();
		if (!result)
		{
			if (rightSide != ErrorType)
			{
				if ((id->Type == Integer) && (rightSide != Integer))
				{
					semantic_error(" Right side not match Left side");
				}
			}
		}

		/*Semantic*/
		break;
	default:
		error();
		break;
	}
}

void parse_IF_STMT() {
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	fprintf(parser_output_file, "Rule {IF_STMT -> if (CONDITION) STMT}\n");
	if (!match(IF_tok))
		return;
	if (!match(PARENTHESIS_OPEN_tok))
		return;
	parse_CONDITION();
	current_follow = follow;
	current_follow_size = 2;
	if (!match(PARENTHESIS_CLOSE_tok))
		return;
	parse_STMT();
}

ListNode* parse_ARGS() {
	eTOKENS follow[] = { PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS tokens[] = {/*Firsts*/ ID_tok, INT_NUM_tok, FLOAT_NUM_tok, PARENTHESIS_OPEN_tok,/*Follows*/ PARENTHESIS_CLOSE_tok};
	expected_token_types = tokens;
	expected_token_types_size = 5;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {ARGS -> ARG_LIST | epsilon}\n");

	switch (current_token->kind)
	{
	case ID_tok:
	case INT_NUM_tok:
	case FLOAT_NUM_tok:
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {ARGS -> ARG_LIST}\n");
		back_token();
		/*Semantic*/
		ListNode* to_check = parse_ARG_LIST();
		if (!search_type_error(to_check))
			return to_check;
		else
			return NULL;
		/*Semantic*/
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {ARGS -> epsilon}\n");
			back_token();
			/*Semantic*/
			return (ListNode*)calloc(1, sizeof(ListNode));
			/*Semantic*/
		}
		error();
		/*Semantic*/
		return NULL;
		/*Semantic*/
	}
}

int match(eTOKENS expected_token) {
	eTOKENS expected_token_array[] = { expected_token };
	current_token = next_token();
	if (current_token->kind != expected_token)
	{
		expected_token_types = expected_token_array;
		expected_token_types_size = 1;
		error();
		return 0;
	}
	return 1;
}

int parse_Follow()
{
	int flag = 0;
	for (int i = 0; i < current_follow_size; i++)
	{
		flag += current_token->kind == current_follow[i];
	}
	return flag;
}

void error() {
	char* tokens_names;
	tokens_names = get_tokens_names();
	fprintf(parser_output_file, "Expected token of type '{%s}' at line: {%d},  Actual token of type '{%s}', lexeme: '{%s}'.\n", tokens_names, current_token->lineNumber, eTokensStrings[current_token->kind], current_token->lexeme);
	do
	{
		current_token = next_token();
	} while (parse_Follow() == 0 && current_token->kind != EOF_tok);
	back_token();

	free(tokens_names);
}

char* get_tokens_names()
{
	char* tokens;
	int size=0;
	for (int i = 0; i < expected_token_types_size; i++)
	{
		size += strlen(eTokensStrings[expected_token_types[i]]);
		size += strlen(", ");
	}
	size -= 2;
	size++;
	tokens = (char*)malloc(size);
	strncpy(tokens, eTokensStrings[expected_token_types[0]],
		strlen(eTokensStrings[expected_token_types[0]])+1);
	for (int i = 1; i < expected_token_types_size; i++)
	{
		strncat(tokens, ", ", strlen(", "));
		strncat(tokens, eTokensStrings[expected_token_types[i]],
			strlen(eTokensStrings[expected_token_types[i]]));
	}
	tokens[size - 1] = 0;
	return tokens;
}

ListNode* parse_ARG_LIST() {
	fprintf(parser_output_file, "Rule {ARG_LIST -> EXPR ARG_LIST'}\n");
	parse_EXPR();
	parse_ARG_LIST_TAG();
}
void parse_ARG_LIST_TAG() {
	eTOKENS follow[] = { PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS expected_tokens[] = { COMMA_tok, PARENTHESIS_CLOSE_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 2;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {ARG_LIST' -> , EXPR ARG_LIST' | Epsilon}\n");

	switch (current_token->kind)
	{
	case COMMA_tok:
		fprintf(parser_output_file, "Rule {ARG_LIST' -> , EXPR ARG_LIST'}\n");
		parse_EXPR();
		parse_ARG_LIST_TAG();
		break;
	case PARENTHESIS_CLOSE_tok:
		fprintf(parser_output_file, "Rule {ARG_LIST' -> Epsilon}\n");
		back_token();
		break;
	default:
		error();
		break;
	}
}
void parse_RETURN_STMT() {
	fprintf(parser_output_file, "Rule {RETURN_STMT -> return RETURN_STMT'}\n");
	if(!match(RETURN_tok))
		return;
	parse_RETURN_STMT_TAG();
}
void parse_RETURN_STMT_TAG() {
	// First of EXPR - id int_num float_num (
	// Follow of RETURN_STMT' - ; }
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	eTOKENS expected_tokens[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, ID_tok, INT_NUM_tok, FLOAT_NUM_tok, PARENTHESIS_OPEN_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 6;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {RETURN_STMT' -> EXPR | Epsilon}\n");

	switch (current_token->kind)
	{
	case ID_tok:
	case INT_NUM_tok:
	case FLOAT_NUM_tok:
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {RETURN_STMT' -> EXPR}\n");
		back_token();
		parse_EXPR();
		break;
	case SEMICOLON_tok:
	case CURLY_BRACKET_CLOSE_tok:
		fprintf(parser_output_file, "Rule {RETURN_STMT' -> Epsilon}\n");
		back_token();
		break;
	default:
		error();
		break;
	}
}
int parse_VAR_TAG() {
	// Follow of VAR' -  ; } , ) ] rel_op + * =
	// rel_op -  <  <=  ==  >=  >  != 
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok, 
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok,
						 ADD_OP_tok, MUL_OP_tok, ASSIGNMENT_OP_tok };
	current_follow = follow;
	current_follow_size = 14;
	eTOKENS expected_tokens[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok,
						 ADD_OP_tok, MUL_OP_tok, ASSIGNMENT_OP_tok, BRACKET_OPEN_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 15;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {VAR' -> [EXPR_LIST] | Epsilon}\n");

	switch (current_token->kind) {
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR' -> [EXPR_LIST]}\n");
		parse_EXPR_LIST();
		current_follow = follow;
		if(!match(BRACKET_CLOSE_tok))
			return;
		break;
	case SEMICOLON_tok:
	case CURLY_BRACKET_CLOSE_tok:
	case COMMA_tok:
	case PARENTHESIS_CLOSE_tok:
	case BRACKET_CLOSE_tok:
	case LESS_tok:
	case LESS_EQUAL_tok:
	case EQUAL_tok:
	case GREATER_tok:
	case GREATER_EQUAL_tok:
	case NOT_EQUAL_tok:
	case ADD_OP_tok:
	case MUL_OP_tok:
	case ASSIGNMENT_OP_tok:
		fprintf(parser_output_file, "Rule {VAR' -> Epsilon}\n");
		back_token();
		break;
	 default:
		error();
		break;
	}
}
void parse_EXPR_LIST() {
	fprintf(parser_output_file, "Rule {EXPR_LIST -> EXPR EXPR_LIST'}\n");
	parse_EXPR();
	parse_EXPR_LIST_TAG();
}
void parse_EXPR_LIST_TAG() {
	// Follow of EXPR_LIST' - ]
	eTOKENS follow[] = { BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS expected_tokens[] = { BRACKET_CLOSE_tok, COMMA_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 2;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {EXPR_LIST' -> , EXPR EXPR_LIST' | Epsilon}\n");

	switch (current_token->kind)
	{
	case COMMA_tok:
		fprintf(parser_output_file, "Rule {EXPR_LIST' -> , EXPR EXPR_LIST'}\n");
		parse_EXPR();
		parse_EXPR_LIST_TAG();
		break;
	case BRACKET_CLOSE_tok:
		fprintf(parser_output_file, "Rule {EXPR_LIST' -> Epsilon}\n");
		back_token();
		break;
	default:
		error();
		break;
	}
}
void parse_CONDITION() {
	eTOKENS follow[] = { PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS expected_tokens[] = { LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok,
											GREATER_EQUAL_tok, NOT_EQUAL_tok , PARENTHESIS_CLOSE_tok };
	fprintf(parser_output_file, "Rule {CONDITION -> EXPR rel_op EXPR}\n");
	parse_EXPR();
	current_follow = follow;
	current_follow_size = 1;
	current_token = next_token();
	if ((current_token->kind == LESS_tok) || (current_token->kind == LESS_EQUAL_tok) ||
		(current_token->kind == EQUAL_tok) || (current_token->kind == GREATER_tok) ||
		(current_token->kind == GREATER_EQUAL_tok) || (current_token->kind == NOT_EQUAL_tok))
		parse_EXPR();
	else
	{
		expected_token_types = expected_tokens;
		expected_token_types_size = 7;
		error();
	}
}
Type parse_EXPR() {
	fprintf(parser_output_file, "Rule {EXPR -> TERM EXPR'}\n");
	parse_TERM();
	parse_EXPR_TAG();
}
void parse_EXPR_TAG() {
	// Follow of EXPR' - ; } , ) ] rel_op
	// rel_op -  <  <=  ==  >=  >  != 
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok };
	current_follow = follow;
	current_follow_size = 11;
	eTOKENS expected_tokens[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok, ADD_OP_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 12;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {EXPR' -> + TERM EXPR' | Epsilon}\n");

	switch (current_token->kind) {
	case ADD_OP_tok:
		fprintf(parser_output_file, "Rule {EXPR' -> + TERM EXPR'}\n");
		parse_TERM();
		parse_EXPR_TAG();
		break;
	case SEMICOLON_tok:
	case CURLY_BRACKET_CLOSE_tok:
	case COMMA_tok:
	case PARENTHESIS_CLOSE_tok:
	case BRACKET_CLOSE_tok:
	case LESS_tok:
	case LESS_EQUAL_tok:
	case EQUAL_tok:
	case GREATER_tok:
	case GREATER_EQUAL_tok:
	case NOT_EQUAL_tok:
		fprintf(parser_output_file, "Rule {EXPR' -> Epsilon}\n");
		back_token();
		break;
	 default:
		error();
		break;
	}
}
void parse_TERM() {
	fprintf(parser_output_file, "Rule {TERM -> FACTOR TERM'}\n");
	parse_FACTOR();
	parse_TERM_TAG();
}
void parse_TERM_TAG() {
	// Follow of TERM' - ; } , ) ] rel_op +
	// rel_op -  <  <=  ==  >=  >  != 
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok, ADD_OP_tok };
	current_follow = follow;
	current_follow_size = 12;
	eTOKENS expected_tokens[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok, ADD_OP_tok,
						 MUL_OP_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 13;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {TERM' ->  * FACTOR TERM' | Epsilon}\n");

	switch (current_token->kind) {
	case MUL_OP_tok:
		fprintf(parser_output_file, "Rule {TERM' ->  * FACTOR TERM'}\n");
		parse_FACTOR();
		parse_TERM_TAG();
		break;
	case SEMICOLON_tok:
	case CURLY_BRACKET_CLOSE_tok:
	case COMMA_tok:
	case PARENTHESIS_CLOSE_tok:
	case BRACKET_CLOSE_tok:
	case LESS_tok:
	case LESS_EQUAL_tok:
	case EQUAL_tok:
	case GREATER_tok:
	case GREATER_EQUAL_tok:
	case NOT_EQUAL_tok:
	case ADD_OP_tok:
		fprintf(parser_output_file, "Rule {TERM' ->  Epsilon}\n");
		back_token();
		break;
	default:
		error();
		break;
	}
}
void parse_FACTOR() {
	// First of FACTOR - id int_num float_num (
	// Follow of FACTOR - ; } , ) ] rel_op + *
	// rel_op -  <  <=  ==  >=  >  != 
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok, ADD_OP_tok, MUL_OP_tok};
	current_follow = follow;
	current_follow_size = 13;
	eTOKENS expected_tokens[] = { ID_tok, INT_NUM_tok, FLOAT_NUM_tok, PARENTHESIS_OPEN_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 4;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {FACTOR -> id VAR_OR_CALL' | int_num | float_num | (EXPR)}\n");

	switch (current_token->kind) {
	case ID_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> id VAR_OR_CALL'}\n");
		parse_VAR_OR_CALL_TAG();
		break;
	case INT_NUM_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> int_num}\n");
		break;
	case FLOAT_NUM_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> float_num}\n");
		break;
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> (EXPR)}\n");
		parse_EXPR();
		current_follow = follow;
		if(!match(PARENTHESIS_CLOSE_tok))
			return;
		break;
	default:
		error();
		break;
	}
}
void parse_VAR_OR_CALL_TAG() {
	// Follow of VAR_OR_CALL' - ; } , ) ] rel_op + *
	// rel_op -  <  <=  ==  >=  >  != 
	// First of VAR' - [
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok, ADD_OP_tok,
						 MUL_OP_tok };
	current_follow = follow;
	current_follow_size = 13;
	eTOKENS expected_tokens[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok, ADD_OP_tok,
						 MUL_OP_tok, PARENTHESIS_OPEN_tok, BRACKET_OPEN_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 15;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {VAR_OR_CALL' -> (ARGS) | VAR'}\n");

	switch (current_token->kind) {
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL' -> (ARGS)}\n");
		parse_ARGS();
		current_follow = follow;
		if(!match(PARENTHESIS_CLOSE_tok))
			return;
		break;
	case SEMICOLON_tok:
	case CURLY_BRACKET_CLOSE_tok:
	case COMMA_tok:
	case PARENTHESIS_CLOSE_tok:
	case BRACKET_CLOSE_tok:
	case LESS_tok:
	case LESS_EQUAL_tok:
	case EQUAL_tok:
	case GREATER_tok:
	case GREATER_EQUAL_tok:
	case NOT_EQUAL_tok:
	case ADD_OP_tok:
	case MUL_OP_tok:
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL' -> VAR'}\n");
		back_token();
		parse_VAR_TAG();
		break;
	default:
		error();
		break;
	}
}