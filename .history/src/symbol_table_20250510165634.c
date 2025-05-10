#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

int tableCount = 0;
int idCounter = 1;

char* generateSchemaKey(ASTNode* node) {
    if (strcmp(node->type, "OBJECT") != 0) return NULL;
    char* keyStr = malloc(1024); keyStr[0] = '\0';
    for (int i = 0; i < node->childCount; i += 2) {
        strcat(keyStr, node->children[i]->strVal);
        if (i + 2 < node->childCount) strcat(keyStr, ",");
    }
    return keyStr;
}

// Add a row to a table
void addRow(Table* table, Row* row) {
    table->rowCount++;
    table->rows = realloc(table->rows, table->rowCount * sizeof(Row*));
    table->rows[table->rowCount - 1] = row;
}

// Find an existing table or create a new one
Table* findOrCreateTable(const char* tableName) {
    Table* table = findTableByName(tableName);  // Find table by name (if it exists)
    if (table) {
        return table; // Return existing table
    }

    // Create a new table if it doesn't exist
    table = malloc(sizeof(Table));
    table->name = strdup(tableName);
    table->rowCount = 0;
    table->rows = NULL;  // Initialize rows as NULL
    addTable(table); // Add to global list of tables
    return table;
}

void walkAST(ASTNode* node, const char* parentTable, int parentId) {
    if (!node) return;

    // Handle objects by adding them to the symbol table
    if (strcmp(node->type, "object") == 0) {
        char* schemaKey = generateSchemaKey(node);
        Table* table = findOrCreateTable(schemaKey);

        // Create a row for the object and add it to the symbol table
        Row* row = malloc(sizeof(Row));
        row->keyCount = node->childCount / 2;
        row->keys = malloc(sizeof(char*) * row->keyCount);
        row->values = malloc(sizeof(char*) * row->keyCount);
        row->id = idCounter++;
        row->parentId = parentId;
        row->tableName = table->name;

        for (int i = 0, j = 0; i < node->childCount; i += 2, j++) {
            ASTNode* keyNode = node->children[i];
            ASTNode* valNode = node->children[i + 1];

            row->keys[j] = keyNode->strVal;
            if (strcmp(valNode->type, "STRING") == 0) {
                row->values[j] = valNode->strVal;
            } else if (strcmp(valNode->type, "NUMBER") == 0) {
                row->values[j] = malloc(32);
                sprintf(row->values[j], "%d", valNode->intVal);
            } else if (strcmp(valNode->type, "BOOLEAN") == 0) {
                row->values[j] = malloc(6);
                sprintf(row->values[j], "%s", valNode->boolVal ? "true" : "false");
            } else {
                row->values[j] = strdup(""); // placeholder for nested objects/arrays
                walkAST(valNode, table->name, row->id);  // Recursively handle nested objects
            }
        }

        // Add row to the symbol table
        addRow(table, row);
        free(schemaKey);
    } 
    // Handle arrays
    else if (strcmp(node->type, "array") == 0) {
        for (int i = 0; i < node->childCount; i++) {
            walkAST(node->children[i], parentTable, parentId);
        }
    }
}

void printSymbolTable() {
    for (int i = 0; i < tableCount; i++) {
        Table* table = symbolTables[i];
        printf("Table: %s\n", table->name);
        for (int j = 0; j < table->rowCount; j++) {
            Row* row = table->rows[j];
            printf("Row %d: ", row->id);
            for (int k = 0; k < row->keyCount; k++) {
                printf("Key: %s, Value: %s | ", row->keys[k], row->values[k]);
            }
            printf("\n");
        }
    }
}
