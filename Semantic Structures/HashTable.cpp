// HashTable.cpp : Defines the functions for the static library.
//
#include "pch.h"
#include "framework.h"
#include<stdio.h>
#include<stdlib.h>

struct data
{
	int key;
	int value; //TODO Change to Structure Entitiy @assafLerman
};

struct HashTable
{
	struct data* array;
	int capacity;
	int size;
};

int hashcode(int key, HashTable ht)
{
	return (key % ht.capacity);
}

int get_prime(int n)
{
	if (n % 2 == 0)
	{
		n++;
	}
	for (; !if_prime(n); n += 2);

	return n;
}

int if_prime(int n)
{
	int i;
	if (n == 1 || n == 0)
	{
		return 0;
	}
	for (i = 2; i*i < n; i++)
	{
		if (n % i == 0)
		{
			return 0;
		}
	}
	return 1;
}

void init_array(HashTable ht, int cap)
{
	int i;
	ht.capacity = get_prime(cap);
	ht.size = 0;
	ht.array = (struct data*)malloc(ht.capacity * sizeof(struct data));
	for (i = 0; i < ht.capacity; i++)
	{
		ht.array[i].key = 0;
		ht.array[i].value = 0;
	}
}

void insert(HashTable ht, int key)
{
	int index = hashcode(key, ht);
	if (ht.array[index].value == 0)
	{
		/*  key not present, insert it  */
		ht.array[index].key = key;
		ht.array[index].value = 1;
		ht.size++;
	}
	else if (ht.array[index].key == key)
	{
		/*  updating already existing key  */
		ht.array[index].value += 1;
	}
	else
	{
		//TODO ERROR ??
		/*  key cannot be insert as the index is already containing some other key  */
	}
}

void delete_element(int key, HashTable ht)
{
	int index = hashcode(key, ht);
	if (ht.array[index].value == 0)
	{
		printf("\n This key does not exist \n");
	}
	else {
		ht.array[index].key = 0;
		ht.array[index].value = 0;
		ht.size--;
		printf("\n Key (%d) has been removed \n", key);
	}
}

int look_up(HashTable ht, int key) // TODO change to the structure @AssafLerman
{
	if (ht.size == 0 || ht.array[key].value == 0) // TODO change to nullptr
		return -1;
	else
		return ht.array[key].value;
}