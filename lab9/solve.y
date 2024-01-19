%{
#include <stdio.h>
#include <stdlib.h>

#define YYDEBUG 1
%}

%token LET;
%token IF;
%token ELSE;
%token DO;
%token FOR;
%token I32;
%token BOOL;
%token IN;
%token READ;
%token PRINT;
%token TRUE;
%token FALSE;

%token IDENTIFIER;

%token MINUS
%token PLUS
%token MINUS
%token MULTIPLY
%token DIVIDIE
%token ASSIGN
%token LESS
%token GREATER
%token LESS_EQUAL
%token GREATER_EQUAL
%token EQUAL
%token NOT_EQUAL
%token MODULU
%token BANG
%token AMPERSAND
%token SEMICOLUMN
%token CURLY_OPEN
%token CURLY_CLOSED
%token ROUND_OPEN
%token ROUND_CLOSED
%token SQAURE_OPEN
%token SQAURE_CLOSED
%token COMMA

%token INTEGER_CONSTANT 
%token BOOL_CONSTANT   

%start program 

%%

program : statement+ ;
statement : variable_declaration |
	        block |
	        if |
	        for |
	        read |
	        print |
	        expression_statement ;
variable_declaration : LET IDENTIFIER (type | type? ASSIGN expression ) SEMICOLON ;
type : I32 | BOOL | array_type ;
array_type : SQUARE_OPEN SQUARE_CLOSED type ;
if : IF expression block ;
block : brace_block |
	    do_block;
do_block : DO statement ;
brace_block : CURLY_OPEN statement* CURLY_CLOSED ;
for : normal_for |
	  for_in;
for_in : FOR identifier IN identifier block ;
normal_for : FOR expression block ;
print : PRINT ROUND_OPEN variable_expression ROUND_CLOSED SEMICOLON ;
read : READ ROUND_OPEN reference_expression ROUND_CLOSED SEMICOLON ;
expression_statment : expression SEMICOLON ;
expression : prefix_expression |
	         variable_expression |
	         constant_expression |
	         infix_expression |
	         group_expression |
	         array_expression ;
prefix_expression   : prefix_operator expression ;
variable_expression : identifier ;
constant_expression : integer_constant |
	                  bool_constant ; 
infix_experssion    : expression infix_operator experssion ;
group_expression    : ROUND_OPEN expression ROUND_CLOSED ;
array_expression    : array_type CURLY_OPEN (expression (COMMA expression)*)? CURLY_CLOSED;

%%
yyerror(char *s)
{	
	printf("%s\n",s);
}

extern FILE *yyin;

main(int argc, char **argv)
{
	if(argc>1) yyin =  fopen(argv[1],"r");
	if(!yyparse()) fprintf(stderr, "\tOK\n");
} 
