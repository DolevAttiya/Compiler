#include "../Lexical Analyzer/Token/Token.h"
char* eTokensStrings[];
#include "Parser.h"
#include <string.h>
#include "../Semantic Structures/SYMBOL_TABLE_ENTRY/SYMBOL_TABLE_ENTRY.h"
#include "Semantic functions.h"
#include "../Lexical Analyzer/Token/Token.h"
#define table_entry SYMBOL_TABLE_ENTRY *
#define already_checked_as_error -1
#define is_empty 0
#define not_exists 0
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
			if (entry != not_exists)
			{
				set_line_number(entry, local_line_number);
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
				set_id_role(entry, PreDefinition);
				set_line_number(entry, local_line_number);
			}
			else
			{
				entry = lookup(current_function_name);
				if (entry == not_exists)
				{
					entry = insert(current_function_name);
					set_id_role(entry, FullDefinition);
					add_array_dimension_to_symbol_table_entry(entry, already_checked_as_error);
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
					//entry = insert(function_name);
					if (entry != not_exists)
					{
						//set_id_role(entry, PreDefinition);
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
					free_list(&(entry->ListOfParameterTypes));
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

		/*SEMANTIC*/
		parse_FB();	printf("from scope : %d ", scope);	printf("to scope : %d\n", --scope);
		/*SEMANTIC*/
	}
	find_predefinitions();
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
	ListNode* DimensionsList = is_empty;
	/*SEMANTIC*/

	parse_VAR_DEC_TAG(&type, &DimensionsList);

	/*SEMANTIC*/
	if(ParsingSucceeded)
	{
		semantic_error_line_number = local_line_number;
		table_entry entry = insert(id_name);
		if (entry != not_exists)
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
	*DimensionsList = is_empty;
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
			*type = FloatArray;
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
	if (role_for_parameters_parser == FullDefinition && entry!=not_exists && entry->ListOfParameterTypes!=is_empty && entry->ListOfParameterTypes->dimension != already_checked_as_error)
	{ /* for full def with pre definition (with or without parameters)*/

		*ParametersList = parse_PARAMS(role_for_parameters_parser, entry->ListOfParameterTypes);
	}
	else /*if we are in predef or if we are in full definition without predef 
		 (in this case the full deff is already is inside the symbol table)*/
	{
		ListNode* pre_list = is_empty; // in case entry is NULL
		if (role_for_parameters_parser == FullDefinition)
		{
			pre_list = entry->ListOfParameterTypes;
		}
		*ParametersList = parse_PARAMS(role_for_parameters_parser, pre_list);
	}

	if (*ParametersList==-1) // parse_PARAMS returned error 
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
	back_token();
	back_token();
	table_entry entry = lookup(current_function_name);
	semantic_error_line_number = local_line_number;
	if (entry == not_exists)
	{
		entry = insert(current_function_name);
		set_id_role(entry, FullDefinition);
		add_array_dimension_to_symbol_table_entry(entry, already_checked_as_error);
	}
	else
	{
		if (entry->Role != PreDefinition)
			semantic_error("Full definition of function already exists\n");
	}
	parse_FUNC_PROTOTYPE(&function_name,&function_type, &parameters_list, FullDefinition);
	if (entry->Role == PreDefinition)  // there was a pre def for this function
	{
		set_id_role(entry, FullDefinition);
		//semantic_error_line_number = local_line_number;

		/*if (entry->Role == FullDefinition)
		{
			semantic_error("Full definition of function already exists\n");
		}*/
		/*else*/
			//check_types_equality(entry->ListOfParameterTypes, parameters_list);	
	}
	else // there was not a pre def for this function
	{
		/*table_entry entry = insert(function_name);*/
		/*set_id_role(entry, FullDefinition);*/
		
	}
	free_list(&(entry->ListOfParameterTypes));
	set_id_type(entry, function_type);
	set_parameters_list(entry, parameters_list);
	
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
			if (role_for_parameters_parser == FullDefinition && predef_types != is_empty && predef_types->dimension != already_checked_as_error)
			{
				static int parameter_number = 1;
				back_token();
				while (predef_types != is_empty)
				{
					semantic_error_line_number = error_potential_line_number;
					char* str = (char*)malloc(sizeof("The #%d  parameter less then in declared predefinition\n") + 11);
					sprintf(str, "The #%d  parameter less then in declared predefinition\n", parameter_number);
					semantic_error(str);
					free(str);
					predef_types = predef_types->next;
					parameter_number++;
				}
				parameter_number = 1;
			}
			return is_empty;
			/*Semantic*/
		}
		error();
		/*Semantic*/
		return -1;
		/*Semantic*/
	}
}

