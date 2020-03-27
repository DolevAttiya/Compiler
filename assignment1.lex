%option noyywrap

%{
int line_num=1;
int flag=0;
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
COMMENT			\/\*(.|\n)*\*\/

%%

\n 								{
									line_num++;
									return 1;
								}

{INT}							{ 	
								create_and_store_token(INT_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type INT_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{FLOAT}							{ 	
								create_and_store_token(FLOAT_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type FLOAT_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{VOID}							{ 	
								create_and_store_token(VOID_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type VOID_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{IF}							{ 	
								create_and_store_token(IF_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type IF_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}	

{RETURN}						{ 	
								create_and_store_token(RETURN_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type RETURN_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{INT_NUM}						{ 	
								create_and_store_token(INT_NUM_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type INT_NUM_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{FLOAT_NUM}						{ 	
								create_and_store_token(FLOAT_NUM_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type FLOAT_NUM_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{AR_OP}							{ 
								create_and_store_token(AR_OP_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type AR_OP_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{REL_OP}						{ 	
								create_and_store_token(REL_OP_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type REL_OP_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{ASSIGNMENT_OP}						{ 	
								create_and_store_token(ASSIGNMENT_OP_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type ASSIGNMENT_OP_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{ID}							{ 	
								create_and_store_token(ID_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type ID_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}


{COMMA}							{ 	
								create_and_store_token(COMMA_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type COMMA_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{COLON}							{ 	
								create_and_store_token(COLON_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type COLON_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{SEMICOLON}						{ 	
								create_and_store_token(SEMICOLON_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type SEMICOLON_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{PARENTHESES}						{ 	
								create_and_store_token(PARENTHESES_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type PARENTHESES_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{BRACKETS}						{ 	
								create_and_store_token(BRACKETS_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type BRACKETS_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{COMMENT}						{ 	
								create_and_store_token(COMMENT_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type COMMENT_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

{CURLY_BRACKETS}					{ 	
								create_and_store_token(CURLY_BRACKETS_tok,  yytext, line_num); 
                   						printf("Line %d : found token of type CURLY_BRACKETS_tok , lexeme %s\n", line_num, yytext);  
                       						return 1;
 	     						}

[ \t]							{}

.							{ 	
                   						printf("Line %d : found illegal token , char: %s\n", line_num, yytext);  
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

yylex();
}
