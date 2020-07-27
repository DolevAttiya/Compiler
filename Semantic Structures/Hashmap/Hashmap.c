/*  Robert Kety
    Modified: 06/23/2014

    Description: Create a hash map based on provided skeleton code and videos from weeks 8 and 9. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hashMap.h"
    /* struct hashLink {
       KeyType key; //the key is what you use to look up a hashLink
       ValueType value; //the value stored with the hashLink, a pointer to int in the case of concordance
       struct hashLink * next; //notice how these are like linked list nodes
    };
    typedef struct hashLink hashLink;

    struct hashMap {
        hashLink ** table; //array of pointers to hashLinks
        int tableSize; //number of buckets in the table
        int count; //number of hashLinks in the table
    };
    typedef struct hashMap hashMap;
    */

    /* the first hashing function you can use*/
int stringHash1(char* str)
{
    int i;
    int r = 0;
    for (i = 0; str[i] != '\0'; i++)
        r += str[i];
    return r;
}

/*the second hashing function you can use*/
int stringHash2(char* str)
{
    int i;
    int r = 0;
    for (i = 0; str[i] != '\0'; i++)
        r += (i + 1) * str[i]; /*the difference between stringHash1 and stringHash2 is on this line*/
    return r;
}

/* initialize the supplied hashMap struct*/
void _initMap(struct hashMap* ht, int tableSize)
{
    int index;
    if (ht == NULL)
        return;
    ht->table = (hashLink**)malloc(sizeof(hashLink*) * tableSize);
    ht->tableSize = tableSize;
    ht->count = 0;
    for (index = 0; index < tableSize; index++)
        ht->table[index] = 0;
}

/* allocate memory and initialize a hash map*/
hashMap* createMap(int tableSize) {
    hashMap* ht;
    assert(tableSize > 0);
    ht = malloc(sizeof(hashMap));
    assert(ht != 0);
    _initMap(ht, tableSize);
    return ht;
}

/*
 Free all memory used by the buckets.
 Note: Before freeing up a hashLink, free the memory occupied by key and value
 */
void _freeMap(struct hashMap* ht) {
    int i = 0;
    struct hashLink* currLink, * nextLink=NULL;

    /* Scan each bucket for hashLinks and remove all hashLinks */
    for (i = 0; i < ht->tableSize; i++) {
        if (ht->count == 0)      //Reduce complexity if hash table doesn't have any hashLinks
            break;

        currLink = ht->table[i];
        if (currLink != 0)
            nextLink = currLink->next;
        while (currLink != 0) {       //Scan entire linked list
            /* "Free"-ing the memory occupied by key and value before freeing hashLink */
            //free(currLink->key);    //This corrupts the current and neighboring hashLink structs
            currLink->key = 0;
            currLink->value = EmptyStruct;

            /* Free hashLink and decrement hashLink counter */
            free(currLink);
            ht->count--;

            /* Move to next link */
            currLink = nextLink;
            if (currLink != 0)
                nextLink = currLink->next;
        }
    }
    free(ht->table);        //Free the dynamically allocated hashLink ** in hashMap struct
}

/* Deallocate buckets and the hash map.*/
void deleteMap(hashMap* ht) {
    assert(ht != 0);
    /* Free all memory used by the buckets */
    _freeMap(ht);
    /* free the hashMap struct */
    free(ht);
}

/*
Resizes the hash table to be the size newTableSize
*/
void _setTableSize(struct hashMap* ht, int newTableSize) {
    int i = 0;
    struct hashMap* newHM, * oldHM;
    struct hashLink* currLink;

    /* Create new hash table of size newTableSize */
    newHM = createMap(newTableSize);
    oldHM = ht;

    /* Iterate through all hashLinks in old hashMap and copy them into new hashMap */
    while (i < ht->tableSize) {
        currLink = ht->table[i];
        while (currLink != 0) {
            insertMap(newHM, currLink->key, currLink->value);
            currLink = currLink->next;
        }
        i++;
    }

    /* Replace hashMap pointer with new hashMap and free old hashMap */
    _freeMap(oldHM);
    ht->table = newHM->table;
    ht->tableSize = newHM->tableSize;
    ht->count = newHM->count;
}

/*
 insert the following values into a hashLink, you must create this hashLink but
 only after you confirm that this key does not already exist in the table. For example, you
 cannot have two hashLinks for the word "taco".

 if a hashLink already exists in the table for the key provided you should
 replace that hashLink--this requires freeing up the old memory pointed by hashLink->value
 and then pointing hashLink->value to value v.

 also, you must monitor the load factor and resize when the load factor is greater than
 or equal LOAD_FACTOR_THRESHOLD (defined in hashMap.h).
 */
hashMap* insertMap(struct hashMap* ht, KeyType k, ValueType v) {
    int hashIndex;
    struct hashLink* newHashLink = (struct hashLink*)malloc(sizeof(struct hashLink));
    char* newKeyType = (char*)malloc(strlen(k) + 1);     //Allocate memory for new char string

    /* Create hash index using one of two hashing function supplied */
    if (HASHING_FUNCTION == 1)
        hashIndex = stringHash1(k) % ht->tableSize;
    else
        hashIndex = stringHash2(k) % ht->tableSize;

    /* Ensure hashIndex is positive */
    if (hashIndex < 0)
        hashIndex += ht->tableSize;

    assert(newHashLink);

    /* Remove duplicate keys so new key replaces old key */
    if (containsKey(ht, k))
        removeKey(ht, k);

    /* Initialize new hashLink and add to appropriate hash index */
    strcpy(newKeyType, k);      //copy string stream into allocated memory reserved for this hashLink

    newHashLink->key = newKeyType;
    newHashLink->value = v;
    newHashLink->next = ht->table[hashIndex];
    ht->table[hashIndex] = newHashLink;

    ht->count++;

    /* Test table load and resize if necessary */
    if (tableLoad(ht) >= LOAD_FACTOR_THRESHOLD)
        _setTableSize(ht, ht->tableSize * 2+1);
    return ht;
}

