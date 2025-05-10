%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"  // ✅ Make sure this line is here before using ASTNode

extern int yylex();
extern int yylineno;
extern int col;

void yyerror(const char *s);
%}


// Factory function (you can define later)
ASTNode* createNode(const char* type);
ASTNode* createStrNode(const char* type, char* val);
ASTNode* createIntNode(const char* type, int val);
ASTNode* createBoolNode(const char* type, int val);

%}

%union {
    char* strVal;
    int intVal;
    int boolVal;
    ASTNode* ast; 
}

%token <strVal> STRING
%token <intVal> NUMBER
%token <boolVal> TRUE FALSE
%token NULLTOK
%token LEFT_BRACE RIGHT_BRACE LEFT_BRACKET RIGHT_BRACKET COLON COMMA

%type <ast> json value object array members pair elements

%start json

%%

json:
    value {
        printf("✅ JSON parsed successfully!\n");
        // You may traverse AST here
    }
;

value:
      STRING        { $$ = createStrNode("string", $1); }
    | NUMBER        { $$ = createIntNode("number", $1); }
    | TRUE          { $$ = createBoolNode("bool", 1); }
    | FALSE         { $$ = createBoolNode("bool", 0); }
    | NULLTOK       { $$ = createNode("null"); }
    | object        { $$ = $1; }
    | array         { $$ = $1; }
;

object:
      LEFT_BRACE members RIGHT_BRACE { $$ = $2; }
    | LEFT_BRACE RIGHT_BRACE         { $$ = createNode("empty_object"); }
;

members:
      pair                   { $$ = $1; }
    | members COMMA pair     { $$ = $1; /* Add $3 to $$->children in AST stage */ }
;

pair:
      STRING COLON value     { $$ = createStrNode("pair", $1); /* attach $3 later */ }
;

array:
      LEFT_BRACKET elements RIGHT_BRACKET { $$ = $2; }
    | LEFT_BRACKET RIGHT_BRACKET          { $$ = createNode("empty_array"); }
;

elements:
      value                    { $$ = $1; }
    | elements COMMA value     { $$ = $1; /* Add $3 to $$->children in AST stage */ }
;

%%

void yyerror(const char *s) {
    fprintf(stderr, "❌ Parse error at line %d, col %d: %s\n", yylineno, col, s);
}
