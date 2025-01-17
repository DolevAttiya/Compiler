#include <stdio.h>
#include "Semantic functions.h"
#define symbol_table value

lookupByTable(TYPE symbolTable,char* id_name);
void semantic_error(char* message);
table_entry _get_current_table();
void _free_current_table_with_contents_from_list();


//SEMANTIC ANALYZER INTERFACE
table_entry insert(char* id_name)
{
	table_ptr current_table = _get_current_table();
	table_entry entry;

	entry = lookup(id_name);
	if (entry != NULL)
	{
		fprintf(semantic_analyzer_output_file,"Duplicate declaration\n");
		return NULL;
	}
	else
	{
		entry = create_new_symbol_table_entry();
		set_id_name(entry, id_name);
		insertMap(current_table, id_name, entry);
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
	table_ptr tab = createMap(1);
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
		if (id_entry != NULL)
			return id_entry;
		else
			node = node->prev;
	}
	return NULL;
}


//INTERNAL FUNCTIONS
void semantic_error(char *message)
{
	// Should print to the log file I guess
}

table_entry _get_current_table()
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
				fprintf(semantic_analyzer_output_file, "Found a predefinition without a full definition\n");
			currLink = nextLink;
			if (currLink != 0)
				nextLink = currLink->next;
		}
	}
}

int check_types_equality(ListNode* id_parameters, ListNode* args)
{
	return 0;
}


/*in: ListNode to check for errors
out: 0 - if no error
	 1 - if the List is null
	 2 - if there is an errorType + Semantic print error

*/
int search_type_error(ListNode* to_check)
{
	if (to_check == NULL)
		return 1;
	if (to_check->type == ErrorType)
	{
		semantic_error("one of the parameter is a error parameter");
		return 2;
	}
	while(to_check->next!=NULL)
	{
		to_check = to_check->next;
		if (to_check->type == ErrorType)
		{
			semantic_error("one of the parameter is a error parameter");
			return 2;
		}
	}
	return 0;
}

lookupByTable(TYPE symbolTable, char* id_name)
{
	return atMap(symbolTable, id_name);
}

//TEST
void AssafTest()
{
	table_ptr table = make_table();
	insert("hello");
	lookup("hello");
	pop_table();
}

/*
in : 2 ListNodes *
out : 
		0 - both types and size are the same
		1 - one of the types isnt the same
		2 - not same length of args
*/
int check_types_equality(ListNode* id_parameters, ListNode* args) 
{
	if (id_parameters == NULL && args == NULL)
		return 0;
	else {
		if (id_parameters == NULL || args == NULL)
		{
			semantic_error("not same sizes");
			return 2;
		}
		else
		{
			if (id_parameters->type != args->type)
			{
				semantic_error("there is a diffrent parameter");
				return 1;
			}
			while (id_parameters->next != NULL && args->next != NULL)
			{
				id_parameters = id_parameters->next;
				args= args->next;
				if (id_parameters->type != args->type)
				{
					semantic_error("there is a diffrent parameter");
					return 1;
				}
			}
			if ((id_parameters->next == NULL && args->next != NULL) || (id_parameters->next != NULL && args->next == NULL))
			{
				semantic_error("not same sizes");
				return 2;
			}
			return 0;
		}
	}
}

int check_dim_equality(ListNode* id_parameters, ListNode* args) 
{
	if (id_parameters == NULL && args == NULL)
		return 0;
	else {
		if (id_parameters == NULL || args == NULL)
		{
			semantic_error("not same sizes");
			return 2;
		}
		else
		{
			if (id_parameters->dimension > args->dimension)
			{
				semantic_error("there is a bigger size than exepted");
				return 1;
			}
			while (id_parameters->next != NULL && args->next != NULL)
			{
				id_parameters = id_parameters->next;
				args = args->next;
				if (id_parameters->dimension > args->dimension)
				{
					semantic_error("there is a bigger size than exepted");
					return 1;
				}
			}
			if ((id_parameters->next == NULL && args->next != NULL) || (id_parameters->next != NULL && args->next == NULL))
			{
				semantic_error("not same sizes");
				return 2;
			}
			return 0;
		}
	}
}
