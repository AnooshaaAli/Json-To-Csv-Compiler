%{
#include <stdio.h>
#include <stdlib.h>

int line = 1;
int col = 1;
%}

%%

"{"             { printf("TOKEN: LEFT_BRACE {\n"); col++; }
"}"             { printf("TOKEN: RIGHT_BRACE }\n"); col++; }
"["             { printf("TOKEN: LEFT_BRACKET [\n"); col++; }
"]"             { printf("TOKEN: RIGHT_BRACKET ]\n"); col++; }
":"             { printf("TOKEN: COLON :\n"); col++; }
","             { printf("TOKEN: COMMA ,\n"); col++; }

"true"          { printf("TOKEN: TRUE\n"); col += yyleng; }
"false"         { printf("TOKEN: FALSE\n"); col += yyleng; }
"null"          { printf("TOKEN: NULL\n"); col += yyleng; }

-?[0-9]+        { printf("TOKEN: NUMBER %s\n", yytext); col += yyleng; }
\"([^\"\\]|\\.)*\"  { printf("TOKEN: STRING %s\n", yytext); col += yyleng; }

[ \t]+          { col += yyleng; /* ignore spaces/tabs */ }
\n              { line++; col = 1; }

.               { printf("UNKNOWN: %s at line %d, col %d\n", yytext, line, col++); }

%%

int main(int argc, char **argv) {
    yylex();
    return 0;
}

int yywrap() {
    return 1;
}
