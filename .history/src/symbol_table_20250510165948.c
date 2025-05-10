#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

int tableCount = 0;
int idCounter = 1;

Table* findTableByName(const char* tableName) {
    for (int i = 0; i < tableCount; i++) {
        if (strcmp(tables[i]->name, tableName) == 0) {
            return tables[i]; // Return the table if found
        }
    }
    return NULL; // Return NULL if not found
}

void addTable(Table* table) {
    // Increase the count of tables
    tableCount++;

    // Resize the symbolTables array to fit the new table
    tables = realloc(tables, tableCount * sizeof(Table*));
    if (symbolTables == NULL) {
        fprintf(stderr, "Error: Out of memory when adding a table\n");
        exit(1);
    }

    // Add the new table to the list
    symbolTables[tableCount - 1] = table;
}

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

void printSymbolTables() {
    for (int i = 0; i < tableCount; i++) {
        Table* t = tables[i];
        printf("Table: %s\n", t->name);

        // Header row
        printf("id");
        if (t->rows[0]->parentId != 0)
            printf(", %s_id", t->rows[0]->tableName); // foreign key
        for (int k = 0; k < t->rows[0]->keyCount; k++)
            printf(", %s", t->rows[0]->keys[k]);
        printf("\n");

        // Data rows
        for (int r = 0; r < t->rowCount; r++) {
            printf("%d", t->rows[r]->id);
            if (t->rows[r]->parentId != 0)
                printf(", %d", t->rows[r]->parentId);
            for (int k = 0; k < t->rows[r]->keyCount; k++) {
                printf(", %s", t->rows[r]->values[k]);
            }
            printf("\n");
        }
        printf("\n");
    }
}
