#include "SYMBOL_TABLE_ENTRY.h"

#include "INTERNAL FUNCTIONS.h"

#include <string.h>

void set_id_name(SYMBOL_TABLE_ENTRY* entry, const char* name)
{
	entry->Name = _strdup(name);
}

const char* get_id_name(SYMBOL_TABLE_ENTRY* entry)
{
	return entry->Name;
}

void set_id_role(SYMBOL_TABLE_ENTRY* entry, Role role)
{
	entry->Role = role;
}

Role get_id_role(SYMBOL_TABLE_ENTRY* entry)
{
	return entry->Role;
}

void set_id_type(SYMBOL_TABLE_ENTRY* entry, Type type)
{
	entry->Type = type;
}

Type get_id_type(SYMBOL_TABLE_ENTRY* entry)
{
	return entry->Type;
}

void add_array_dimension_to_symbol_table_entry(SYMBOL_TABLE_ENTRY* entry, int dimension)
{
	ListNode* NewDimension = allocate_new_node_for_list(entry);
	NewDimension->dimension = dimension;
}

ListNode* get_dimensions_of_array(SYMBOL_TABLE_ENTRY* entry)
{
	return entry->ListOfArrayDimensions;
}

void add_parameter_type_to_symbol_table_entry(SYMBOL_TABLE_ENTRY* entry, Type type)
{
	ListNode* NewParameterType = allocate_new_node_for_list(entry);
	NewParameterType->type = type;
}

ListNode* get_parameter_types(SYMBOL_TABLE_ENTRY* entry)
{
	return entry->ListOfArrayDimensions;
}

SYMBOL_TABLE_ENTRY* create_new_symbol_table_entry()
{
	SYMBOL_TABLE_ENTRY* entry = (SYMBOL_TABLE_ENTRY*)calloc(sizeof(SYMBOL_TABLE_ENTRY));
	return entry;
}

void free_symbol_table_entry(SYMBOL_TABLE_ENTRY* entry)
{
	ListNode* currentNode = entry->ListOfArrayDimensions;
	ListNode* previousNode;

	free(entry->Name);
	while (currentNode != NULL)
	{
		previousNode = currentNode;
		currentNode = currentNode->next;
		free(previousNode);
	}

	free(entry);
}