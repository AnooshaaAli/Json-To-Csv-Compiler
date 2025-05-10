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

void walkAST(ASTNode* node, const char* parentTable, int parentId) {
    if (node == NULL || node->type == NULL) {
        printf("Debug: Node or node->type is NULL. Returning.\n");
        return;
    }

    printf("Debug: Walking node of type: %s\n", node->type);

    // Handle objects
    if (strcmp(node->type, "object") == 0) {
        for (int i = 0; i < node->childCount; i++) {
            ASTNode* child = node->children[i];
            if (strcmp(child->type, "members") == 0) {
                for (int j = 0; j < child->childCount; j++) {
                    walkAST(child->children[j], parentTable, parentId); // Recurse over members
                }
            }
        }
    }
    // Handle key-value pairs within the object
    else if (strcmp(node->type, "pair") == 0) {
        char* schemaKey = generateSchemaKey(node);
        Table* table = findOrCreateTable(schemaKey);

        Row* row = (Row*)malloc(sizeof(Row));
        row->id = idCounter++; // Auto increment id
        row->parentId = parentId; // Set parent ID for relationships
        row->tableName = strdup(table->name);
        row->keyCount = 1;
        row->keys = (char**)malloc(sizeof(char*));
        row->keys[0] = strdup(node->strVal); // The key name from the pair
        row->values = (char**)malloc(sizeof(char*));

        ASTNode* valNode = node->children[0];
        int isComplex = (strcmp(valNode->type, "object") == 0 || strcmp(valNode->type, "array") == 0);

        // Handle primitive types and complex types
        if (valNode->strVal) {
            row->values[0] = strdup(valNode->strVal);
        } else if (valNode->hasInt) {
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "%d", valNode->intVal);
            row->values[0] = strdup(buffer);
        } else if (valNode->hasBool) {
            row->values[0] = strdup(valNode->boolVal ? "true" : "false");
        } else if (isComplex) {
            row->values[0] = strdup(valNode->type);  // Save type (object/array)
        } else {
            row->values[0] = strdup("null");
        }

        addRow(table, row);

        // Recurse with new parentId if it's a complex type (array or object)
        if (isComplex) {
            walkAST(valNode, table->name, row->id); // Use current row's ID as new parent
        }

        free(schemaKey);
    }
    else if (strcmp(node->type, "array") == 0) {
        // Handle array nodes
        char* schemaKey = generateSchemaKey(node);
        Table* table = findOrCreateTable(schemaKey);

        Row* row = (Row*)malloc(sizeof(Row));
        row->id = idCounter++; // Auto increment ID for the array
        row->parentId = parentId; // Set parent ID as the array's parent
        row->tableName = strdup(table->name);
        row->keyCount = 1;
        row->keys = (char**)malloc(sizeof(char*));
        row->keys[0] = strdup("items");
        row->values = (char**)malloc(sizeof(char*));
        row->values[0] = strdup("array");  // Mark this as an array

        addRow(table, row);

        // Check for valid children in the array before recursing
        if (node->childCount > 0) {
            for (int i = 0; i < node->childCount; i++) {
                if (node->children[i] != NULL) {  // Ensure the child exists
                    walkAST(node->children[i], table->name, row->id);  // Recurse with array element
                } else {
                    printf("Debug: NULL child found in array, skipping.\n");
                }
            }
        } else {
            printf("Debug: Empty array, no children to process.\n");
        }

        free(schemaKey);
    }

}

void printSymbolTables() {
    if (tableCount == 0) {
        printf("Debug: No tables to print!\n");
    }
    for (int i = 0; i < tableCount; i++) {
        Table* table = tables[i];
        printf("Table: %s\n", table->name);
        for (int j = 0; j < table->rowCount; j++) {
            Row* row = table->rows[j];
            printf("  Row %d (Parent ID: %d):\n", row->id, row->parentId);
            for (int k = 0; k < row->keyCount; k++) {
                printf("    Key: %s, Value: %s\n", row->keys[k], row->values[k]);
            }
        }
        printf("\n");
    }
}
