#pragma once
#include "../Semantic Structures/SYMBOL_TABLE_ENTRY/SYMBOL_TABLE_ENTRY.h"
#include "../Semantic Structures/Hashmap/Hashmap.h"
#define table_entry SYMBOL_TABLE_ENTRY *
#define table_ptr hashMap*

table_entry insert(table_ptr current_table, char* id_name);
table_entry lookup(table_ptr current_table, char* id_name);
void AssafTest();
