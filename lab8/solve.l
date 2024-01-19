%{ 	 
#include <math.h> 
%} 

/*
operator = prefix_operator | infix_operator.
prefix_operator = "!" | "&".
infix_operator = "+" | "-" | "*" | "/" | "=" | "<" | ">" | "<=" | ">=" | "!=" | "==" | "%".
separator = ";" | "{" | "}" | "(" | ")" | "[" | "]" | ",".
reserved_words = "let" | "if" | "else" | "do" | "for" | "i32" | "bool" | "in" | "read" | "print" | "true" | "false".
any = (* anything in the alphabet *).
letter     = "A" | ... | "Z" | "a" | ... | "z".
character  = letter | "_".
non_zero_digit = "1" | ... | "9".
digit      = "0" | non_zero_digit.
identifier = character {character | digit} (* no more than 256 characters *).
integer_constant = 0 | ["+" | "-"] non_zero_digit {digit}.
bool_constant = "true" | "false".
*/

DIGIT 		     [0-9] 
IDENTIFIER 		 [_a-zA-Z][_a-zA-Z0-9]* 
PREFIX_OPERATOR  [!&]
INFIX_OPERATOR   (+ | - | * | / | = | < | > | <= | >= | != | == | %)
OPERATOR         ({PERFIX_OPERATOR} | {INFIX_OPERATOR})
SERPARATOR       (; | { | } | \( | \) | \[ | \] | ,)
RESERVERD_WORD   (let | if | else | do | for | i32 | bool | in | read | print | true | false) 
INTEGER_CONSTANT (0 | (+ | \-) [1-9][0-9]*)
BOOL_CONSTANT    (true | false)

%% 

%% 
main( argc, argv ) 
int argc; 
char **argv; 
{ 
    ++argv, --argc; /* skip over program name */ 
    if ( argc > 0 ) 
    yyin = fopen( argv[0], "r" ); 
    else 
     yyin = stdin; 
    yylex(); 
} 

