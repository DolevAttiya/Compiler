#pragma once
#include "../../Semantic Structures/SYMBOL_TABLE_ENTRY/SYMBOL_TABLE_ENTRY.h"

typedef struct ExprList
{
	struct ExprList* next;
	Type type;
	int Valueable;
	int Value;
}ExprList;
