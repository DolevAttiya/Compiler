#include "Semantic functions.h"

table_entry insert(table_ptr current_table, char* id_name)
{
	table_entry entry;
	entry = lookup(current_table, id_name);
}

table_entry lookup(table_ptr current_table, char* id_name)
{
	return atMap(current_table, id_name);
}

void AssafTest()
{
	hashMap* ht = createMap(1);
	SYMBOL_TABLE_ENTRY* entry = create_new_symbol_table_entry();
	set_id_name(entry, "hello");
	insertMap(ht,get_id_name(entry) ,entry);
	lookup(ht, "hello2");
}