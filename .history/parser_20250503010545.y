%{
#include <stdio.h>
#include <stdlib.h>

extern int yylex();
extern int yyparse();
extern int yylineno;
extern int col;

void yyerror(const char *s);
%}

%token STRING NUMBER TRUE FALSE NULLTOK
%token LEFT_BRACE RIGHT_BRACE LEFT_BRACKET RIGHT_BRACKET COLON COMMA

%start json

%%

json:
      value { printf("JSON parsed successfully\n"); }
    ;

value:
      STRING
    | NUMBER
    | TRUE
    | FALSE
    | NULLTOK
    | object
    | array
    ;

object:
      LEFT_BRACE members RIGHT_BRACE
    | LEFT_BRACE RIGHT_BRACE
    ;

members:
      pair
    | members COMMA pair
    ;

pair:
      STRING COLON value
    ;

array:
      LEFT_BRACKET elements RIGHT_BRACKET
    | LEFT_BRACKET RIGHT_BRACKET
    ;

elements:
      value
    | elements COMMA value
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Parse error at line %d, col %d: %s\n", yylineno, col, s);
}
