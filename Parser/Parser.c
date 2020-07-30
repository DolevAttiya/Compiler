#include "../Lexical Analyzer/Token/Token.h"
char* eTokensStrings[];
#include "Parser.h"
#include <string.h>
#include "../Semantic Structures/SYMBOL_TABLE_ENTRY/SYMBOL_TABLE_ENTRY.h"
#include "Semantic functions.h"
#include "../Lexical Analyzer/Token/Token.h"
int scope = 0;
Token* current_token;
eTOKENS* current_follow;
int current_follow_size;
eTOKENS* expected_token_types;
int expected_token_types_size;

/*SEMANTIC*/
void init_semantic_analyzer();
void clean_semantic_analyzer();
int ParsingSucceeded;
/*SEMANTIC*/

int buffer_size=0;
char* temp_buffer=NULL;
int parser_output_file_last_position;
FILE* parser_output_file;

void parser()
{
	/*SEMANTIC*/
	init_semantic_analyzer();
	/*SEMANTIC*/

	parse_PROG();
	clean_token_storage();

	/*SEMANTIC*/
	clean_semantic_analyzer();
	/*SEMANTIC*/
}

void parse_PROG()
{
	int steps = 0;
	eTOKENS follow[] = { EOF_tok };
	current_follow = follow;
	current_follow_size = 1;

	/*SEMANTIC*/
	parse_BB();	printf("from scope : %d ", scope);	printf("to scope : %d\n", ++scope);
	/*SEMANTIC*/

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

	} while (1);

	eTOKENS follow2[] = { INT_tok, FLOAT_tok, VOID_tok };
	current_follow = follow2;
	current_follow_size = 3;
	fprintf(parser_output_file, "Rule {FUNC_PREDEFS -> FUNC_PROTYTYPE; FUNC_PREDEFS'}\n");

	/*SEMANTIC*/
	char* function_name;
	Type function_type;
	ListNode* parameters_list;
	/*SEMANTIC*/

	int local_line_number = current_token->lineNumber;
	parse_FUNC_PROTOTYPE(&function_name, &function_type, &parameters_list, PreDefinition);

	current_follow = follow2;
	current_follow_size = 3;

	if (match(SEMICOLON_tok))
	{
		parse_FB();	printf("from scope : %d ", scope);	printf("to scope : %d\n", --scope);	/*For the parameters scope*/ //TODO: Remove prints when finished
		/*SEMANTIC*/
		if (ParsingSucceeded)
		{
			semantic_error_line_number = local_line_number;
			table_entry entry = insert(function_name);
			if (entry != NULL)
			{
				set_id_role(entry, PreDefinition);
				set_id_type(entry, function_type);
				set_parameters_list(entry, parameters_list);
			}
		}
		/*SEMANTIC*/

		int count;
		Role role_for_params_parser;
		table_entry entry;
		do {
			count = 2;
			current_token = next_token();
			current_token = next_token();
			char* current_function_name = current_token->lexeme;
			semantic_error_line_number = current_token->lineNumber;
			while (current_token->kind != SEMICOLON_tok && current_token->kind != CURLY_BRACKET_OPEN_tok && current_token->kind != EOF)
			{
				count++;
				current_token = next_token();
			}
			if (current_token->kind == SEMICOLON_tok)
				role_for_params_parser = PreDefinition;
			else
				role_for_params_parser = FullDefinition;
			while (count--)
				back_token();
			if (role_for_params_parser == PreDefinition)
			{
				entry = insert(current_function_name);
			}
			else
			{
				entry = lookup(current_function_name);
				if (entry == NULL)
				{
					entry = insert(current_function_name);
					set_id_role(entry, FullDefinition);
				}
				else
				{
					if(entry->Role != PreDefinition)
						semantic_error("Full definition of function already exists\n");
				}
			}
			fprintf(parser_output_file, "Rule {FUNC_PREDEFS' -> FUNC_PROTYTYPE; FUNC_PREDEFS' | epsilon}\n");
			parser_output_file_last_position = ftell(parser_output_file);//save file seeker location
			local_line_number = current_token->lineNumber;
			parse_FUNC_PROTOTYPE(&function_name, &function_type, &parameters_list, role_for_params_parser);
			current_token = next_token();

			/*SEMANTIC*/
			if (current_token->kind == SEMICOLON_tok)
			{
				parse_FB();	printf("from scope : %d ", scope);	printf("to scope : %d\n", --scope);	/*For the parameters scope*/
				if (ParsingSucceeded)
				{
					semantic_error_line_number = local_line_number;
					entry = insert(function_name);
					if (entry != NULL)
					{
						set_id_role(entry, PreDefinition);
						set_id_type(entry, function_type);
						set_parameters_list(entry, parameters_list);
					}
				}
			}
			else
			{
				if (entry->Role == PreDefinition)
				{
					//check_types_equality(entry->ListOfParameterTypes, parameters_list);
					set_id_role(entry, FullDefinition);
					free_list(entry->ListOfParameterTypes);
					set_id_type(entry, function_type);
					set_parameters_list(entry, parameters_list);
				}
				else {
					set_parameters_list(entry, parameters_list);
					set_id_type(entry, function_type);
				}
			}
			/*SEMANTIC*/

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
		parse_FUNC_PROTOTYPE(&function_name, &function_type, &parameters_list, FullDefinition);
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
	if (temp_buffer != NULL)
		fprintf(parser_output_file, temp_buffer); //Append to output file
	free(temp_buffer);
	temp_buffer = NULL;

	/*SEMANTIC*/ //TODO: May need to remove
	//table_entry entry = find(function_name);
	//if (entry != NULL)
	//{
	//	if (entry->Role == FullDefinition)
	//	{
	//		fprintf(semantic_analyzer_output_file, "A full definition already exists\n");
	//	}
	//	else if (entry->Role == PreDefinition)
	//	{
	//		check_types_equality(entry->ListOfParameterTypes, parameters_list);
	//		set_id_role(entry, FullDefinition);
	//		free_list(entry->ListOfParameterTypes);
	//		set_parameters_list(entry, parameters_list);
	//	}
	//}
	//else
	//{
	//	table_entry entry = insert(function_name);
	//	set_id_role(entry, FullDefinition);
	//	set_id_type(entry, function_type);
	//	set_parameters_list(entry, parameters_list);
	//}
	/*SEMANTIC*/

	parse_COMP_STMT();

	/*SEMANTIC*/
	parse_FB();	printf("from scope : %d ", scope);	printf("to scope : %d\n", --scope); // For the parameters scope
	/*SEMANTIC*/

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

		find_predefinitions();

		/*SEMANTIC*/
		parse_FB();	printf("from scope : %d ", scope);	printf("to scope : %d\n", --scope);
		/*SEMANTIC*/
	}
	scope = 0;
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

	/*SEMANTIC*/
	ParsingSucceeded = 1;
	/*SEMANTIC*/

	fprintf(parser_output_file, "Rule {VAR_DEC -> TYPE id VAR_DEC'}\n");

	/*SEMANTIC*/
	Type type = parse_TYPE();
	/*SEMANTIC*/

	current_follow = follow;
	current_follow_size = 7;
	int local_line_number = current_token->lineNumber;
	if (!match(ID_tok))
	{
		ParsingSucceeded = 0;
		return;
	}

	/*SEMANTIC*/
	char* id_name = current_token->lexeme;
	/*SEMANTIC*/

	/*SEMANTIC*/
	ListNode* DimensionsList = NULL;
	/*SEMANTIC*/

	parse_VAR_DEC_TAG(&type, &DimensionsList);

	/*SEMANTIC*/
	if(ParsingSucceeded)
	{
		semantic_error_line_number = local_line_number;
		table_entry entry = insert(id_name);
		if (entry != NULL)
		{
			set_id_role(entry, Variable);
			set_id_type(entry, type);
			if (type == IntArray || type == FloatArray)
			{
				set_dimensions_list(entry, DimensionsList);
			}
		}
	}
	/*SEMANTIC*/
}

