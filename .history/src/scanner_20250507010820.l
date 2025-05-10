%{
#include "parser.tab.h"
#include <string.h>
int col = 1;
%}

%%

[ \t\r]+        { col += yyleng; /* skip whitespace */ }
\n              { col = 1; return '\n'; }

\"(\\.|[^\"])*\" {
    yylval.strVal = strdup(yytext);
    col += yyleng;
    return STRING;
}

[0-9]+ {
    yylval.intVal = atoi(yytext);
    col += yyleng;
    return NUMBER;
}

"true"          { yylval.boolVal = 1; col += yyleng; return TRUE; }
"false"         { yylval.boolVal = 0; col += yyleng; return FALSE; }
"null"          { col += yyleng; return NULLTOK; }

"{"             { col++; return LEFT_BRACE; }
"}"             { col++; return RIGHT_BRACE; }
"["             { col++; return LEFT_BRACKET; }
"]"             { col++; return RIGHT_BRACKET; }
":"             { col++; return COLON; }
","             { col++; return COMMA; }

.               { fprintf(stderr, "Unknown character: %s\n", yytext); return -1; }

%%
