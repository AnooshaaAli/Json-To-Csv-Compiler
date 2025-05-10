#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

int tableCount = 0;
int idCounter = 1;

// Generate a schema key based on the AST node.
char* generateSchemaKey(ASTNode* node) {
    // Assuming the node type is "pair" and it stores key-value pairs
    if (strcmp(node->type, "pair") == 0) {
        // We will generate a schema key based on the key (strVal) of the pair
        return strdup(node->strVal);  // Return a copy of the key as the schema key
    }
    return NULL;
}

// Find or create a table based on the schemaKey
Table* findOrCreateTable(const char* schemaKey) {
    // Check if the table already exists
    for (int i = 0; i < tableCount; i++) {
        if (strcmp(tables[i]->schemaKey, schemaKey) == 0) {
            return tables[i];  // Return the existing table
        }
    }

    // If no table is found, create a new one
    Table* newTable = (Table*)malloc(sizeof(Table));
    newTable->schemaKey = strdup(schemaKey);  // Copy the schemaKey
    newTable->name = strdup(schemaKey);  // Use the schemaKey as the table name
    newTable->rows = NULL;  // Initially no rows
    newTable->rowCount = 0;
    newTable->rowCap = 10;  // Start with a capacity of 10 rows

    // Add the new table to the global tables list
    tables[tableCount++] = newTable;

    return newTable;
}

