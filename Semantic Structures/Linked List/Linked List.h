#pragma once
#include "../Hashmap/Hashmap.h"
#ifndef __LISTDEQUE_H
#define __LISTDEQUE_H

# ifndef TYPE
# define TYPE      hashMap*
# define TYPE_SIZE sizeof(hashMap*)
# endif
# ifndef LT
# define LT(A, B) ((A) < (B))
# endif

# ifndef EQ
# define EQ(A, B) ((A) == (B))
# endif
/* Double Link*/
typedef struct DLink {
	TYPE value;
	struct DLink* next;
	struct DLink* prev;
}DLink;

/* Double Linked List with Head and Tail Sentinels  */

typedef struct linkedList {
	int size;
	struct DLink* firstLink;
	struct DLink* lastLink;
}linkedList;

 linkedList;

 linkedList* createLinkedList();

/* Deque Interface */
int 	isEmptyList( linkedList* lst);
void  addBackList( linkedList* lst, TYPE e);
void 	addFrontList( linkedList* lst, TYPE e);

TYPE  frontList( linkedList* lst);
TYPE 	backList( linkedList* lst);

void  removeFrontList( linkedList* lst);
void 	removeBackList( linkedList* lst);

/*Bag Interface */
void addList( linkedList* lst, TYPE v);
int containsList( linkedList* lst, TYPE e);
void removeList( linkedList* lst, TYPE e);

#endif