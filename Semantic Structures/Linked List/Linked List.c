#include "Linked List.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/*
	initList
	param lst the linkedList
	pre: lst is not null
	post: lst size is 0
*/

void _initList( linkedList* lst) {
	lst->firstLink = malloc(sizeof( DLink));
	assert(lst->firstLink != 0);
	lst->lastLink = malloc(sizeof( DLink));
	assert(lst->lastLink);
	lst->firstLink->next = lst->lastLink;
	lst->lastLink->prev = lst->firstLink;
	lst->size = 0;
}

/*
 createList
 param: none
 pre: none
 post: firstLink and lastLink reference sentinels
 */

 linkedList* createLinkedList() {
	 linkedList* newList = malloc(sizeof( linkedList));
	_initList(newList);
	return(newList);
}

/*
	_addLinkBeforeBefore
	param: lst the linkedList
	param: l the  link to add before
	param: v the value to add
	pre: lst is not null
	pre: l is not null
	post: lst is not empty
*/

/* Adds Before the provided link, l */

void _addLinkBefore( linkedList* lst,  DLink* l, TYPE v) {
	 DLink* prevLink = l->prev;
	 DLink* newLink = ( DLink*)malloc(sizeof( DLink));

	assert(lst);
	/*assign newLink attributes*/
	newLink->value = v;
	newLink->next = l;
	newLink->prev = prevLink;

	/*reassign lnk and previousLink attributes*/
	l->prev = newLink;
	prevLink->next = newLink;

	lst->size++;
}


/*
	addFrontList
	param: lst the linkedList
	param: e the element to be added
	pre: lst is not null
	post: lst is not empty, increased size by 1
*/

void addFrontList( linkedList* lst, TYPE e) {
	assert(lst);
	_addLinkBefore(lst, lst->firstLink->next, e);
}

/*
	addBackList
	param: lst the linkedList
	param: e the element to be added
	pre: lst is not null
	post: lst is not empty, increased size by 1
*/

void addBackList( linkedList* lst, TYPE e) {
	assert(lst);
	_addLinkBefore(lst, lst->lastLink, e);
}

/*
	frontList
	param: lst the linkedList
	pre: lst is not null
	pre: lst is not empty
	post: none
*/

TYPE frontList( linkedList* lst) {
	assert(lst);
	assert(lst->size > 0);

	return lst->firstLink->next->value;
}

/*
	backList
	param: lst the linkedList
	pre: lst is not null
	pre: lst is not empty
	post: lst is not empty
*/

TYPE backList( linkedList* lst) {
	assert(lst);
	assert(lst->size > 0);

	return lst->lastLink->prev->value;
}

/*
	_removeLink
	param: lst the linkedList
	param: l the linke to be removed
	pre: lst is not null
	pre: l is not null
	post: lst size is reduced by 1
*/

void _removeLink( linkedList* lst,  DLink* l) {
	 DLink* prevLink = l->prev;
	 DLink* nextLink = l->next;

	assert(lst);

	prevLink->next = nextLink;
	nextLink->prev = prevLink;

	free(l);
	lst->size--;
}

/*
	removeFrontList
	param: lst the linkedList
	pre:lst is not null
	pre: lst is not empty
	post: size is reduced by 1
*/

void removeFrontList( linkedList* lst) {
	assert(lst);
	assert(!isEmptyList(lst));
	_removeLink(lst, lst->firstLink->next);
}

/*
	removeBackList
	param: lst the linkedList
	pre: lst is not null
	pre:lst is not empty
	post: size reduced by 1
*/

void removeBackList( linkedList* lst) {
	assert(lst);
	assert(!isEmptyList(lst));
	_removeLink(lst, lst->lastLink->prev);
}

/*
	isEmptyList
	param: lst the linkedList
	pre: lst is not null
	post: none
*/

int isEmptyList( linkedList* lst) {
	assert(lst);
	return lst->size == 0;
}


/*
	Add an item to the bag
	param: 	lst		pointer to the bag
	param: 	v		value to be added
	pre:	lst is not null
	post:	a link storing val is added to the bag
 */
void addList( linkedList* lst, TYPE v) {
	assert(lst);
	addFrontList(lst, v);
}

/*	Returns boolean (encoded as an int) demonstrating whether or not
	the specified value is in the collection
	true = 1
	false = 0
	param:	lst		pointer to the bag
	param:	e		the value to look for in the bag
	pre:	lst is not null
	pre:	lst is not empty
	post:	no changes to the bag
*/
int containsList( linkedList* lst, TYPE e) {
	 DLink* current = lst->firstLink;

	assert(lst);
	assert(!isEmptyList(lst));
	while (current->next != lst->lastLink) {
		current = current->next;
		if (compareTYPEValues(current->value , e))
			return 1;
	}

	return 0;
}

/*	Removes the first occurrence of the specified value from the collection
	if it occurs 
	param:	lst		pointer to the bag
	param:	e		the value to be removed from the bag
	pre:	lst is not null
	pre:	lst is not empty
	post:	e has been removed
	post:	size of the bag is reduced by 1
*/
void removeList( linkedList* lst, TYPE e) {
	 DLink* current = lst->firstLink->next;

	assert(lst);
	assert(!isEmptyList(lst));
	while (current->next != lst->lastLink) {
		current = current->next;
		if (compareTYPEValues(current->value, e)) {
			_removeLink(lst, current);
			return;
		}

	}

	return;
}


/*	Free LinkedList if it's empty
	param:	first		hashTable to free
	post:	return 1 if the list is not empty , 0 if succeded.
*/
int free_empty_list(linkedList* lst)
{
	if (!isEmptyList(lst))
		return 1;
	free(lst->firstLink);
	free(lst->lastLink);
	free(lst);
	return 0;
}

/*	Comapare hashTable's keys
	param:	first		hashTable to compare
	param:	second		hashTable to compare
	post:	return 1 if they are the same 0 if there is a diffrence
*/
int compareTYPEValues(TYPE first, TYPE second)
{
	//check the simple parameters in the hash table
	if (first->count == second->count && first->tableSize == second->tableSize)

	{
		hashLink* firsthash = first->table;// get the first hash
		hashLink* secondhash = second->table;// get the first hash
		while (firsthash->next != NULL && secondhash->next != NULL) // while one of the entries is not null
		{
			if (!strcmp(firsthash->key, secondhash->key)) // if the keys are the same
			{
				//move to the next entry
				firsthash = firsthash->next;
				secondhash = secondhash->next;
			}
			else return 0;
		}
		//if both of the hashes had ended
		if (firsthash->next == NULL && secondhash->next == NULL)
			return 1;
	}
	return 0;

}