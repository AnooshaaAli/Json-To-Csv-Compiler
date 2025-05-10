%{
#include "parser.tab.h"  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int line = 1;
int col = 1;

extern YYSTYPE yylval;  
%}

%%

"{"             { col++; return LEFT_BRACE; }
"}"             { col++; return RIGHT_BRACE; }
"["             { col++; return LEFT_BRACKET; }
"]"             { col++; return RIGHT_BRACKET; }
":"             { col++; return COLON; }
","             { col++; return COMMA; }

"true"          { yylval.boolVal = 1; col += yyleng; return TRUE; }
"false"         { yylval.boolVal = 0; col += yyleng; return FALSE; }
"null"          { col += yyleng; return NULLTOK; }

-?[0-9]+        {
                    yylval.intVal = atoi(yytext);
                    col += yyleng;
                    return NUMBER;
                }

\"([^\"\\]|\\.)*\" {
                    yylval.strVal = strdup(yytext); 
                    col += yyleng;
                    return STRING;
                }

[ \t]+          { col += yyleng; 
\n              { line++; col = 1; }

.               { printf("UNKNOWN CHARACTER: %s at line %d, col %d\n", yytext, line, col++); }

%%

int yywrap() { return 1; }