void parse_VAR_DEC_TAG(Type* type, ListNode** DimensionsList)
{
	eTOKENS follow[] = { INT_tok, FLOAT_tok, VOID_tok, ID_tok, CURLY_BRACKET_OPEN_tok, IF_tok, RETURN_tok };
	current_follow = follow;
	current_follow_size = 7;
	current_token = next_token();
	eTOKENS tokens[] = { SEMICOLON_tok, BRACKET_OPEN_tok };
	expected_token_types = tokens;
	expected_token_types_size = 2;

	/*SEMANTIC*/
	*DimensionsList = NULL;
	/*SEMANTIC*/

	fprintf(parser_output_file, "Rule {VAR_DEC' -> ; | [DIM_SIZES] ;}\n");

	switch (current_token->kind)
	{
	case SEMICOLON_tok: 
		fprintf(parser_output_file, "Rule {VAR_DEC' -> ;}\n");
		break;
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_DEC' -> [DIM_SIZES] ;}\n");
		parse_DIM_SIZES(DimensionsList);
		current_follow = follow;
		current_follow_size = 7;
		if (!match(BRACKET_CLOSE_tok))
		{
			/*SEMANTIC*/
			ParsingSucceeded = 0;
			/*SEMANTIC*/

			return;
		}
		if (!match(SEMICOLON_tok))
		{
			/*SEMANTIC*/
			ParsingSucceeded = 0;
			/*SEMANTIC*/

			return;
		}
		/*SEMANTIC*/
		if (*type == Integer)
			*type = IntArray;
		else if (*type == Float)
			*type == FloatArray;
		/*SEMANTIC*/

		break;
	default:
		error();

		/*SEMANTIC*/
		ParsingSucceeded = 0;
		/*SEMANTIC*/

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

		/*SEMANTIC*/
		return Integer;
		/*SEMANTIC*/

		break;
	case FLOAT_tok:
		fprintf(parser_output_file, "Rule {TYPE -> float}\n");

		/*SEMANTIC*/
		return Float;
		/*SEMANTIC*/

		break;
	default:
		error();

		/*SEMANTIC*/
		ParsingSucceeded = 0;
		return TypeError;
		/*SEMANTIC*/

		break;
	}
}

