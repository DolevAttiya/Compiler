%option noyywrap

%{
int counter=0;
%}

DIGIT    [0-9]
ALPHA	 [a-zA-Z]


%%

{ALPHA}({ALPHA}|{DIGIT})*       fprintf(yyout, "An identifier, count=%d: %s\n", ++counter, yytext);
[ \t\n] 			{}
.       			fprintf(yyout ,"Unrecognized character!: %s\n", yytext );

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



yylex();}