/*
 NOTE: The return value for this function has changed.
 This returns the value (which is NOT void*) stored in a hashLink specified by the key k.

 if the user supplies the key "taco" you should find taco in the hashTable, then
 return the value member of the hashLink that represents taco.

 if the supplied key is not in the hashtable return NULL.
 */
ValueType atMap(struct hashMap* ht, KeyType k) {
    int hashIndex;
    struct hashLink* currLink;

    /* Determine hash index using the supplied hash functions */
    if (HASHING_FUNCTION == 1)
        hashIndex = stringHash1(k) % ht->tableSize;
    else
        hashIndex = stringHash2(k) % ht->tableSize;

    /* Set pointer to head of linked list at hash index of table */
    currLink = ht->table[hashIndex];
    /* Check each member in linked list for the requested KeyType and return value */
    while (currLink != 0) {
        if (strcmp(k, currLink->key) == 0)
            return currLink->value;
        currLink = currLink->next;
    }

    /* If search of linked list did not return a match, return NULL */
    return EmptyStruct;
}

/*
 a simple yes/no if the key is in the hashtable.
 0 is no, all other values are yes.
 */
int containsKey(struct hashMap* ht, KeyType k) {
    int hashIndex;
    struct hashLink* currLink;

    /* Determine hash index using the supplied hash functions */
    if (HASHING_FUNCTION == 1)
        hashIndex = stringHash1(k) % ht->tableSize;
    else
        hashIndex = stringHash2(k) % ht->tableSize;

    /* Set pointer to head of linked list at hash index of table */
    currLink = ht->table[hashIndex];
    /*  Check each member in linked list for the requested KeyType
        return true (1) if found. */
    while (currLink != 0) {
        if (strcmp(k, currLink->key) == 0)
            return 1;
        currLink = currLink->next;
    }

    /* If search of linked list did not return a match, return false (0) */
    return 0;
}

/*
 find the hashlink for the supplied key and remove it, also freeing the memory
 for that hashlink. it is not an error to be unable to find the hashlink, if it
 cannot be found do nothing (or print a message) but do not use an assert which
 will end your program.
 */
void removeKey(struct hashMap* ht, KeyType k) {
    int hashIndex;
    struct hashLink* lastLink, * currLink;

    /* Determine hash index using the supplied hash functions */
    if (HASHING_FUNCTION == 1)
        hashIndex = stringHash1(k) % ht->tableSize;
    else
        hashIndex = stringHash2(k) % ht->tableSize;

    /* Set pointer to head of linked list at hash index of table */
    currLink = ht->table[hashIndex];
    lastLink = currLink;
    /* Check each member in linked list for the requested KeyType and return value */
    while (currLink != 0) {
        if (strcmp(k, currLink->key) == 0) {
            if (lastLink == currLink) {
                lastLink = currLink->next;
                ht->table[hashIndex] = lastLink;
            }
            else
                lastLink->next = currLink->next;


            /* "Free"-ing the memory occupied by key and value before freeing hashLink */
            //free(currLink->key);    //Free char*
            currLink->value = EmptyStruct;

            /* Free hashLink and decrement hashLink counter */
            free(currLink);

            /* Decrement hashLink counter */
            ht->count--;

            /* Break from while loop */
            break;
        }
        currLink = currLink->next;
    }
}

/*
 returns the number of hashLinks in the table
 */
int size(struct hashMap* ht) {
    return ht->count;
}

/*
 returns the number of buckets in the table
 */
int capacity(struct hashMap* ht) {
    return ht->tableSize;
}

/*
 returns the number of empty buckets in the table, these are buckets which have
 no hashlinks hanging off of them.
 */
int emptyBuckets(struct hashMap* ht) {
    int emptyCounter = 0, i = 0;

    for (i = 0; i < ht->tableSize; i++)
        if (ht->table[i] == 0)
            emptyCounter++;

    return emptyCounter;
}

/*
 returns the ratio of: (number of hashlinks) / (number of buckets)

 this value can range anywhere from zero (an empty table) to more then 1, which
 would mean that there are more hashlinks then buckets (but remember hashlinks
 are like linked list nodes so they can hang from each other)
 */
float tableLoad(struct hashMap* ht) {
    float loadRatio = 0.0;

    loadRatio = (float)ht->count / (float)ht->tableSize;

    return loadRatio;
}

/*void printMap(struct hashMap* ht)
{
    int i;
    struct hashLink* temp;
    for (i = 0;i < capacity(ht); i++) {
        temp = ht->table[i];
        if (temp != 0) {
            printf("\nBucket Index %d -> ", i);
        }
        while (temp != 0) {
            printf("Key:%s|", temp->key);
            printValue(temp->value);
            printf(" -> ");
            temp = temp->next;
        }
    }
}*/
