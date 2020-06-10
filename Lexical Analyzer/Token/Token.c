#include "Token.h"

/* This package describes the storage of tokens identified in the input text.
* The storage is a bi-directional list of nodes.
* Each node is an array of tokens; the size of this array is defined as TOKEN_ARRAY_SIZE.
* Such data structure supports an efficient way to manipulate tokens.

There are three functions providing an external access to the storage:
- function create_and_store_tokens ; it is called by the lexical analyzer when it identifies a legal token in the input text.
- functions next_token and back_token; they are called by parser during the syntax analysis (the second stage of compilation)
*/

int currentIndex = 0;
Node* currentNode = NULL;
extern int yylex(void);

#define TOKEN_ARRAY_SIZE 1000

/*
* This function creates a token and stores it in the storage.
*/
void create_and_store_token(eTOKENS kind, char* lexeme, int numOfLine)
{
	int length = strlen(lexeme) + 1;

	// case 1: there is still no tokens in the storage.
	if (currentNode == NULL)
	{
		currentNode = (Node*)malloc(sizeof(Node));

		if (currentNode == NULL)
		{
			fprintf(yyout, "\nUnable to allocate memory! \n");
			exit(0);
		}
		currentNode->tokensArray =
			(Token*)calloc(sizeof(Token), TOKEN_ARRAY_SIZE);
		if (currentNode->tokensArray == NULL)
		{
			fprintf(yyout, "\nUnable to allocate memory! \n");
			exit(0);
		}
		currentNode->prev = NULL;
		currentNode->next = NULL;
	}

	// case 2: at least one token exsits in the storage.
	else
	{
		// the array (the current node) is full, need to allocate a new node
		if (currentIndex == TOKEN_ARRAY_SIZE - 1)
		{
			currentIndex = 0;
			currentNode->next = (Node*)malloc(sizeof(Node));

			if (currentNode == NULL)
			{
				fprintf(yyout, "\nUnable to allocate memory! \n");
				exit(0);
			}
			currentNode->next->prev = currentNode;
			currentNode = currentNode->next;
			currentNode->tokensArray =
				(Token*)calloc(sizeof(Token), TOKEN_ARRAY_SIZE);

			if (currentNode->tokensArray == NULL)
			{
				fprintf(yyout, "\nUnable to allocate memory! \n");
				exit(0);
			}
			currentNode->next = NULL;
		}

		// the array (the current node) is not full
		else
		{
			currentIndex++;
		}
	}

	currentNode->tokensArray[currentIndex].kind = kind;
	currentNode->tokensArray[currentIndex].lineNumber = numOfLine;

	currentNode->tokensArray[currentIndex].lexeme = (char*)malloc(sizeof(char) * length);
#ifdef _WIN32
	strcpy_s(currentNode->tokensArray[currentIndex].lexeme, length, lexeme);
#else
	strcpy(currentNode->tokensArray[currentIndex].lexeme, lexeme);
#endif		
}

/*
* This function returns the token in the storage that is stored immediately before the current token (if exsits).
*/
Token* back_token()
{ 
	if (currentIndex > 0)
		currentIndex--;
	else if (currentNode -> prev == NULL)
		currentIndex=-1;
	else //When the last token created is at the beginning of the array
	{
		currentNode = currentNode->prev;
		currentIndex = TOKEN_ARRAY_SIZE - 1;
	}
	return NULL;
}

/*
* If the next token already exists in the storage (this happens when back_token was called before this call to next_token):
*  this function returns the next stored token.
* Else: continues to read the input file in order to identify, create and store a new token (using yylex function);
*  returns the token that was created.
*/
Token* next_token()
{
	if ((currentIndex + 1 == TOKEN_ARRAY_SIZE) && (currentNode->next != NULL)) //When there are more token in the next node and we are at the end of the array (back_token() was called before)
	{
		currentNode = currentNode->next;
		currentIndex = 0;
	}
	else if ((currentNode != NULL) && (currentIndex + 1 != TOKEN_ARRAY_SIZE) && ((&currentNode->tokensArray[currentIndex + 1])->lineNumber != 0)) //When we are not at the end of the token array and there is a token in the next index (back_token() was called)
	{
		currentIndex++;
	}
	else //When we need to ask for a new token from the lexical analyzer
	{
		yylex();
	}
	return &currentNode->tokensArray[currentIndex];
}

void clean_token_storage()
{
	int i;
	Node* prevNode;
	while (currentNode->next != NULL)
		currentNode = currentNode->next;
	while (currentNode != NULL)
	{
		i = 0;
		while (currentNode->tokensArray[i].lineNumber != 0)
		{
			free(currentNode->tokensArray[i].lexeme);
			i++;
		}
		free(currentNode->tokensArray);
		prevNode = currentNode->prev;
		free(currentNode);
		currentNode = prevNode;
	}
	currentNode = NULL;
	currentIndex = 0;
}
