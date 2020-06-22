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
	Float,
	IntArray,
	FloatArray,
	Void,
	TypeError
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


void set_id_name(SYMBOL_TABLE_ENTRY* entry, const char* name);

const char* get_id_name(SYMBOL_TABLE_ENTRY* entry);

void set_id_role(SYMBOL_TABLE_ENTRY* entry, Role role);

Role get_id_role(SYMBOL_TABLE_ENTRY* entry);

void set_id_type(SYMBOL_TABLE_ENTRY* entry, Type type);

Type get_id_type(SYMBOL_TABLE_ENTRY* entry);

void add_array_dimension_to_symbol_table_entry(SYMBOL_TABLE_ENTRY* entry, int dimension);

ListNode* get_dimensions_of_array(SYMBOL_TABLE_ENTRY* entry);

void add_parameter_type_to_symbol_table_entry(SYMBOL_TABLE_ENTRY* entry, Type type);

ListNode* get_parameter_types(SYMBOL_TABLE_ENTRY* entry);

int is_parameter_name_exist(SYMBOL_TABLE_ENTRY* entry, char* parameter_name);

SYMBOL_TABLE_ENTRY* create_new_symbol_table_entry();

void free_symbol_table_entry(SYMBOL_TABLE_ENTRY* entry);

void add_node_to_list(ListNode* list, ListNode* newNode);

void set_list(SYMBOL_TABLE_ENTRY* entry, ListNode* list);

void free_list(ListNode** list);