void parse_DIM_SIZES(ListNode** DimensionsList)
{
	eTOKENS follow[] = { BRACKET_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	fprintf(parser_output_file, "Rule {DIM_SIZES -> int_num DIM_SIZES'}\n");
	if (!match(INT_NUM_tok))
	{
		/*SEMANTIC*/
		ParsingSucceeded = 0;
		free_list(DimensionsList);
		/*SEMANTIC*/

		return;
	}
	else
	{
		/*SEMANTIC*/
		ListNode* NewDimension = (ListNode*)calloc(1, sizeof(ListNode));
		NewDimension->dimension = atoi(current_token->lexeme);
		add_node_to_list(DimensionsList, NewDimension);
		/*SEMANTIC*/
	}
	parse_DIM_SIZES_TAG(DimensionsList);
}

void parse_DIM_SIZES_TAG(ListNode** DimensionsList)
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
		parse_DIM_SIZES(DimensionsList);
		break;
	case BRACKET_CLOSE_tok:
		fprintf(parser_output_file, "Rule {DIM_SIZES' -> epsilon}\n");
		back_token();
		break;
	default:
		error();
		ParsingSucceeded = 0;
		free_list(DimensionsList);
		break;
	}
}

void parse_FUNC_PROTOTYPE(char** function_name, Type* function_type ,ListNode** ParametersList, Role role_for_parameters_parser)
{
	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_OPEN_tok };
	current_follow = follow;
	current_follow_size = 2;
	fprintf(parser_output_file, "Rule {FUNC_PROTOTYPE -> RETURN_TYPE id (PARAMS)}\n");

	/*SEMANTIC*/
	ParsingSucceeded = 1;
	if (*function_type = parse_RETURN_TYPE() == TypeError)
	{
		ParsingSucceeded = 0;
	}
	/*SEMANTIC*/

	current_follow = follow;
	current_follow_size = 2;
	if (!match(ID_tok))
	{

		/*SEMANTIC*/
		ParsingSucceeded = 0;
		/*SEMANTIC*/

		return;
	}

	/*SEMANTIC*/
	*function_name = current_token->lexeme;
	/*SEMANTIC*/

	if (!match(PARENTHESIS_OPEN_tok))
	{

		/*SEMANTIC*/
		ParsingSucceeded = 0;
		/*SEMANTIC*/

		return;
	}

	/*SEMANTIC*/
	table_entry entry = lookup(*function_name);
	parse_BB();	printf("from scope : %d ", scope);	printf("to scope : %d\n", ++scope);
	/*SEMANTIC*/
	if (role_for_parameters_parser == FullDefinition && entry!=NULL)
	{
		// there was a predef with or without paramteres
		/*if (entry->ListOfParameterTypes == NULL)
		{

		}*/
		*ParametersList = parse_PARAMS(role_for_parameters_parser, entry->ListOfParameterTypes);
	}
	else // func_pre_def || entry == NULL
	{
		ListNode* pre_list = NULL; // in case entry is NULL
		if (role_for_parameters_parser == FullDefinition) // entry is NULL
		{
			// AKA there was not pre def, so if this is the case there is no need to check for the entry pre deff cause there isn`t one
			pre_list = (ListNode*)malloc(sizeof(ListNode*));
			pre_list->dimension = -1;
			// if role_for_parameters_parser == FullDefinition check if pre_list->dimension == -1 
			// if -1 do not print error
			// else check if type != result type
			// if so prinnt error
		}
		*ParametersList = parse_PARAMS(role_for_parameters_parser, pre_list);
		free(pre_list);
	}

	if (!(*ParametersList))
	{
		/*SEMANTIC*/
		ParsingSucceeded = 0;
		/*SEMANTIC*/
	}

	current_follow = follow;
	current_follow_size = 2;
	if (!match(PARENTHESIS_CLOSE_tok))
	{
		/*SEMANTIC*/
		ParsingSucceeded = 0;
		/*SEMANTIC*/

		return;
	}


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
	
	/*SEMANTIC*/
	char* function_name;
	Type function_type;
	ListNode* parameters_list;
	/*SEMANTIC*/
	current_token = next_token();
	current_token = next_token();
	int local_line_number = current_token->lineNumber;
	char* current_function_name = current_token->lexeme;
	table_entry entry = lookup(current_function_name);
	back_token();
	back_token();
	parse_FUNC_PROTOTYPE(&function_name,&function_type, &parameters_list, FullDefinition);
	if (entry != NULL)
	{
		semantic_error_line_number = local_line_number;

		if (entry->Role == FullDefinition)
		{
			semantic_error("Full definition of function already exists\n");
		}
		else if (entry->Role == PreDefinition)
		{
			//check_types_equality(entry->ListOfParameterTypes, parameters_list);
			set_id_role(entry, FullDefinition);
			free_list(&(entry->ListOfParameterTypes));
			set_parameters_list(entry, parameters_list);
		}
	}
	else
	{
		table_entry entry = insert(function_name);
		set_id_role(entry, FullDefinition);
		set_id_type(entry, function_type);
		set_parameters_list(entry, parameters_list);
	}

	parse_COMP_STMT();

	/*SEMANTIC*/ //TODO: Remove prints when finished testing
	parse_FB();	printf("from scope : %d ", scope);	printf("to scope : %d\n", --scope); // For the parameters scope
	/*SEMANTIC*/
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
		parse_TYPE();
		break;
	case VOID_tok:
		fprintf(parser_output_file, "Rule {RETURN_TYPE -> void}\n");
		back_token();
		if (!match(VOID_tok))
			return TypeError;
		return Void;
	default:
		error();
		return TypeError;
	}
}

