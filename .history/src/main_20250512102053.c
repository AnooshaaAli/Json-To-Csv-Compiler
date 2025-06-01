#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "symbol_table.h"
#include "csv-writer.h"

extern int yyparse();
extern ASTNode* rootNode;

int main(int argc, char** argv) {
    int printAst = 0;
    int printSymbolTbl = 0;
    char* outDir = NULL;

    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--print-ast") == 0) {
            printAst = 1;
        } else if (strcmp(argv[i], "--print-symbol-table") == 0) {
            printSymbolTbl = 1;
        } else if (strcmp(argv[i], "--out-dir") == 0 && i + 1 < argc) {
            outDir = argv[++i];
        } else {
            fprintf(stderr, "Warning: Unknown argument '%s'\n", argv[i]);
        }
    }

    if (yyparse() == 0 && rootNode != NULL) {
        // Build symbol table
        walkAST(rootNode, NULL, 0);

        if (printAst) {
            printf("\n------------------------- AST Structure -------------------------\n\n");
            printAST(rootNode, 0, 1);
            printf("\n");
        }

        if (printSymbolTbl) {
            printf("\n------------------------- Symbol Table -------------------------\n\n");
            printSymbolTables();
        }

        if (outDir) {
            saveSymbolTableToCSV(outDir);
        }

        // Clean up
        freeSymbolTables();
    } else {
        fprintf(stderr, "Parsing failed.\n");
        return 1;
    }

    return 0;
}