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
    char* inputFile = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--print-ast") == 0) {
            printAst = 1;
        } else if (strcmp(argv[i], "--print-symbol-table") == 0) {
            printSymbolTbl = 1;
        } else if (strcmp(argv[i], "--out-dir") == 0) {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                outDir = argv[++i];
            } else {
                fprintf(stderr, "Error: --out-dir requires a directory argument\n");
                return 1;
            }
        } else if (argv[i][0] != '-') {
            if (inputFile) {
                fprintf(stderr, "Error: Only one input file can be specified\n");
                return 1;
            }
            inputFile = argv[i];
        } else {
            fprintf(stderr, "Warning: Unknown argument '%s'\n", argv[i]);
        }
    }

    FILE* input = stdin;
    if (inputFile) {
        input = fopen(inputFile, "r");
        if (!input) {
            fprintf(stderr, "Error: Could not open input file '%s'\n", inputFile);
            return 1;
        }
        if (freopen(inputFile, "r", stdin) == NULL) {
            fprintf(stderr, "Error: Failed to redirect input from '%s'\n", inputFile);
            fclose(input);
            return 1;
        }
        fclose(input);
    }

    printf("Debug: Starting yyparse\n");
    int parseResult = yyparse();
    printf("Debug: yyparse completed, result=%d, rootNode=%p\n", parseResult, (void*)rootNode);

    if (parseResult == 0 && rootNode != NULL) {
        printf("Debug: Root node type=%s, childCount=%d\n",
               rootNode->type, rootNode->childCount);

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

        freeAST(rootNode);
        freeSymbolTables();
    } else {
        fprintf(stderr, "Parsing failed.\n");
        return 1;
    }

    return 0;
}