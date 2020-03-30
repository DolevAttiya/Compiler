%option noyywrap

%{
#include "Token.h"
int line_num=1;
void print_message(char*);
%}

INT_NUM 		[0-9]+
FLOAT_NUM 		[0-9]+(\.[0-9]+)?e[\+-]?[0-9]+
AR_OP			[+-]
REL_OP			([<>])|([<>=!]=)
ASSIGNMENT_OP		=
ID 			[a-z]([_a-zA-Z0-9][a-zA-Z0-9])*[a-zA-Z0-9]?
INT 			int
FLOAT 			float
VOID 			void
IF 			if
RETURN 			return
COMMA 			,
COLON 			:
SEMICOLON 		;
PARENTHESES		[\(\)]
BRACKETS 		[\[\]]
CURLY_BRACKETS		[{}]
COMMENT			\/\*([^*]|[\r\n]|(\*+([^*\/]|[\r\n])))*\*+\/

%%

\n 								{
									line_num++;
									return 1;
								}

<<EOF>>							{ 	
								create_and_store_token(EOF_tok,  yytext, line_num); 
                       						return 0;
 	     						}

{INT}							{ 	
								create_and_store_token(INT_tok,  yytext, line_num); 
                   						print_message("INT_tok");  
                       						return 1;
 	     						}

{FLOAT}							{ 	
								create_and_store_token(FLOAT_tok,  yytext, line_num); 
                   						print_message("FLOAT_tok");  
                       						return 1;
 	     						}

{VOID}							{ 	
								create_and_store_token(VOID_tok,  yytext, line_num); 
                   						print_message("VOID_tok");  
                       						return 1;
 	     						}

{IF}							{ 	
								create_and_store_token(IF_tok,  yytext, line_num); 
                   						print_message("IF_tok");  
                       						return 1;
 	     						}	

{RETURN}						{ 	
								create_and_store_token(RETURN_tok,  yytext, line_num); 
                   						print_message("RETURN_tok");  
                       						return 1;
 	     						}

{INT_NUM}						{ 	
								create_and_store_token(INT_NUM_tok,  yytext, line_num); 
                   						print_message("INT_NUM_tok");  
                       						return 1;
 	     						}

{FLOAT_NUM}						{ 	
								create_and_store_token(FLOAT_NUM_tok,  yytext, line_num); 
                   						print_message("FLOAT_NUM_tok");  
                       						return 1;
 	     						}

{AR_OP}							{ 
								create_and_store_token(AR_OP_tok,  yytext, line_num); 
                   						print_message("AR_OP_tok");  
                       						return 1;
 	     						}

{REL_OP}						{ 	
								create_and_store_token(REL_OP_tok,  yytext, line_num); 
                   						print_message("REL_OP_tok");  
                       						return 1;
 	     						}

{ASSIGNMENT_OP}						{ 	
								create_and_store_token(ASSIGNMENT_OP_tok,  yytext, line_num); 
                   						print_message("ASSIGNMENT_OP_tok");  
                       						return 1;
 	     						}

{ID}							{ 	
								create_and_store_token(ID_tok,  yytext, line_num); 
                   						print_message("ID_tok");  
                       						return 1;
 	     						}


{COMMA}							{ 	
								create_and_store_token(COMMA_tok,  yytext, line_num); 
                   						print_message("COMMA_tok");  
                       						return 1;
 	     						}

{COLON}							{ 	
								create_and_store_token(COLON_tok,  yytext, line_num); 
                   						print_message("COLON_tok");  
                       						return 1;
 	     						}

{SEMICOLON}						{ 	
								create_and_store_token(SEMICOLON_tok,  yytext, line_num); 
                   						print_message("SEMICOLON_tok");  
                       						return 1;
 	     						}

{PARENTHESES}						{ 	
								create_and_store_token(PARENTHESES_tok,  yytext, line_num); 
                   						print_message("PARENTHESES_tok");  
                       						return 1;
 	     						}

{BRACKETS}						{ 	
								create_and_store_token(BRACKETS_tok,  yytext, line_num); 
                   						print_message("BRACKETS_tok");  
                       						return 1;
 	     						}

{COMMENT}						{ 	
								create_and_store_token(COMMENT_tok,  yytext, line_num); 
                   						print_message("COMMENT_tok");  
                       						return 1;
 	     						}

{CURLY_BRACKETS}					{ 	
								create_and_store_token(CURLY_BRACKETS_tok,  yytext, line_num); 
                   						print_message("CURLY_BRACKETS_tok");  
                       						return 1;
 	     						}

[ \t]							{}

.							{ 	
                   						printf("Character '%s' in line: %d does not begin any legal token in the language\n", yytext, line_num);  
                       						return 1;
 	     						}

%%

int main(int argc, char **argv ){

++argv, --argc;  /* skip over program name */

if ( argc > 0 )
	yyin = fopen( argv[0], "r" );
else
	yyin = stdin;


if ( argc > 1 )
	yyout = fopen( argv[1], "w" );
else
	yyout = stdout;

while(yylex());
}

void print_message(char* token_type) {
	printf("Token of type '%s', lexeme: '%s', found in line: %d\n", token_type, yytext, line_num);
}