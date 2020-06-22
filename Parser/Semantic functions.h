#pragma once
#include "../Semantic Structures/SYMBOL_TABLE_ENTRY/SYMBOL_TABLE_ENTRY.h"
#include "../Semantic Structures/Hashmap/Hashmap.h"
#include "../Semantic Structures/Linked List/Linked List.h"
#define table_entry SYMBOL_TABLE_ENTRY *
#define table_ptr hashMap*

FILE* semantic_analyzer_output_file;
linkedList* symbolTableList;
table_entry insert(char* id_name);
table_entry lookup(char* id_name);
table_ptr make_table();
table_ptr pop_table();
table_entry find(char* id_name);
int check_types_equality(ListNode* id_parameters, ListNode* args);
void find_predefinitions();
void AssafTest();
