#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "symbol_table.h"
#include "csv-writer.h"  // Include the new header

extern int yyparse();
extern ASTNode* rootNode;

int main(int argc, char** argv) {
    int printAst = 0;
    int printSymbolTbl = 0;  
    int saveCsv = 0;  // Flag for saving to CSV

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--print-ast") == 0) {
            printAst = 1;
        } else if (strcmp(argv[i], "--print-symbol-table") == 0) {
            printSymbolTbl = 1;
        } else if (strcmp(argv[i], "--out-dir") == 0) {
            saveCsv = 1;  // Enable CSV output
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
            printSymbolTables();
        }

        if (saveCsv) {
            saveSymbolTableToCSV("symbol_table.csv");  // Save symbol table to CSV
        }

    } else {
        fprintf(stderr, "Parsing failed.\n");
    }

    return 0;
}