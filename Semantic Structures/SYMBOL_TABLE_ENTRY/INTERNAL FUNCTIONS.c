#include "INTERNAL FUNCTIONS.h"

ListNode* allocate_new_node_for_list(SYMBOL_TABLE_ENTRY* entry)
{
	ListNode* NewDimension = (ListNode*)malloc(sizeof(ListNode));
	ListNode* currentNode = entry->ListOfArrayDimensions;

	if (entry->ListOfArrayDimensions != NULL) // When there are dimensions in the list
	{
		while (currentNode->next != NULL)
		{
			currentNode = currentNode->next;
		}
		currentNode->next = NewDimension;

	}
	else // When there are no dimensions in the list
	{
		entry->ListOfArrayDimensions = NewDimension;
	}
	NewDimension->next = NULL;
	return NewDimension;
}

void add_node_to_list(ListNode* list, ListNode* newNode)
{
	ListNode* currentNode = list;

	if (list == NULL)
	{
		list = newNode;
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