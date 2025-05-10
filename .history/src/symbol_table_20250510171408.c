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

Table* findOrCreateTable(const char* schemaKey) {
        printf("/ntablecount: %d", tableCount);
    // Check if the table already exists
    for (int i = 0; i < tableCount; i++) {
        if (strcmp(tables[i]->schemaKey, schemaKey) == 0) {
            return tables[i];  // Table already exists, return it
        }
    }

    // If the table doesn't exist, create a new one
    Table* newTable = (Table*)malloc(sizeof(Table));
    newTable->schemaKey = strdup(schemaKey);
    newTable->name = strdup(schemaKey);  // You can customize this if needed
    newTable->rows = (Row**)malloc(sizeof(Row*) * 10);  // Initial capacity for rows
    newTable->rowCount = 0;
    newTable->rowCap = 10;

    // Add the new table to the list of tables
    tables[tableCount++] = newTable;  // Increment tableCount here
    return newTable;
}

void addRow(Table* t, Row* row) {
    if (t->rowCount >= t->rowCap) {
        t->rowCap *= 2;
        t->rows = (Row**)realloc(t->rows, sizeof(Row*) * t->rowCap);
    }
    t->rows[t->rowCount++] = row;
    printf("Debug: Row added to table: %s, Row ID: %d\n", t->name, row->id);  // Debugging line
}

// Walk the AST and group data into tables
void walkAST(ASTNode* node, const char* parentTable, int parentId) {
    if (node == NULL) {
        printf("Debug: Node is NULL. Returning.\n");
        return;
    }

    printf("Debug: Walking node of type: %s\n", node->type);

    // Example: If node is an object, walk its members (key-value pairs)
    if (strcmp(node->type, "object") == 0) {
        printf("Debug: Found an object node. Walking its members.\n");
        for (int i = 0; i < node->childCount; i++) {
            walkAST(node->children[i], parentTable, parentId);
        }
    }
    // If node is a pair (key-value), add it to the table
    else if (strcmp(node->type, "pair") == 0) {
        printf("Debug: Found a pair node with key: %s\n", node->strVal);
        
        char* schemaKey = generateSchemaKey(node);
        printf("Debug: Generated schema key: %s\n", schemaKey);

        Table* table = findOrCreateTable(schemaKey);  // Find or create the table
        printf("Debug: Table name: %s\n", table->name);

        // Create a new row and set its values
        Row* row = (Row*)malloc(sizeof(Row));
        row->id = idCounter++;  // Increment the id counter for each row
        row->parentId = parentId;  // Link to the parent table if applicable
        row->tableName = strdup(table->name);

        row->keyCount = 1;  // Single key in this example
        row->keys = (char**)malloc(sizeof(char*));
        row->keys[0] = strdup(node->strVal);  // Key from the pair

        row->values = (char**)malloc(sizeof(char*));
        row->values[0] = strdup(node->children[0]->strVal);  // Value from the pair

        // Debugging row details before adding
        printf("Debug: Row created with ID: %d\n", row->id);
        printf("Debug: Row key: %s, Row value: %s\n", row->keys[0], row->values[0]);

        // Add the row to the table
        addRow(table, row);

        printf("Debug: Row added to table: %s\n", table->name);
        free(schemaKey);
    }
    // If node is an array, process its elements (values)
    else if (strcmp(node->type, "array") == 0) {
        printf("Debug: Found an array node. Walking its elements.\n");
        for (int i = 0; i < node->childCount; i++) {
            walkAST(node->children[i], parentTable, parentId);
        }
    }
}

void printSymbolTables() {
    printf("Debug: Printing all symbol tables\n");
    if (tableCount == 0) {
        printf("Debug: No tables to print!\n");
    }
    for (int i = 0; i < tableCount; i++) {
        Table* table = tables[i];
        printf("Debug: Printing table: %s\n", table->name);
        printf("Table: %s\n", table->name);
        for (int j = 0; j < table->rowCount; j++) {
            Row* row = table->rows[j];
            printf("  Debug: Printing row %d (Parent ID: %d)\n", row->id, row->parentId);
            printf("  Row %d (Parent ID: %d):\n", row->id, row->parentId);
            for (int k = 0; k < row->keyCount; k++) {
                printf("    Key: %s, Value: %s\n", row->keys[k], row->values[k]);
            }
        }
        printf("\n");
    }
}