ListNode* parse_PARAMS(Role role_for_parameters_parser, ListNode* predef_types) {
	
	eTOKENS follow[] = { PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS tokens[] = {/*Firsts*/ INT_tok, FLOAT_tok,/*Follows*/ PARENTHESIS_CLOSE_tok };
	expected_token_types = tokens;
	expected_token_types_size = 3;
	int error_potential_line_number = current_token->lineNumber;
	current_token = next_token();
	fprintf(parser_output_file, "Rule {PARAMS -> PARAMS_LIST | epsilon}\n");
	switch (current_token->kind)
	{
	case INT_tok:
	case FLOAT_tok:
		fprintf(parser_output_file, "Rule {PARAMS -> PARAMS_LIST}\n");
		back_token();
		/*Semantic*/
		return parse_PARAM_LIST(role_for_parameters_parser, predef_types);
		/*Semantic*/
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {PARAMS -> epsilon}\n");
			back_token();
			/*Semantic*/
			if (role_for_parameters_parser == FullDefinition && predef_types != NULL && predef_types->dimension != -1)
			{
				back_token();
				semantic_error_line_number = error_potential_line_number;
				semantic_error("There are less params then in predefinition");
			}
			return NULL;
			/*Semantic*/
		}
		error();
		/*Semantic*/
		return NULL;
		/*Semantic*/
	}
}

ListNode* parse_PARAM_LIST(Role role_for_parameters_parser, ListNode* predef_types) {
	fprintf(parser_output_file, "Rule {PARAMS_LIST -> PARAM PARAMS_LIST'}\n");
	/*Semantic*/
	ListNode* Head = NULL;
	add_type_to_list_node(&Head, parse_PARAM(role_for_parameters_parser ,predef_types));
	ListNode* down_the_list = NULL;
	if (predef_types == NULL)
		down_the_list == NULL;
	else
		if (predef_types->dimension != -1)
			down_the_list = predef_types->next;
		else
			down_the_list = predef_types;
	/*Semantic*/
	parse_PARAM_LIST_TAG(Head, role_for_parameters_parser, down_the_list);
	return Head;
}

