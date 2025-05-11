#include <stdio.h>
#include <string.h>
#include <sys/stat.h>  // for mkdir
#include <errno.h>
#include "ast.h"
#include "symbol_table.h"
#include "csv-writer.h"  // Updated header with new function

extern int yyparse();
extern ASTNode* rootNode;

int main(int argc, char** argv) {
    int printAst = 0;
    int printSymbolTbl = 0;  
    int saveCsv = 0;  // Flag for saving to CSV

    const char* outputDir = "output";  // Default CSV directory

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--print-ast") == 0) {
            printAst = 1;
        } else if (strcmp(argv[i], "--print-symbol-table") == 0) {
            printSymbolTbl = 1;
        } else if (strcmp(argv[i], "--save-csv") == 0) {
            saveCsv = 1;
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

            if (mkdir(outputDir, 0777) == -1 && errno != EEXIST) {
                fprintf(stderr, "Error: Could not create output directory '%s'\n", outputDir);
                return 1;
            }

            saveSymbolTableToMultipleCSVs(outputDir);
        }

    } else {
        fprintf(stderr, "Parsing failed.\n");
        return 1;
    }

    return 0;
}
