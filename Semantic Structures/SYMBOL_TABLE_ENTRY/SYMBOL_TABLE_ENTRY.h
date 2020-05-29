#pragma once

#define NULL 0

typedef enum Role
{
	Variable,
	PreDefinition,
	FullDefinition
} Role;

typedef enum Type
{
	Integer,
	Float
} Type;

typedef struct ListNode
{
	union {
		int dimension;
		Type type;
	};
	struct ListNode* next;
} ListNode;

typedef struct SYMBOL_TABLE_ENTRY
{
	char* Name;
	Role Role;
	Type Type;
	union {
		ListNode* ListOfArrayDimensions;
		ListNode* ListOfParameterTypes;
	};
} SYMBOL_TABLE_ENTRY;