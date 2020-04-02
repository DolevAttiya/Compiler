%option noyywrap
%x COMMENT

%{
#include "Token.h"
int line_num=1;
void print_message(char*);
char *eTokensStrings[]=
{
	"INT_tok",
	"FLOAT_tok",
	"VOID_tok",
	"IF_tok",
	"RETURN_tok",
	"NUM_tok",
	"OP_tok",
	"ID_tok",
	"COMMA_tok",
	"COLON_tok",
	"SEMICOLON_tok",
	"PARENTHESIS_OPEN_tok",
	"PARENTHESIS_CLOSE_tok",
	"BRACKET_OPEN_tok",
	"BRACKET_CLOSE_tok",
	"COMMENT_tok",
	"INT_NUM_tok",
	"FLOAT_NUM_tok",
	"ADD_OP_tok",
	"MUL_OP_tok",	
	"LESS_tok",
	"LESS_EQUAL_tok",
	"EQUAL_tok",
	"GREATER_tok",
	"GREATER_EQUAL_tok",
	"NOT_EQUAL_tok",	
	"ASSIGNMENT_OP_tok",
	"CURLY_BRACKET_OPEN_tok",
	"CURLY_BRACKET_CLOSE_tok",
	"EOF_tok"
};
%}

INT_NUM 				0|[1-9][0-9]*
FLOAT_NUM 				(0|[1-9][0-9]*)\.[0-9]+[eE][\+-]?(0|[1-9][0-9]*)
ADD_OP					\+
MUL_OP 					\*
LESS 					<
LESS_EQUAL 				<=
EQUAL					==
GREATER 				>
GREATER_EQUAL 			>=
NOT_EQUAL				!=
ASSIGNMENT_OP			=
ID 						[a-z]_?(([a-zA-Z0-9])+_?)*[a-zA-Z0-9]
INT 					int
FLOAT 					float
VOID 					void
IF 						if
RETURN 					return
COMMA 					,
COLON 					:
SEMICOLON 				;
PARENTHESIS_OPEN			\(
PARENTHESIS_CLOSE			\)
BRACKET_OPEN 			\[
BRACKET_CLOSE 			\]
CURLY_BRACKET_OPEN		\{
CURLY_BRACKET_CLOSE		\}

%%

"/*"  BEGIN(COMMENT);
<COMMENT>[^*\n]*                 
<COMMENT>\n 	line_num++;                    
<COMMENT>"*/" 	BEGIN(0);

\n 								{
									line_num++;
									return 1;
								}

<<EOF>>							{ 	
								create_and_store_token(EOF_tok,  yytext, line_num); 
									printf("Token of type '%s', found in line: %d\n",eTokensStrings[EOF_tok], line_num); 
                       						return 0;
 	     						}

{INT}							{ 	
								create_and_store_token(INT_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[INT_tok]);  
                       						return 1;
 	     						}

{FLOAT}							{ 	
								create_and_store_token(FLOAT_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[FLOAT_tok]);  
                       						return 1;
 	     						}

{VOID}							{ 	
								create_and_store_token(VOID_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[VOID_tok]);  
                       						return 1;
 	     						}

{IF}							{ 	
								create_and_store_token(IF_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[IF_tok]);  
                       						return 1;
 	     						}	

{RETURN}						{ 	
								create_and_store_token(RETURN_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[RETURN_tok]);  
                       						return 1;
 	     						}

{FLOAT_NUM}						{ 	
								create_and_store_token(FLOAT_NUM_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[FLOAT_NUM_tok]);  
                       						return 1;
 	     						}

{INT_NUM}						{ 	
								create_and_store_token(INT_NUM_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[INT_NUM_tok]);  
                       						return 1;
 	     						}

{ADD_OP}						{ 
								create_and_store_token(ADD_OP_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[ADD_OP_tok]);  
                       						return 1;
 	     						}

{MUL_OP}						{ 
								create_and_store_token(MUL_OP_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[MUL_OP_tok]);  
                       						return 1;
 	     						}