void parse_PARAM_LIST_TAG(ListNode* Head, Role role_for_parameters_parser, ListNode* predef_types) {
	eTOKENS follow[] = { PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS tokens[] = {/*Firsts*/ COMMA_tok,/*Follows*/ PARENTHESIS_CLOSE_tok };
	expected_token_types = tokens;
	expected_token_types_size = 2;
	int error_potential_line_number = current_token->lineNumber;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {PARAMS_LIST' -> , PARAM PARAMS_LIST' | epsilon}\n");

	switch (current_token->kind)
	{
	case COMMA_tok:
		fprintf(parser_output_file, "Rule {PARAMS_LIST' -> , PARAM PARAMS_LIST'}\n");
		/*Semantic*/
		add_type_to_list_node(&Head, parse_PARAM(role_for_parameters_parser, predef_types));
		ListNode* down_the_list = NULL;
		if (predef_types == NULL)
			down_the_list == NULL;
		else
			if (predef_types->dimension != -1)
				down_the_list = predef_types->next;
			else
				down_the_list = predef_types;
		/*Semantic*/
		parse_PARAM_LIST_TAG(Head, role_for_parameters_parser, down_the_list);
		break;
	default:
		if (parse_Follow() != 0)
		{
			if (role_for_parameters_parser == FullDefinition && predef_types != NULL && predef_types->dimension != -1) {
				semantic_error_line_number = error_potential_line_number;
				semantic_error("Less params then declered in predefinition\n");
			}
			fprintf(parser_output_file, "Rule {PARAMS_LIST' -> epsilon}\n");
			back_token();
			break;
		}
		error();
		break;
	}
}

Type parse_PARAM(Role role_for_parameters_parser, ListNode* predef_types) {
	eTOKENS follow[] = { COMMA_tok, PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	fprintf(parser_output_file, "Rule {PARAM -> TYPE id PARAM'}\n");
	/*Semantic*/
	if (role_for_parameters_parser == FullDefinition && predef_types == NULL)
	{
		semantic_error_line_number = current_token->lineNumber; 
		semantic_error("More params then declered in predefinition\n");
	}
	Type param_type = parse_TYPE();

	if (role_for_parameters_parser == FullDefinition && predef_types != NULL && predef_types->dimension != -1) // -1 means there is no pre def so there is nothing to check on
	{
		if (param_type != predef_types->type && param_type != TypeError && predef_types->type != TypeError)
		{
			semantic_error_line_number = current_token->lineNumber;
			semantic_error("Param type from decleration and implementation does not match\n");
		}
	}
	ListNode* dimList = NULL;
	/*Semantic*/
	current_follow = follow;
	current_follow_size = 2;
	if (!match(ID_tok))
		return NULL;
	int error_line_number = current_token->lineNumber;
	parse_PARAM_TAG(&param_type, &dimList, role_for_parameters_parser, predef_types);
	/*Semantic*/
	if (param_type != TypeError && dimList != -1)
	{
		back_token();
		current_token = next_token();
		table_entry id = insert(current_token->lexeme);
		if (id != NULL)
		{
			set_id_role(id, Variable);
			set_dimensions_list(id, dimList);
			set_id_type(id, param_type);
			return get_id_type(id);
		}
		else
		{
			if (role_for_parameters_parser == FullDefinition)
			{
				semantic_error_line_number = error_line_number;
				semantic_error("Dupplicated in line\n");
				return DupplicateError;
			}
		}
	}
	else if (dimList != -1)
	{
		free_list(&dimList);
	}
	return TypeError;
	/*Semantic*/
}

void parse_PARAM_TAG(Type* param_type, ListNode** dimList, Role role_for_parameters_parser, ListNode* predef_types) {
	eTOKENS follow[] = { COMMA_tok, PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	eTOKENS tokens[] = {/*Firsts*/ BRACKET_OPEN_tok,/*Follows*/ COMMA_tok, PARENTHESIS_CLOSE_tok };
	expected_token_types = tokens;
	expected_token_types_size = 3;
	int potential_semantic_error_line_number = current_token->lineNumber;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {PARAM' -> [DIM_SIZES] | epsilon}\n");

	switch (current_token->kind)
	{
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {PARAM' -> [DIM_SIZES]}\n");
		/*Semantic*/
		if (role_for_parameters_parser == FullDefinition && predef_types != NULL )
			if ((predef_types->type == Integer) || (predef_types->type == Float))
			{
				semantic_error_line_number = current_token->lineNumber;
				semantic_error("The Parameter is not an array as mentioned in predefinition\n");
			}
		parse_DIM_SIZES(dimList);
		if (*param_type == Integer)

			*param_type = IntArray;
		else
		{
			if (*param_type != TypeError)
				*param_type = FloatArray;
			else
				return;
		}
		/*Semantic*/
		current_follow = follow;
		current_follow_size = 2;
		if (!match(BRACKET_CLOSE_tok))
			return;
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {PARAM' -> epsilon}\n");
			/*Semantic*/
			// if() TODO need to get back to in order to check if param` is epsilon and if so what i need to do
			if (role_for_parameters_parser == FullDefinition && predef_types != NULL && predef_types->dimension != NULL)
			{
				semantic_error_line_number = potential_semantic_error_line_number;
				semantic_error("param in predefinition is an array but in full definition is not");
			}
			*dimList = NULL;
			/*Semantic*/
			back_token();
			break;
		}
		error();
		/*Semantic*/
		*param_type = TypeError;
		*dimList = -1;
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
	parse_BB();	printf("from scope : %d ", scope);	printf("to scope : %d\n", ++scope);
	/*Semantic*/
	parse_VAR_DEC_LIST();
	parse_STMT_LIST();
	current_follow = follow;
	current_follow_size = 6;
	if (!match(CURLY_BRACKET_CLOSE_tok))
		return;
	/*Semantic*/
	parse_FB();	printf("from scope : %d ", scope);	printf("to scope : %d\n", --scope);
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
		table_entry id = find(current_token->lexeme);
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
	int result;
	fprintf(parser_output_file, "Rule {VAR_OR_CALL -> (ARGS) | VAR' = EXPR}\n");

	switch (current_token->kind)
	{
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL -> (ARGS)}\n");
		ListNode* down_the_tree;
		/*Semantic*/
		if (id == NULL)
		{
			semantic_error("Undeclered function\n");
			down_the_tree = -1;
		}
		else {
			if (id->Role != FullDefinition)
			{
				semantic_error("Calling args on not a Function var or not declared\n");
			}
			down_the_tree = id->ListOfParameterTypes;

		}

		parse_ARGS(down_the_tree);
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
		if (id == NULL)
		{
			semantic_error("Undeclered variable\n");
			id = -1;
		}
		else if (id->Role != Variable)
		{
			semantic_error("The id is not a variable\n");
		}

		Type leftSide = parse_VAR_TAG(id);

		/*if(get_id_type(id) != IntArray || get_id_type(id) != FloatArray)
			if (IntArray == leftSide || FloatArray == leftSide)
			{
				semantic_error("trying accsses a wrong type from the real type on the left side");
			}*/
		/*Semantic*/
		current_follow = follow;
		current_follow_size = 2;
		if (!match(ASSIGNMENT_OP_tok))
			return;
		/*Semantic*/
		Expr* rightSide = parse_EXPR();
		if (IntArray == rightSide->type || FloatArray == rightSide->type)
		{
			semantic_error("trying accsses a wrong type from the real type on the right side\n");
		}
		if (rightSide->type != TypeError && leftSide != TypeError)
		{

			if (!((leftSide == Integer && rightSide->type == Integer) || (leftSide == Float && (rightSide->type == Integer || rightSide->type == Float))))
			{
				semantic_error("Right side's type does not match left side's type\n");
			}
		}
		else
		{
			// Error ?
		}
		free(rightSide);
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
void parse_ARGS(ListNode* list_of_params_types) {
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
		parse_ARG_LIST(list_of_params_types);
		break;
	default:
		if (parse_Follow() != 0)
		{
			fprintf(parser_output_file, "Rule {ARGS -> epsilon}\n");
			/*Semantic*/
			if (list_of_params_types != NULL && list_of_params_types != -1)
			{
				semantic_error("there are less params then expected\n");
			}
			/*Semantic*/
			back_token();
		}
		error();
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

void parse_ARG_LIST(ListNode* list_of_params_types) { //funcs
	fprintf(parser_output_file, "Rule {ARG_LIST -> EXPR ARG_LIST'}\n");
	int current_token_line = current_token->lineNumber;
	if (list_of_params_types == NULL)// there is at list one argument but not in the func with body
		semantic_error("difference between definitions\n");// TODO: Handle a call without a definition
	Expr* expr = parse_EXPR();
	ListNode* down_the_list = NULL;
	if (list_of_params_types == NULL)
		semantic_error("Less argumments then expected");
	else {
		if (list_of_params_types != -1)
		{
			check_table_against_reality(list_of_params_types->type, expr->type);
			down_the_list = list_of_params_types->next;
		}
		else { down_the_list = -1; }
	}
	parse_ARG_LIST_TAG(down_the_list);
	free(expr);
}

void parse_ARG_LIST_TAG(ListNode* list_of_params_types) {
	eTOKENS follow[] = { PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS expected_tokens[] = { COMMA_tok, PARENTHESIS_CLOSE_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 2;
	current_token = next_token();
	fprintf(parser_output_file, "Rule {ARG_LIST' -> , EXPR ARG_LIST' | Epsilon}\n");
	ListNode* down_the_tree;
	switch (current_token->kind)
	{
	case COMMA_tok:
		
		fprintf(parser_output_file, "Rule {ARG_LIST' -> , EXPR ARG_LIST'}\n");
		/* Semantic */
		if (list_of_params_types == NULL)
			semantic_error("difference between definitions\n");
		Expr* expr = parse_EXPR();
		if (list_of_params_types == NULL)// because we are already in type error lets print it
			semantic_error("not equeinvalent number of params\n");// TODO SEMANTIC ERROR
		else
			if(list_of_params_types != -1)
				check_table_against_reality(list_of_params_types->type, expr->type);
		if (list_of_params_types == NULL || list_of_params_types == -1)
			down_the_tree = list_of_params_types;
		else 
			down_the_tree = list_of_params_types->next;
		/* Semantic */
		parse_ARG_LIST_TAG(down_the_tree);
		free(expr);
		break;
	case PARENTHESIS_CLOSE_tok:
		fprintf(parser_output_file, "Rule {ARG_LIST' -> Epsilon}\n");
		back_token();
		/* Semantic */
		if(NULL!=list_of_params_types)
			semantic_error("not equeinvalent number of params\n");
		/* Semantic */
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
	table_entry id;
	fprintf(parser_output_file, "Rule {RETURN_STMT' -> EXPR | Epsilon}\n");

	switch (current_token->kind)
	{
	case ID_tok:
		/* Semantic */
		id = find(current_token->lexeme);
		if (id == NULL)
			semantic_error(semantic_analyzer_output_file, "ID is not declared\n");
		/* Semantic */
	case INT_NUM_tok:
	case FLOAT_NUM_tok:
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {RETURN_STMT' -> EXPR}\n");
		back_token();
		free(parse_EXPR());
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

Type parse_VAR_TAG(table_entry id) { // arrays
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
	Type id_type;
	fprintf(parser_output_file, "Rule {VAR' -> [EXPR_LIST] | Epsilon}\n");
	if (id!=NULL && id!=-1)
		id_type = get_id_type(id);
	else id_type = TypeError;
	switch (current_token->kind) {
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR' -> [EXPR_LIST]}\n");
		ListNode* down_the_tree = NULL;
		if (id_type != FloatArray && id_type != IntArray)
			semantic_error("The id must be declared as array\n");
		if(id!=NULL && id!=-1)
			down_the_tree = id->ListOfArrayDimensions;
		else 
			down_the_tree = NULL;
		parse_EXPR_LIST(down_the_tree);
		current_follow = follow;
		if (!match(BRACKET_CLOSE_tok))
			return TypeError;
		if (id_type == FloatArray)
			return Float;
		else
			return Integer;
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
		if (id != NULL && id!=-1 && id->ListOfArrayDimensions != NULL)
			semantic_error("expected no params but shit happens\n");
		return id_type;
	default:
		error();
		return TypeError;
	}
}
void parse_EXPR_LIST(ListNode* list_of_dimensions) {
	fprintf(parser_output_file, "Rule {EXPR_LIST -> EXPR EXPR_LIST'}\n");
	if (list_of_dimensions == NULL)
		semantic_error("List of EXPR must include values\n");
	Expr* expr = parse_EXPR();
	if (expr->type != Integer)
		semantic_error("EXPR type must be Integer\n");
	else if(expr->Valueable)
	{
		
		if (list_of_dimensions != NULL && expr->Value >= list_of_dimensions->dimension)
			semantic_error("if expr_i is a token of kind int_num, value should not exceed the size of i - th dimension of the array\n");
	}
	ListNode* down_the_tree;
	if (list_of_dimensions == NULL)
		down_the_tree = list_of_dimensions;
	else
		down_the_tree = list_of_dimensions->next;
	parse_EXPR_LIST_TAG(down_the_tree);
	free(expr);
}
void parse_EXPR_LIST_TAG(ListNode* list_of_dimensions) {
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
		/* Semantic */
		if (list_of_dimensions == NULL)
			semantic_error("different num of dimensions\n");
		Expr* expr = parse_EXPR();
		if (expr->type != Integer)
			semantic_error("Type of expr in array must be integer\n");
		else if (expr->Valueable)
		{
			if (list_of_dimensions != -1 && list_of_dimensions != NULL && expr->Value >= list_of_dimensions->dimension)
				semantic_error("if expr_i is a token of kind int_num, value should not exceed the size of i - th dimension of the array\n");
		}
		ListNode* down_the_tree;
		if (list_of_dimensions == NULL || list_of_dimensions == -1)
			down_the_tree = list_of_dimensions;
		else
			down_the_tree = list_of_dimensions->next;
		parse_EXPR_LIST_TAG(down_the_tree);
		free(expr);

		/* Semantic */
	case BRACKET_CLOSE_tok:
		fprintf(parser_output_file, "Rule {EXPR_LIST' -> Epsilon}\n");
		back_token();
		if (list_of_dimensions != NULL)
			semantic_error("not equeivalent number of dimensions\n");
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
	Expr* expr1, *expr2;
	eTOKENS expected_tokens[] = { LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok,
											GREATER_EQUAL_tok, NOT_EQUAL_tok , PARENTHESIS_CLOSE_tok };
	fprintf(parser_output_file, "Rule {CONDITION -> EXPR rel_op EXPR}\n");
	expr1 = parse_EXPR();
	current_follow = follow;
	current_follow_size = 1;
	current_token = next_token();
	if ((current_token->kind == LESS_tok) || (current_token->kind == LESS_EQUAL_tok) ||
		(current_token->kind == EQUAL_tok) || (current_token->kind == GREATER_tok) ||
		(current_token->kind == GREATER_EQUAL_tok) || (current_token->kind == NOT_EQUAL_tok))
	{
		expr2 = parse_EXPR();
		free(expr2);
	}
	else
	{
		expected_token_types = expected_tokens;
		expected_token_types_size = 7;
		error();
	}
	free(expr1);
}
Expr* parse_EXPR() {
	fprintf(parser_output_file, "Rule {EXPR -> TERM EXPR'}\n");
	Expr* term_expr = parse_TERM();
	Expr* expr_tag =  parse_EXPR_TAG();//TODO If EPSILON
	Expr* expr = (Expr*)malloc(sizeof(Expr));
	if (term_expr->type == Integer && expr_tag->type == Integer)
	{
		expr->type = Integer;
		if (term_expr->Valueable == 1 && expr_tag->Valueable == 1)
		{
			expr->Valueable = 1;
			expr->Value = term_expr->Value + expr_tag->Value;
		}
		expr->Valueable = 0;
	}
	else if (term_expr->type == TypeError || expr_tag->type == TypeError)
	{
		expr->type = TypeError;
		expr->Valueable = 0;
	}
	else {
		expr->type = Float;
		expr->Valueable = 0;
	}
	free(term_expr);
	free(expr_tag);
	return expr;
}

Expr* parse_EXPR_TAG() {
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
	Expr* expr = (Expr*)malloc(sizeof(Expr));
	fprintf(parser_output_file, "Rule {EXPR' -> + TERM EXPR' | Epsilon}\n");

	switch (current_token->kind) {
	case ADD_OP_tok:
		fprintf(parser_output_file, "Rule {EXPR' -> + TERM EXPR'}\n");
		/* Semantic */
		Expr* term_expr = parse_TERM();
		Expr* expr_tag = parse_EXPR_TAG();
		if (term_expr->type == Integer && expr_tag->type == Integer) {
			expr->type = Integer;
			if (term_expr->Valueable == 1 && expr_tag->Valueable == 1)
			{
				expr->Valueable = 1;
				expr->Value = term_expr->Value + expr_tag->Value;
			}
			else
				expr->Valueable = 0;
		}
		else if (term_expr->type == TypeError || expr_tag->type == TypeError) {
			expr->type = TypeError;
			expr->Valueable = 0;
		}
		else {
			expr->type = Float;
			expr->Valueable = 0;
		}
		free(term_expr);
		free(expr_tag);
		return expr;
		/* Semantic */
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
		expr->type = Integer;
		expr->Valueable = 1;
		expr->Value = 0;
		return expr;
	 default:
		error();
		expr->type = TypeError;
		expr->Valueable = 0;
		return expr;
	}
}
Expr* parse_TERM() {
	fprintf(parser_output_file, "Rule {TERM -> FACTOR TERM'}\n");
	Expr* factor_expr = parse_FACTOR();
	Expr* term_tag_expr = parse_TERM_TAG();
	Expr* expr = (Expr*)malloc(sizeof(Expr));
	if (factor_expr->type == Integer && term_tag_expr->type == Integer)
	{
		expr->type = Integer;
		if (factor_expr->Valueable == 1 && term_tag_expr->Valueable == 1)
		{
			expr->Value = factor_expr->Value * term_tag_expr->Value;
			expr->Valueable = 1;
		}
		else
			expr->Valueable = 0;
	}
	else if (factor_expr->type == TypeError || term_tag_expr->type == TypeError)
	{
		expr->type = TypeError;
		expr->Valueable = 0;
	}
	else
	{
		expr->type = Float;
		expr->Valueable = 0;
	}
	free(factor_expr);
	free(term_tag_expr);
	return expr;
}
Expr* parse_TERM_TAG() {
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
	Expr* expr = (Expr*)malloc(sizeof(Expr));
	fprintf(parser_output_file, "Rule {TERM' ->  * FACTOR TERM' | Epsilon}\n");

	switch (current_token->kind) {
	case MUL_OP_tok:
		fprintf(parser_output_file, "Rule {TERM' ->  * FACTOR TERM'}\n");
		/* Semantic */
		Expr* factor_expr = parse_FACTOR();
		Expr* term_tag_expr = parse_TERM_TAG();
		if (factor_expr->type == Integer && term_tag_expr->type == Integer) {
			expr->type = Integer;
			if (factor_expr->Valueable == 1 && term_tag_expr->Valueable == 1)
			{
				expr->Valueable = 1;
				expr->Value = factor_expr->Value * term_tag_expr->Value;
			}
			else 
				expr->Valueable = 0;
		}
		else if (factor_expr->type == TypeError || term_tag_expr->type == TypeError) {
			expr->type = TypeError;
			expr->Valueable = 0;
		}
		else {
			expr->type = Float;
			expr->Valueable = 0;
		}
		free(factor_expr);
		free(term_tag_expr);
		return expr;
		/* Semantic */
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
		expr->type = Integer;
		expr->Valueable = 1;
		expr->Value = 1;
		return expr;
	default:
		error();
		expr->type = TypeError;
		expr->Valueable = 0;
		return expr;
	}
}

Expr* parse_FACTOR() {
	// First of FACTOR - id int_num float_num (
	// Follow of FACTOR - ; } , ) ] rel_op + *
	// rel_op -  <  <=  ==  >=  >  != 

	eTOKENS follow[] = { SEMICOLON_tok, CURLY_BRACKET_CLOSE_tok, COMMA_tok, PARENTHESIS_CLOSE_tok, BRACKET_CLOSE_tok,
						 LESS_tok, LESS_EQUAL_tok, EQUAL_tok, GREATER_tok, GREATER_EQUAL_tok, NOT_EQUAL_tok, ADD_OP_tok, MUL_OP_tok };
	current_follow = follow;
	current_follow_size = 13;
	eTOKENS expected_tokens[] = { ID_tok, INT_NUM_tok, FLOAT_NUM_tok, PARENTHESIS_OPEN_tok };
	expected_token_types = expected_tokens;
	expected_token_types_size = 4;
	current_token = next_token();
	fprintf(parser_output_file, "Rule {FACTOR -> id VAR_OR_CALL' | int_num | float_num | (EXPR)}\n");
	Expr* expr = (Expr*)malloc(sizeof(Expr)); 
	switch (current_token->kind) {
	case ID_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> id VAR_OR_CALL'}\n");
		/* Semantic */
		table_entry id = find(current_token->lexeme);
		if (id != NULL) {
			expr->type = get_id_type(id);
			expr->Valueable = 0;
		}
		else
		{
			expr->type = TypeError;
			expr->Valueable = 0;
		}
		/* Semantic */
		Type result = parse_VAR_OR_CALL_TAG(id);
		return expr;
	case INT_NUM_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> int_num}\n");
		expr->type = Integer;
		expr->Valueable = 1;
		expr->Value = atoi(current_token->lexeme);
		return expr;
	case FLOAT_NUM_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> float_num}\n");
		expr->type = Float;
		expr->Valueable = 0;
		return expr;
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> (EXPR)}\n");
		free(expr);
		expr = parse_EXPR();
		current_follow = follow;
		if (!match(PARENTHESIS_CLOSE_tok))
		{
			expr->type = TypeError;
			expr->Valueable = 0;
		}
		return expr;
	default:
		error();
		expr->type = TypeError;
		expr->Valueable = 0;
		return expr;
	}
}

Type parse_VAR_OR_CALL_TAG(table_entry id) {
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
	Type type;
	fprintf(parser_output_file, "Rule {VAR_OR_CALL' -> (ARGS) | VAR'}\n");

	switch (current_token->kind) {
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL' -> (ARGS)}\n");
		ListNode* down_the_tree;
		if (id == NULL)
		{
			// Line Number
			semantic_error("no implementation of function\n");
			down_the_tree = -1;
			type = TypeError;
		}
		else
		{
			down_the_tree = id->ListOfParameterTypes;
			type = id->Type;
		}
		parse_ARGS(down_the_tree);
		current_follow = follow;	
		if (!match(PARENTHESIS_CLOSE_tok))
			return TypeError;
		return type;
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
		table_entry id_down_the_tree;
		back_token();
		if (id == NULL)
		{
			semantic_error("Undeclared variable\n");
			id_down_the_tree = -1;
		}
		else
		{
			id_down_the_tree = id;
		}
		type = parse_VAR_TAG(id_down_the_tree);
		return type;
	default:
		error();
		return TypeError;
	}
}


void init_semantic_analyzer()
{
	semantic_error_line_number = 0;
	symbolTableList = createLinkedList();
	ParsingSucceeded = 1;
}

void clean_semantic_analyzer()
{
	//TODO: Add a freeing function for the symbolTableList
}

void parse_BB()
{
	make_table();
}

void parse_FB()
{
	pop_table();
}
