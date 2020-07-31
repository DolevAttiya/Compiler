#include <stdio.h>
#include "Semantic functions.h"
#define symbol_table value

table_entry lookupByTable(TYPE symbolTable,char* id_name);
void semantic_error(char* message);
table_ptr _get_current_table();
void _free_current_table_with_contents_from_list();


//SEMANTIC ANALYZER INTERFACE
table_entry insert(char* id_name)
{
	table_ptr current_table = _get_current_table();
	table_entry entry;

	entry = lookup(id_name);
	if (entry != NULL)
	{
		semantic_error("ID already exists\n");
		return NULL;
	}
	else
	{
		entry = create_new_symbol_table_entry();
		set_id_name(entry, id_name);
		current_table = insertMap(current_table, id_name, entry);
		symbolTableList->lastLink->prev->value = current_table;
		return entry;
	}
}

table_entry lookup(char* id_name) 
{
	table_ptr current_table = _get_current_table();
	return atMap(current_table, id_name);
}

table_ptr make_table() 
{ 
	table_ptr tab = createMap(3);
	addBackList(symbolTableList, tab);
	return tab; 
}

table_ptr pop_table()
{
	_free_current_table_with_contents_from_list();
	return _get_current_table();
}

table_entry find(char* id_name)
{
	DLink* node = symbolTableList->lastLink->prev;
	table_entry id_entry;

	while (node != symbolTableList->firstLink)
	{
		id_entry = lookupByTable(node->symbol_table, id_name);
		if (id_entry != NULL && id_entry != EmptyStruct)
			return id_entry;
		else
			node = node->prev;
	}
	return NULL;
}

//INTERNAL FUNCTIONS
void semantic_error(char *message)
{
	fprintf(semantic_analyzer_output_file, "line %4d : %s", semantic_error_line_number, message);
}

table_ptr _get_current_table()
{
	if (symbolTableList->size != 0)
		return symbolTableList->lastLink->prev->symbol_table;
	else
		return NULL;
}

void _free_current_table_with_contents_from_list()
{
	int i;
	struct hashLink* currLink, * nextLink = NULL;
	table_ptr current_table = _get_current_table();

	/*Checks if there's a symbol table left*/
	if (current_table == NULL)
		return;

	/* Frees all entries from the symbol table (Implementation copied from _freeMap() */
	for (i = 0; i < current_table->tableSize; i++) {
		currLink = current_table->table[i];
		if (currLink != 0)
			nextLink = currLink->next;
		while (currLink != 0) {
			free_symbol_table_entry(currLink->value);
			currLink = nextLink;
			if (currLink != 0)
				nextLink = currLink->next;
		}
	}

	/*Frees the symbol table (hashmap)*/
	deleteMap(current_table);

	/*Frees the node from the list of the symbol tables*/
	removeBackList(symbolTableList);
}

void find_predefinitions()
{
	int i;
	struct hashLink* currLink, * nextLink = NULL;
	table_ptr current_table = _get_current_table();

	if (current_table == NULL)
		return;

	for (i = 0; i < current_table->tableSize; i++) {
		currLink = current_table->table[i];
		if (currLink != 0)
			nextLink = currLink->next;
		while (currLink != 0) {
			if (currLink->value->Role == PreDefinition)
				semantic_error("Found a predefinition without a full definition\n"); //TODO: Assaf - need to add line number some how
			currLink = nextLink;
			if (currLink != 0)
				nextLink = currLink->next;
		}
	}
}

table_entry lookupByTable(TYPE symbolTable, char* id_name)
{
	return atMap(symbolTable, id_name);
}


/*in: ListNode to check for errors
out: 0 - if no error
	 1 - if the List is null
	 2 - if there is an errorType + Semantic print error
	 2 - if there is an DupplicatedError + Semantic print error
*/

//int search_type_error(ListNode* to_check)
//
//{
//	if (to_check == NULL)
//		return 1;
//	if (to_check->type == TypeError)
//	{
//		semantic_error("one of the parameter is a error parameter");
//		return 2;
//	}
//	else  if(to_check->type == DupplicateError)
//	{
//		semantic_error("There is a dupplicated parameter");
//		return 3;
//	}
//	while(to_check->next!=NULL)
//	{
//		to_check = to_check->next;
//		if (to_check->type == TypeError)
//		{
//			semantic_error("one of the parameter is a error parameter");
//			return 2;
//		}
//		else  if (to_check->type == DupplicateError)
//		{
//			semantic_error("There is a dupplicated parameter");
//			return 3;
//		}
//	}
//	return 0;
//}

/*
in : 2 ListNodes *
out : 
		0 - both types and size are the same
		1 - one of the types isnt the same
		2 - not same length of args
*/
//int check_types_equality(ListNode* id_parameters, ListNode* args) 
//{
//	if (id_parameters == NULL && args == NULL)
//		return 0;
//	else {
//		if (id_parameters == NULL || args == NULL)
//		{
//			semantic_error("not same sizes");
//			return 2;
//		}
//		else
//		{
//			if (id_parameters->type != args->type)
//			{
//				semantic_error("there is a diffrent parameter");
//				return 1;
//			}
//			while (id_parameters->next != NULL && args->next != NULL)
//			{
//				id_parameters = id_parameters->next;
//				args= args->next;
//				if (id_parameters->type != args->type)
//				{
//					semantic_error("there is a diffrent parameter");
//					return 1;
//				}
//			}
//			if ((id_parameters->next == NULL && args->next != NULL) || (id_parameters->next != NULL && args->next == NULL))
//			{
//				semantic_error("not same sizes");
//				return 2;
//			}
//			return 0;
//		}
//	}
//}
/*
in : 2 ListNodes *
out :
		0 - both types and size are the same
		1 - one of the dim is bigger isnt the same
		2 - not same length of args
*/

//int check_dim_equality(ListNode* id_parameters, ListNode* args)
//{
//	if (id_parameters == NULL && args == NULL)
//		return 0;
//	else {
//		if (id_parameters == NULL || args == NULL)
//		{
//			semantic_error("not same sizes");
//			return 2;
//		}
//		else
//		{
//			if (id_parameters->dimension > args->dimension)
//			{
//				semantic_error("there is a bigger size than exepted");
//				return 1;
//			}
//			while (id_parameters->next != NULL && args->next != NULL)
//			{
//				id_parameters = id_parameters->next;
//				args = args->next;
//				if (id_parameters->dimension > args->dimension)
//				{
//					semantic_error("there is a bigger size than exepted");
//					return 1;
//				}
//			}
//			if ((id_parameters->next == NULL && args->next != NULL) || (id_parameters->next != NULL && args->next == NULL))
//			{
//				semantic_error("not same sizes");
//				return 2;
//			}
//			return 0;
//		}
//	}
//}

void check_table_against_reality(Type table, Type reality) //TODO: Rotem - Map the reality types to printable strings
{
	if (table == FloatArray)
		if (!(reality == FloatArray || reality == IntArray))
			semantic_error("Expected type FloatArray, got %s", reality);
	if (table == IntArray)
		if (!reality == IntArray)
			semantic_error("Expected type IntArray, got %s", reality);
	if (table == Integer)
		if (!reality == Integer)
		{
			semantic_error("Expected type Integer, got %s", reality);
		}
	if (table == Float)
		if (!(reality == Float || reality == Integer))
		{
			semantic_error("Expected type Float, got %s", reality);
		}
}