ListNode* parse_PARAM_LIST(Role role_for_parameters_parser, ListNode* predef_types) {
	fprintf(parser_output_file, "Rule {PARAMS_LIST -> PARAM PARAMS_LIST'}\n");
	/*Semantic*/
	ListNode* Head = is_empty;
	add_type_to_list_node(&Head, parse_PARAM(role_for_parameters_parser ,predef_types, 1));
	ListNode* down_the_list = is_empty;
	if (predef_types == is_empty)
		down_the_list = is_empty;
	else
		if (predef_types->dimension != already_checked_as_error)
			down_the_list = predef_types->next;
		else
			down_the_list = predef_types;
	/*Semantic*/
	parse_PARAM_LIST_TAG(Head, role_for_parameters_parser, down_the_list);
	return Head;
}

void parse_PARAM_LIST_TAG(ListNode* Head, Role role_for_parameters_parser, ListNode* predef_types) {
	static int parameter_number = 2;
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
		add_type_to_list_node(&Head, parse_PARAM(role_for_parameters_parser, predef_types, parameter_number));
		ListNode* down_the_list = is_empty;
		if (predef_types == is_empty)
			down_the_list = is_empty;
		else
			if (predef_types->dimension != already_checked_as_error)
				down_the_list = predef_types->next;
			else
				down_the_list = predef_types;
		/*Semantic*/
		parameter_number++;
		parse_PARAM_LIST_TAG(Head, role_for_parameters_parser, down_the_list);
		break;
	default:
		if (parse_Follow() != 0)
		{
			if (role_for_parameters_parser == FullDefinition && predef_types != is_empty && predef_types->dimension != already_checked_as_error) {

				while (predef_types != is_empty)
				{
					semantic_error_line_number = error_potential_line_number;
					char* str = (char*)malloc(sizeof("The #%d parameter doesn't appear in the full definition but appears in predefinition\n") + 11);
					sprintf(str, "The #%d parameter doesn't appear in the full definition but appears in predefinition\n", parameter_number+1);
					semantic_error(str);
					free(str);
					predef_types = predef_types->next;
					parameter_number++;
				}
				
			}
			fprintf(parser_output_file, "Rule {PARAMS_LIST' -> epsilon}\n");
			back_token();
			parameter_number = 2;
			break;
			
		}
		error();
		break;
	}
}

