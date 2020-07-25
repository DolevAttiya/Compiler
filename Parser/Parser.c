#include "../Lexical Analyzer/Token/Token.h"
char* eTokensStrings[];
#include "Parser.h"
#include <string.h>
#include "../Semantic Structures/SYMBOL_TABLE_ENTRY/SYMBOL_TABLE_ENTRY.h"
#include "Semantic functions.h"
#include "../Lexical Analyzer/Token/Token.h"

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
	parse_BB();
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

	} while(1);

	eTOKENS follow2[] = { INT_tok, FLOAT_tok, VOID_tok };
	current_follow = follow2;
	current_follow_size = 3;
	fprintf(parser_output_file, "Rule {FUNC_PREDEFS -> FUNC_PROTYTYPE; FUNC_PREDEFS'}\n");

	/*SEMANTIC*/
	char* function_name;
	Type function_type;
	ListNode* parameters_list;
	/*SEMANTIC*/

 	parse_FUNC_PROTOTYPE(&function_name, &function_type, &parameters_list, PreDefinition);

	current_follow = follow2;
	current_follow_size = 3;

	if (match(SEMICOLON_tok))
	{
		parse_FB();	/*For the parameters scope*/
		/*SEMANTIC*/
		if (ParsingSucceeded)
		{
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

		do {
			count = 0;
			while (current_token->kind != SEMICOLON_tok && current_token->kind != CURLY_BRACKET_OPEN_tok && current_token->kind != EOF )
			{
				count ++; 
				current_token = next_token();
			}
			if (current_token->kind == SEMICOLON_tok)
				role_for_params_parser = PreDefinition;
			else
				role_for_params_parser = FullDefinition;
			while (count--)
				back_token();

			fprintf(parser_output_file, "Rule {FUNC_PREDEFS' -> FUNC_PROTYTYPE; FUNC_PREDEFS' | epsilon}\n");
			parser_output_file_last_position = ftell(parser_output_file);//save file seeker location
			parse_FUNC_PROTOTYPE(&function_name, &function_type ,&parameters_list, role_for_params_parser);
			current_token = next_token();

			/*SEMANTIC*/
			if (current_token->kind == SEMICOLON_tok)
			{
				parse_FB();	/*For the parameters scope*/
				if (ParsingSucceeded)
				{
					table_entry entry = insert(function_name);
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
				table_entry entry = lookup(function_name);
				if (entry != NULL)
				{
					if (entry->Role == FullDefinition)
					{
						fprintf(semantic_analyzer_output_file, "A full definition already exists\n");
					}
					else if (entry->Role == PreDefinition)
					{
						check_types_equality(entry->ListOfParameterTypes, parameters_list);
						set_id_role(entry, FullDefinition);
						free_list(entry->ListOfParameterTypes);
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
	if(temp_buffer!=NULL)
		fprintf(parser_output_file, temp_buffer); //Append to output file
		free(temp_buffer);
	temp_buffer = NULL;

	/*SEMANTIC*/
	table_entry entry = find(function_name);
	if (entry != NULL)
	{
		if (entry->Role == FullDefinition)
		{
			fprintf(semantic_analyzer_output_file, "A full definition already exists\n");
		}
		else if (entry->Role == PreDefinition)
		{
			check_types_equality(entry->ListOfParameterTypes, parameters_list);
			set_id_role(entry, FullDefinition);
			free_list(entry->ListOfParameterTypes);
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
	/*SEMANTIC*/

	parse_COMP_STMT();

	/*SEMANTIC*/
	parse_FB(); // For the parameters scope
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
		parse_FB();
		/*SEMANTIC*/
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

	/*SEMANTIC*/
	ParsingSucceeded = 1;
	/*SEMANTIC*/

	fprintf(parser_output_file, "Rule {VAR_DEC -> TYPE id VAR_DEC'}\n");

	/*SEMANTIC*/
	Type type = parse_TYPE();
	/*SEMANTIC*/

	current_follow = follow;
	current_follow_size = 7;
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
	parse_BB();
	/*SEMANTIC*/

	if (!(*ParametersList = parse_PARAMS(role_for_parameters_parser)))
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

	parse_FUNC_PROTOTYPE(&function_name,&function_type, &parameters_list, FullDefinition);

	table_entry entry = lookup(function_name);
	if (entry != NULL)
	{
		if (entry->Role == FullDefinition)
		{
			fprintf(semantic_analyzer_output_file, "A full definition already exists\n");
		}
		else if (entry->Role == PreDefinition)
		{
			check_types_equality(entry->ListOfParameterTypes, parameters_list);
			set_id_role(entry, FullDefinition);
			free_list(entry->ListOfParameterTypes);
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

	/*SEMANTIC*/
	parse_FB(); // For the parameters scope
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

ListNode* parse_PARAMS(Role role_for_parameters_parser) {
	
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
		ListNode* to_check = parse_PARAM_LIST(role_for_parameters_parser);
		if (!search_type_error(to_check))  //TODO: change to 
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
			return (ListNode*)calloc(1, sizeof(ListNode));
			/*Semantic*/
		}
		error();
		/*Semantic*/
		return NULL;
		/*Semantic*/
	}
}

ListNode* parse_PARAM_LIST(Role role_for_parameters_parser) {
	fprintf(parser_output_file, "Rule {PARAMS_LIST -> PARAM PARAMS_LIST'}\n");
	/*Semantic*/
	ListNode* Head = NULL;
	add_type_to_list_node(&Head, parse_PARAM(role_for_parameters_parser));
	parse_PARAM_LIST_TAG(Head, role_for_parameters_parser);
	return Head;
	/*Semantic*/
}

void parse_PARAM_LIST_TAG(ListNode* Head, Role role_for_parameters_parser) {
	eTOKENS follow[] = { PARENTHESIS_CLOSE_tok };
	current_follow = follow;
	current_follow_size = 1;
	eTOKENS tokens[] = {/*Firsts*/ COMMA_tok,/*Follows*/ PARENTHESIS_CLOSE_tok };
	expected_token_types = tokens;
	expected_token_types_size = 2;
	current_token = next_token();

	fprintf(parser_output_file, "Rule {PARAMS_LIST' -> , PARAM PARAMS_LIST' | epsilon}\n");

	switch (current_token->kind)
	{
	case COMMA_tok:
		fprintf(parser_output_file, "Rule {PARAMS_LIST' -> , PARAM PARAMS_LIST'}\n");
		/*Semantic*/
		add_type_to_list_node(&Head, parse_PARAM(role_for_parameters_parser));
		/*Semantic*/
		parse_PARAM_LIST_TAG(Head, role_for_parameters_parser);	
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

Type parse_PARAM(Role role_for_parameters_parser) {
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
	if (!match(ID_tok))
		return NULL;
	int error_line_number = current_token->lineNumber;
	parse_PARAM_TAG(&param_type, &dimList);
	/*Semantic*/
	if (param_type != TypeError && dimList != NULL)
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
				char* str;
				sprintf(str, "Dupplicated in line %d", error_line_number);
				semantic_error(str);
				return DupplicateError;
			}
		}
	}
	else if (dimList != NULL)
	{
		free_list(&dimList);
	}
	return TypeError;
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
		*param_type = TypeError;
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
	parse_BB();
	/*Semantic*/
	parse_VAR_DEC_LIST();
	parse_STMT_LIST();
	current_follow = follow;
	current_follow_size = 6;
	if (!match(CURLY_BRACKET_CLOSE_tok))
		return;
	/*Semantic*/
	parse_FB();
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
		table_entry id = lookup(current_token->lexeme);
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
		/*Semantic*/
		if (id == EmptyStruct)
		{
			semantic_error("Undeclered function");
		}
		if (id->Role != FullDefinition)
		{
			semantic_error("Calling args on not a Function var or not declared");
		}
		parse_ARGS(id->ListOfParameterTypes);
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
		if (id == EmptyStruct)
		{
			semantic_error("Undeclered varable");
		}
		if (id->Role != Variable)
		{
			semantic_error("The id is not a varable");
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
		Type rightSide = parse_EXPR();
		if (IntArray == rightSide || FloatArray == rightSide)
		{
			semantic_error("trying accsses a wrong type from the real type on the right side");
		}
		if (rightSide != TypeError && leftSide != TypeError)
		{

			if (!((leftSide == Integer && rightSide == Integer) || (leftSide == Float && (rightSide == Integer) || (rightSide == Float))))
			{
				semantic_error("Right side's type does not match left side's type");
			}
		}
		else
		{
			// Error ?
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
			if (list_of_params_types != NULL)
			{
				semantic_error("there is a diffrence between the list");
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

void parse_ARG_LIST(ListNode* list_of_params_types) {
	fprintf(parser_output_file, "Rule {ARG_LIST -> EXPR ARG_LIST'}\n");
	int current_token_line = current_token->lineNumber;
	if (list_of_params_types == NULL)
		semantic_error("difference between definitions");
	check_table_against_reality(list_of_params_types->type, parse_EXPR());
	parse_ARG_LIST_TAG(*list_of_params_types->next);
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

	switch (current_token->kind)
	{
	case COMMA_tok:
		fprintf(parser_output_file, "Rule {ARG_LIST' -> , EXPR ARG_LIST'}\n");
		/* Semantic */
		if (list_of_params_types == NULL)
			semantic_error("difference between definitions");		
		check_table_against_reality(list_of_params_types->type, parse_EXPR());
		/* Semantic */
		parse_ARG_LIST_TAG(list_of_params_types->next);
		break;
	case PARENTHESIS_CLOSE_tok:
		fprintf(parser_output_file, "Rule {ARG_LIST' -> Epsilon}\n");
		back_token();
		/* Semantic */
		if(NULL!=list_of_params_types)
			semantic_error("not equeinvalent number of params");
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
		/* Semantic */
		table_entry id = lookup(current_token->lexeme);
		if (id == EmptyStruct)
			semantic_error(semantic_analyzer_output_file, "ID is not declared");
		/* Semantic */
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

Type parse_VAR_TAG(table_entry id) {
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
	Type id_type = get_id_type(id);
	switch (current_token->kind) {
	case BRACKET_OPEN_tok:
		fprintf(parser_output_file, "Rule {VAR' -> [EXPR_LIST]}\n");
		if (id_type != FloatArray && id_type != IntArray)
			semantic_error("The id must be declared as array");
		parse_EXPR_LIST(id->ListOfArrayDimensions);
		current_follow = follow;
		if(!match(BRACKET_CLOSE_tok))
			return TypeError;
		return id_type;
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
		if (id->ListOfArrayDimensions != NULL)
			semantic_error("expected no params but shit happens");
		return id_type;
	 default:
		error();
		return TypeError;
	}
}
void parse_EXPR_LIST(ListNode* list_of_dimensions) {
	fprintf(parser_output_file, "Rule {EXPR_LIST -> EXPR EXPR_LIST'}\n");
	if (list_of_dimensions == NULL)
		semantic_error("List of EXPR must include values");
	Expr expr = parse_EXPR(id);
	if (expr.type != Integer)
		semantic_error("EXPR type must be Integer");
	else if(expr.Valueable)
	{
		if (expr.Value >= list_of_dimensions->dimension)
			semantic_error("if expr_i is a token of kind int_num, value should not exceed the size of i - th dimension of the array");
	}
	parse_EXPR_LIST_TAG(list_of_dimensions->next);
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
		counter_param++;
		if (list_of_params_types->type != Integer)
			semantic_error(semantic_analyzer_output_file, "Type of expr in array must be integer");
		parse_EXPR();
		list_of_params_types->next;
		/* Semantic */
		parse_EXPR();
		parse_EXPR_LIST_TAG();
		break;
	case BRACKET_CLOSE_tok:
		fprintf(parser_output_file, "Rule {EXPR_LIST' -> Epsilon}\n");
		back_token();
		/* Semantic */
		if (expr_counter != list_of_params_types->dimension)
			semantic_error("n should be equal to the amount of dimensions in the array")
		/* Semantic */
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

Expr parse_EXPR(table_entry id) {
	fprintf(parser_output_file, "Rule {EXPR -> TERM EXPR'}\n");
	Expr term_expr = parse_TERM();
	Expr expr_tag =  parse_EXPR_TAG();
	if (term_expr->type == Integer && expr_tag == Integer)
		expr->type = Integer;
	return expr;
}
Expr parse_EXPR_TAG() {
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
Expr parse_TERM() {
	fprintf(parser_output_file, "Rule {TERM -> FACTOR TERM'}\n");
	parse_FACTOR();
	parse_TERM_TAG();
}
Expr parse_TERM_TAG() {
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
	Expr expr;
	fprintf(parser_output_file, "Rule {TERM' ->  * FACTOR TERM' | Epsilon}\n");

	switch (current_token->kind) {
	case MUL_OP_tok:
		fprintf(parser_output_file, "Rule {TERM' ->  * FACTOR TERM'}\n");
		/* Semantic */
		Expr factor_expr = parse_FACTOR();
		Expr expr_tag = parse_TERM_TAG();
		if (factor_expr.type == Integer && expr_tag.type == Integer)
			expr.type = Integer;
		else if (factor_expr.type == TypeError || expr_tag.type == TypeError)
			expr.type = TypeError;
		else expr.type = Float;
		expr.Valueable = 0;
		expr.Value = factor_expr.Value * expr_tag.Value;
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
		/* TODO - not sure what to do here */
		break;
	default:
		error();
		expr.type = TypeError;
		return expr;
	}
}

Expr parse_FACTOR() {
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
	Expr expr; 
	table_entry id = lookup(current_token->lexeme);
	switch (current_token->kind) {
	case ID_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> id VAR_OR_CALL'}\n");
		/* Semantic */
		if (id != NULL) {
			expr.type = get_id_type(id);
			expr.Valueable = 0;
		}
		/* Semantic */
		parse_VAR_OR_CALL_TAG(id);
		break;
	case INT_NUM_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> int_num}\n");
		if (id != NULL) {
			expr.type = Integer;
			expr.Valueable = 1;
			expr.Value = current_token->lexeme;
		}
		break;
	case FLOAT_NUM_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> float_num}\n");
		if (id != NULL) {
			expr.type = Float;
			expr.Valueable = 1;
			expr.Value = current_token->lexeme;
		}
		break;
	case PARENTHESIS_OPEN_tok:
		fprintf(parser_output_file, "Rule {FACTOR -> (EXPR)}\n");
		expr = parse_EXPR(id);
		current_follow = follow;
		if (!match(PARENTHESIS_CLOSE_tok))
		{
			expr.type = TypeError;
		}
		return expr;
	default:
		error();
		expr->type = TypeError;
		return expr;
	}
}

void parse_VAR_OR_CALL_TAG(table_entry id) {
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
		parse_ARGS(id->ListOfParameterTypes);
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
		parse_VAR_TAG(id);
		break;
	default:
		error();
		break;
	}
}


void init_semantic_analyzer()
{
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
