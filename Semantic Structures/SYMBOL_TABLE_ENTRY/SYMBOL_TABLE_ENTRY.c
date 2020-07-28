#include "SYMBOL_TABLE_ENTRY.h"

#include "INTERNAL FUNCTIONS.h"

#include <string.h>

#include <stdlib.h>

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
int  get_id_list_size(SYMBOL_TABLE_ENTRY* entry)
{
	return entry->ListSize;
}

void add_array_dimension_to_symbol_table_entry(SYMBOL_TABLE_ENTRY* entry, int dimension)
{
	ListNode* NewDimension = allocate_new_node_for_list(entry);
	NewDimension->dimension = dimension;
	entry->ListSize += 1;
}

ListNode* get_dimensions_of_array(SYMBOL_TABLE_ENTRY* entry)
{
	return entry->ListOfArrayDimensions;
}

void add_parameter_type_to_symbol_table_entry(SYMBOL_TABLE_ENTRY* entry, Type type)
{
	ListNode* NewParameterType = allocate_new_node_for_list(entry);
	NewParameterType->type = type;
	entry->ListSize += 1;
}

ListNode* get_parameter_types(SYMBOL_TABLE_ENTRY* entry)
{
	return entry->ListOfArrayDimensions;
}

SYMBOL_TABLE_ENTRY* create_new_symbol_table_entry()
{
	SYMBOL_TABLE_ENTRY* entry = (SYMBOL_TABLE_ENTRY*)calloc(1, sizeof(SYMBOL_TABLE_ENTRY));
	entry->ListSize = 0;
	//entry->ListOfParameterTypes = NULL;
	//entry->ListOfArrayDimensions = NULL;
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

int get_id_size(SYMBOL_TABLE_ENTRY* entry)
{
	if (entry->Type==Integer || entry->Type == Float)
	{
		return 0;
	}
	else if (entry->Type == IntArray || entry->Type == FloatArray)
	{
		if (entry->ListSize == 0)
		{
			entry->ListSize= get_node_list_size(entry->ListOfArrayDimensions) + get_node_list_size(entry->ListOfParameterTypes);
			return entry->ListSize;
		}

	}
		
}
int get_node_list_size(ListNode* listNode)
{
	int count = 0;
	if (listNode != NULL)
	{
		count++;
		while (listNode->next != NULL)
		{
			count++;
			listNode = listNode->next;
		}
	}
	return count;
}


void set_dimensions_list(SYMBOL_TABLE_ENTRY* entry, ListNode* list)

{
	entry->ListOfArrayDimensions = list;
}

void set_parameters_list(SYMBOL_TABLE_ENTRY* entry, ListNode* list)
{
	entry->ListOfParameterTypes = list;
}

void add_node_to_list(ListNode** list, ListNode* newNode)
{
	ListNode* currentNode = *list;

	if (*list == NULL)
	{
		*list = newNode;
	}
	else
	{
		while (currentNode->next != NULL)
		{
			currentNode = currentNode->next;
		}
		currentNode->next = newNode;
	}
	newNode->next = NULL;
}

void add_type_to_list_node(ListNode** list_node, Type type)
{
	ListNode* NewType = (ListNode*)malloc(sizeof(ListNode));
	NewType->type = type;

	if (*list_node != NULL) // When there are dimensions in the list
	{
		while ((*list_node)->next != NULL)
		{
			*list_node = (*list_node)->next;
		}
		(*list_node)->next = NewType;

	}
	else // When there are no dimensions in the list
	{
		*list_node = NewType;
	}
	NewType->next = NULL;
}