{LESS_EQUAL}					{ 	
								create_and_store_token(LESS_EQUAL_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[LESS_EQUAL_tok]);  
                       						return 1;
 	     						}

{GREATER_EQUAL}					{ 	
								create_and_store_token(GREATER_EQUAL_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[GREATER_EQUAL_tok]);  
                       						return 1;
 	     						}

{NOT_EQUAL}						{ 	
								create_and_store_token(NOT_EQUAL_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[NOT_EQUAL_tok]);  
                       						return 1;
 	     						}

{EQUAL}							{ 	
								create_and_store_token(EQUAL_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[EQUAL_tok]);  
                       						return 1;
 	     						}

{LESS}							{ 	
								create_and_store_token(LESS_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[LESS_tok]);  
                       						return 1;
 	     						}

{GREATER}						{ 	
								create_and_store_token(GREATER_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[GREATER_tok]);  
                       						return 1;
 	     						}


{ASSIGNMENT_OP}					{ 	
								create_and_store_token(ASSIGNMENT_OP_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[ASSIGNMENT_OP_tok]);  
                       						return 1;
 	     						}

{ID}							{ 	
								create_and_store_token(ID_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[ID_tok]);  
                       						return 1;
 	     						}


{COMMA}							{ 	
								create_and_store_token(COMMA_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[COMMA_tok]);  
                       						return 1;
 	     						}

{COLON}							{ 	
								create_and_store_token(COLON_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[COLON_tok]);  
                       						return 1;
 	     						}

{SEMICOLON}						{ 	
								create_and_store_token(SEMICOLON_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[SEMICOLON_tok]);  
                       						return 1;
 	     						}

{PARENTHESIS_OPEN}				{ 	
								create_and_store_token(PARENTHESIS_OPEN_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[PARENTHESIS_OPEN_tok]);  
                       						return 1;
 	     						}
															
{PARENTHESIS_CLOSE}				{ 	
								create_and_store_token(PARENTHESIS_CLOSE_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[PARENTHESIS_CLOSE_tok]);  
                       						return 1;
 	     						}

{BRACKET_OPEN}					{ 	
								create_and_store_token(BRACKET_OPEN_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[BRACKET_OPEN_tok]);  
                       						return 1;
 	     						}						
								
{BRACKET_CLOSE}					{ 	
								create_and_store_token(BRACKET_CLOSE_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[BRACKET_CLOSE_tok]);  
                       						return 1;
 	     						}

{CURLY_BRACKET_OPEN}			{ 	
								create_and_store_token(CURLY_BRACKET_OPEN_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[CURLY_BRACKET_OPEN_tok]);  
                       						return 1;
 	     						}
														
{CURLY_BRACKET_CLOSE}			{ 	
								create_and_store_token(CURLY_BRACKET_CLOSE_tok,  yytext, line_num); 
                   						print_message(eTokensStrings[CURLY_BRACKET_CLOSE_tok]);  
                       						return 1;
 	     						}

[ \t]							{}

.								{ 	
                   						printf("Character '%s' in line: %d does not begin any legal token in the language\n", yytext, line_num);  
                       						return 1;
 	     						}

%%

int main(int argc, char **argv ){

	++argv, --argc;  /* skip over program name */

	/*if ( argc > 0 )
		yyin = fopen( argv[0], "r" );
	else
		yyin = stdin;


	if ( argc > 1 )
		yyout = fopen( argv[1], "w" );
	else
		yyout = stdout;

	while(yylex());
}*/

	yyin = fopen("C:\\temp\\test1.txt", "r" );
	yyout = fopen("c:\\temp\\test1_206920282_313533374_205811797_lex.txt", "w" );
	while(yylex());
	fclose(yyin);
	fclose(yyout);
	yyin = fopen("C:\\temp\\test2.txt", "r" );
	yyout = fopen("c:\\temp\\test2_206920282_313533374_205811797_lex.txt", "w" );
	while(yylex());
}

void print_message(char* token_type) {
	printf("Token of type '%s', lexeme: '%s', found in line: %d\n", token_type, yytext, line_num);
}
