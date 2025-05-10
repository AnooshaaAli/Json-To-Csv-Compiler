#include <stdio.h>
#include <string.h>
#include "ast.h"

extern int yyparse();
extern ASTNode* rootNode;

int main(int argc, char** argv) {
    int printAst = 0;
    int printSymbolTbl = 0;  

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--print-ast") == 0) {
            printAst = 1;
        } else if (strcmp(argv[i], "--print-symbol-table") == 0) {
            printSymbolTbl = 1;
        }
    }

    if (yyparse() == 0) {
        if (printAst && rootNode != NULL) {
            printf(" \n ------------------------- AST Structure ------------------------- \n\n");
            printAST(rootNode, 0, 1);
            printf("\n");
        }

        if (printSymbolTbl) {
            printf(" \n ------------------------- Symbol Table ------------------------- \n\n");
            printTables(); 
            printf("\n");
        }

    } else {
        fprintf(stderr, "Parsing failed.\n");
    }

    return 0;
}