Type parse_PARAM(Role role_for_parameters_parser, ListNode* predef_types, int parameter_number) {
	eTOKENS follow[] = { COMMA_tok, PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 2;
	fprintf(parser_output_file, "Rule {PARAM -> TYPE id PARAM'}\n");
	/*Semantic*/
																			  
	if (role_for_parameters_parser == FullDefinition && predef_types == is_empty/*TODO optional add a flag in order to print only once*/)
	{
		semantic_error_line_number = current_token->lineNumber;
		char* str = (char*)malloc(sizeof("The #%d parameter that appears in the full definition wasn't declared in the predefinition\n") + 11);
		sprintf(str, "The #%d parameter that appears in the full definition wasn't declared in the predefinition\n", parameter_number);
		semantic_error(str);
		free(str);
	}
	Type param_type = parse_TYPE();
	int potential_error_type = current_token->lineNumber;
	ListNode* dimList = is_empty;
	/*Semantic*/
	current_follow = follow;
	current_follow_size = 2;
	if (!match(ID_tok))
		return TypeError;
	/*Semantic*/
	int error_line_number = current_token->lineNumber;
	char* current_param = current_token->lexeme;
	/*Semantic*/
	parse_PARAM_TAG(&param_type, &dimList, role_for_parameters_parser, predef_types);
	/*Semantic*/
	if (role_for_parameters_parser == FullDefinition && predef_types != is_empty && predef_types->dimension != already_checked_as_error) // already_checked_as_error means there is no pre def so there is nothing to check on
	{
		if (param_type != predef_types->type && param_type != TypeError && predef_types->type != TypeError && (!((param_type == Integer && predef_types->type ==IntArray)|| (param_type == IntArray && predef_types->type == Integer) || (param_type == Float && predef_types->type == FloatArray) || (param_type == FloatArray && predef_types->type == Float) )))
		{
			semantic_error_line_number = potential_error_type;
			semantic_error("Parameter type mismatch between declaration and implementation\n");
		}
	}
	if (param_type != TypeError && dimList != already_checked_as_error)
	{
		if (role_for_parameters_parser == FullDefinition)
		{
			semantic_error_line_number = error_line_number;
			table_entry id;
			id = find(current_param);
			if (id != not_exists)
			{
				//if (id->Role != FullDefinition && id->Role != PreDefinition)
				//{
				//	id = insert(current_param);
				//	if (id != not_exists) // maybe did not succeded 
				//	{
				//		set_id_role(id, Variable);
				//		set_dimensions_list(id, dimList);
				//		set_id_type(id, param_type);
				//		return get_id_type(id);
				//	}
				//	else
				//	{
				//		return DupplicateError;
				//	}
				//}
				//else
				if (id->Role == FullDefinition|| id->Role == PreDefinition)

				{
					semantic_error("The parameter has the same name as a function");
					free_list(&dimList);
				}
			}
			//else {

				id = insert(current_param);
				if (id != not_exists) // maybe did not succeded 
				{
					set_id_role(id, Variable);
					set_dimensions_list(id, dimList);
					set_id_type(id, param_type);
					return get_id_type(id);
				}
				else
				{
					return DupplicateError;
				}
			//}
		}
		else {
			free_list(&dimList);
			return param_type;
		}
	}
	else if (dimList != already_checked_as_error)
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
		if (role_for_parameters_parser == FullDefinition && predef_types != is_empty)
			if ((predef_types->type == Integer) || (predef_types->type == Float))
			{
				semantic_error_line_number = current_token->lineNumber;
				semantic_error("The parameter is not an array as declared in the predefinition\n");
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
			if (role_for_parameters_parser == FullDefinition && predef_types != is_empty && predef_types->dimension != already_checked_as_error)//full definition  with predefinition and there was'nt an error befor on the type
			{ 
				if (!((predef_types->type == Integer) || (predef_types->type == Float)))
				{
					semantic_error_line_number = potential_semantic_error_line_number;
					semantic_error("Parameter in predefinition is declared as an array but in full definition is not\n");
				}
			}
			*dimList = is_empty;
			/*Semantic*/
			back_token();
			break;
		}
		error();
		/*Semantic*/
		*param_type = TypeError;
		*dimList = already_checked_as_error;
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
	int potential_error_line_number = current_token->lineNumber;
	current_token = next_token();
	int result;
	fprintf(parser_output_file, "Rule {VAR_OR_CALL -> (ARGS) | VAR' = EXPR}\n");

	switch (current_token->kind)
	{
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL -> (ARGS)}\n");
		ListNode* down_the_tree=is_empty;
		/*Semantic*/
		if (id == not_exists)
		{
			semantic_error_line_number = potential_error_line_number;
			semantic_error("Undeclared function\n");
			down_the_tree = already_checked_as_error;
		}
		else {
			if (id->Role != FullDefinition)
			{
				semantic_error_line_number = current_token->lineNumber;
				semantic_error("The called function has no implementation\n");
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
		if (id == not_exists)
		{
			semantic_error_line_number = potential_error_line_number;
			semantic_error("Undeclared variable\n");
			id = already_checked_as_error;
		}
		else if (id->Role != Variable)
		{
			semantic_error_line_number = potential_error_line_number;
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
		if (IntArray == rightSide->type || FloatArray == rightSide->type || IntArray == leftSide || FloatArray == leftSide)
		{
			semantic_error_line_number = current_token->lineNumber;
			semantic_error("Refering an entire array is forbidden\n");
		}
		if (rightSide->type != TypeError && leftSide != TypeError)
		{

			if (!((leftSide == Integer && rightSide->type == Integer) || (leftSide == Float && (rightSide->type == Integer || rightSide->type == Float))))
			{
				semantic_error_line_number = current_token->lineNumber;
				semantic_error("Type mismatch\n");
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
	int potential_line_number_error = current_token->lineNumber;
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
			if (list_of_params_types != is_empty && list_of_params_types != already_checked_as_error)
			{
				int arg_number = 1;
				semantic_error_line_number = current_token->lineNumber;
				while (list_of_params_types != is_empty)
				{
					char* str = (char*)malloc(sizeof("The #%d parameter in the function declaration is not used in the function call\n") + 11);
					sprintf(str, "The #%d parameter in the function declaration is not used in the function call\n",  arg_number);
					semantic_error(str);
					free(str);
					arg_number++;
				}
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
	int current_line_number = current_token->lineNumber;
	semantic_error_line_number = current_line_number;
	if (list_of_params_types == is_empty)// there is at list one argument but not in the func with body
		semantic_error("The #1 parameter in the function call is not declared in the function declaration\n");// TODO: Handle a call without a definition (maybe not relevant)
	Expr* expr = parse_EXPR();
	ListNode* down_the_list = is_empty;
	//if (list_of_params_types == is_empty)
	//	semantic_error("Less argumments then expected");
	if (list_of_params_types != is_empty) {
		if (list_of_params_types != already_checked_as_error)
		{
			check_table_against_reality(list_of_params_types->type, expr->type);
			down_the_list = list_of_params_types->next;
		}
		else { down_the_list = already_checked_as_error; }
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
	ListNode* down_the_tree = is_empty;
	int current_line_number = current_token->lineNumber;
	semantic_error_line_number = current_line_number;
	static int arg_number = 2;
	switch (current_token->kind)
	{
	case COMMA_tok:
		
		fprintf(parser_output_file, "Rule {ARG_LIST' -> , EXPR ARG_LIST'}\n");
		/* Semantic */
		if (list_of_params_types == is_empty)
		{
			char* str = (char*)malloc(sizeof("The #%d parameter in the function declaration is not used in the function call\n") + 11);
			sprintf(str, "The #%d parameter in the function declaration is not used in the function call\n", arg_number);
			semantic_error(str);
			free(str);
			
		}
		arg_number++;
		Expr* expr = parse_EXPR();
		if (list_of_params_types != is_empty)
			if(list_of_params_types != already_checked_as_error)
				check_table_against_reality(list_of_params_types->type, expr->type);
		if (list_of_params_types == is_empty || list_of_params_types == already_checked_as_error)
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
		if (list_of_params_types != is_empty && list_of_params_types != already_checked_as_error)
		{
			int arg_number = 1;
			semantic_error_line_number = current_token->lineNumber;
			while (list_of_params_types != is_empty)
			{
				char* str = (char*)malloc(sizeof("The #%d parameter in the function declaration is not used in the function call\n") + 11);
				sprintf(str, "The #%d parameter in the function declaration is not used in the function call\n", arg_number);
				semantic_error(str);
				free(str);
				arg_number++;
			}
		}
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
	fprintf(parser_output_file, "Rule {RETURN_STMT' -> EXPR | Epsilon}\n");

	switch (current_token->kind)
	{
	case ID_tok:
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
	int current_line_number = current_token->lineNumber;
	semantic_error_line_number = current_line_number;
	fprintf(parser_output_file, "Rule {VAR' -> [EXPR_LIST] | Epsilon}\n"); 
	if (id==already_checked_as_error)
		id_type = TypeError;
	else
		id_type = get_id_type(id);
	switch (current_token->kind) {
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR' -> [EXPR_LIST]}\n");
		ListNode* down_the_tree = is_empty;
		if (id != already_checked_as_error && id->ListOfArrayDimensions == is_empty && id_type != TypeError)
		{
			semantic_error("ID was not declared as an array\n");
			//down_the_tree = NULL; is already NULL 
		}
		else
		{
			if (id_type != FloatArray && id_type != IntArray)
				semantic_error("ID was not declared as an array\n");
			if (id != already_checked_as_error)
				down_the_tree = id->ListOfArrayDimensions;
			else
				down_the_tree = already_checked_as_error; //because the id is not declared so we need not to check really the exprestions 
		}
			parse_EXPR_LIST(down_the_tree);
			current_follow = follow;
			if (!match(BRACKET_CLOSE_tok))
				return TypeError;
			if (id_type == FloatArray)
				return Float;
			else if ((id_type == IntArray))
				return Integer;
			else return TypeError;
		
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
		if (id!=already_checked_as_error && id->ListOfArrayDimensions != is_empty)
			semantic_error("Refering an entire array is forbidden\n");
		return id_type;
	default:
		error();
		return TypeError;
	}
}
void parse_EXPR_LIST(ListNode* list_of_dimensions) {
	fprintf(parser_output_file, "Rule {EXPR_LIST -> EXPR EXPR_LIST'}\n");
	int current_line_number = current_token->lineNumber;
	semantic_error_line_number = current_line_number;
	if (list_of_dimensions == is_empty)
		semantic_error("The ID cannot be used as an array\n");
	Expr* expr = parse_EXPR();
	if (expr->type != Integer)
		semantic_error("The dimension's type is not an integer\n");
	else if (expr->Valueable)
	{
		if (list_of_dimensions != already_checked_as_error && list_of_dimensions != is_empty && expr->Value >= list_of_dimensions->dimension)
			semantic_error("The specified dimension is out of range compared to the declaration\n"); //TODO: Added dimension number
	}
	ListNode* down_the_tree=is_empty;
	if (list_of_dimensions == is_empty || list_of_dimensions == already_checked_as_error)
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
	int current_line_number = current_token->lineNumber;
	semantic_error_line_number = current_line_number;
	switch (current_token->kind)
	{
	case COMMA_tok:
		fprintf(parser_output_file, "Rule {EXPR_LIST' -> , EXPR EXPR_LIST'}\n");
		/* Semantic */
		if (list_of_dimensions == is_empty)
			semantic_error("There is a mismatch between the number of dimensions used in the assignment than in the declaration of the array\n");
		Expr* expr = parse_EXPR();
		if (expr->type != Integer)
			semantic_error("Type of expr in array must be integer\n");
		else if (expr->Valueable)
		{
			if (list_of_dimensions != already_checked_as_error && list_of_dimensions != is_empty && expr->Value >= list_of_dimensions->dimension)
				semantic_error("The specified dimension is out of range compared to the declaration\n"); //TODO: Added dimension number
		}
		ListNode* down_the_tree=is_empty;
		if (list_of_dimensions == is_empty || list_of_dimensions == already_checked_as_error)
			down_the_tree = list_of_dimensions;
		else
			down_the_tree = list_of_dimensions->next;
		parse_EXPR_LIST_TAG(down_the_tree);
		free(expr);

		/* Semantic */
	case BRACKET_CLOSE_tok:
		fprintf(parser_output_file, "Rule {EXPR_LIST' -> Epsilon}\n");
		back_token();
		if (list_of_dimensions != is_empty)
			semantic_error("Trying to pass the array dimintion size\n");
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
	Expr* expr_tag =  parse_EXPR_TAG();
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
		if (id != not_exists) { // we do not print error to check what kind of id is it in parse_VAR_OR_CALL_TAG
			expr->type = get_id_type(id);
			if (expr->type == Integer)
			{
				expr->Valueable = 1;
			}
			else expr->Valueable = 0;
			//TODO - do we need to add check for id_type? Did I lied here? Maybe
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
	int current_line_number = current_token->lineNumber;
	fprintf(parser_output_file, "Rule {VAR_OR_CALL' -> (ARGS) | VAR'}\n");
	semantic_error_line_number = current_line_number;
	switch (current_token->kind) {
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR_OR_CALL' -> (ARGS)}\n");
		ListNode* down_the_tree=is_empty;
		if (id == not_exists)
		{
			semantic_error("The called function was not declared\n");
			down_the_tree = already_checked_as_error;
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
		table_entry id_down_the_tree = not_exists;
		back_token();
		if (id == not_exists)
		{
			semantic_error("Undeclared variable\n");
			id_down_the_tree = already_checked_as_error;
		}
		else
			id_down_the_tree = id;
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
	scope = 0;
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
