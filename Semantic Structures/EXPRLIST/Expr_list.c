#include "Expr_list.h"
typedef struct ExprList
{
	struct ExprList* next;
	Type type;
	int Valueable;
	int Value;
}ExprList;
