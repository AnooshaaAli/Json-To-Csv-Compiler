%{
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"  

extern int yylex();
extern int yylineno;
extern int col;

void yyerror(const char *s);

ASTNode* createNode(const char* type);
ASTNode* createStrNode(const char* type, char* val);
ASTNode* createIntNode(const char* type, int val);
ASTNode* createBoolNode(const char* type, int val);

ASTNode* rootNode = NULL;

%}

%union {
    char* strVal;
    int intVal;
    int boolVal;
    struct ASTNode* ast;  
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
        printf("JSON parsed successfully!\n");
        rootNode = $1; // 
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
    pair                { $$ = createNode("members"); addChild($$, $1); }
  | members COMMA pair  { $$ = $1; addChild($$, $3); }
;

pair: 
    STRING COLON value {
    $$ = createStrNode("pair", $1);
    addChild($$, $3);
};


array:
      LEFT_BRACKET elements RIGHT_BRACKET { $$ = $2; }
    | LEFT_BRACKET RIGHT_BRACKET          { $$ = createNode("empty_array"); }
;

elements:
      value                    { $$ = $1; }
    | elements COMMA value     { $$ = $1; 
;

%%

void yyerror(const char *s) {
    fprintf(stderr, "❌ Parse error at line %d, col %d: %s\n", yylineno, col, s);
}
