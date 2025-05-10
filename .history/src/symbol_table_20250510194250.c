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
    if (node == NULL || node->type == NULL) return;

    if (strcmp(node->type, "object") == 0) {
        for (int i = 0; i < node->childCount; i++) {
            ASTNode* child = node->children[i];
            if (strcmp(child->type, "members") == 0) {
                for (int j = 0; j < child->childCount; j++) {
                    walkAST(child->children[j], parentTable, parentId);
                }
            }
        }
    }

    else if (strcmp(node->type, "pair") == 0) {
        const char* key = node->strVal;
        ASTNode* valNode = node->children[0];

        // If value is an array of objects (e.g., items: [ {...}, {...} ])
        if (strcmp(valNode->type, "array") == 0 && valNode->childCount > 0 && strcmp(valNode->children[0]->type, "object") == 0) {
            // Determine schema name for array
            char* tableName = strdup(key);  // use key as table name
            Table* table = findOrCreateTable(tableName);

            for (int i = 0; i < valNode->childCount; i++) {
                ASTNode* obj = valNode->children[i];
                if (strcmp(obj->type, "object") == 0) {
                    Row* row = (Row*)malloc(sizeof(Row));
                    row->id = idCounter++;
                    row->parentId = parentId;
                    row->tableName = strdup(table->name);
                    row->keyCount = 0;
                    row->keys = NULL;
                    row->values = NULL;

                    // Parse object members into row keys/values
                    for (int j = 0; j < obj->childCount; j++) {
                        ASTNode* membersNode = obj->children[j];
                        for (int k = 0; k < membersNode->childCount; k++) {
                            ASTNode* pairNode = membersNode->children[k];
                            const char* pairKey = pairNode->strVal;
                            ASTNode* valueNode = pairNode->children[0];

                            row->keyCount++;
                            row->keys = realloc(row->keys, row->keyCount * sizeof(char*));
                            row->values = realloc(row->values, row->keyCount * sizeof(char*));
                            row->keys[row->keyCount - 1] = strdup(pairKey);

                            if (valueNode->strVal)
                                row->values[row->keyCount - 1] = strdup(valueNode->strVal);
                            else if (valueNode->hasInt) {
                                char buffer[20];
                                snprintf(buffer, sizeof(buffer), "%d", valueNode->intVal);
                                row->values[row->keyCount - 1] = strdup(buffer);
                            } else {
                                row->values[row->keyCount - 1] = strdup("null");
                            }
                        }
                    }

                    addRow(table, row);
                }
            }

            free(tableName);
        }

        // If value is object or primitive
        else {
            // Existing logic — treat as object or primitive
            char* schemaKey = generateSchemaKey(node);
            Table* table = findOrCreateTable(schemaKey);

            Row* row = (Row*)malloc(sizeof(Row));
            row->id = idCounter++;
            row->parentId = parentId;
            row->tableName = strdup(table->name);
            row->keyCount = 1;
            row->keys = (char**)malloc(sizeof(char*));
            row->keys[0] = strdup(key);
            row->values = (char**)malloc(sizeof(char*));

            if (valNode->strVal)
                row->values[0] = strdup(valNode->strVal);
            else if (valNode->hasInt) {
                char buffer[20];
                snprintf(buffer, sizeof(buffer), "%d", valNode->intVal);
                row->values[0] = strdup(buffer);
            } else if (valNode->hasBool) {
                row->values[0] = strdup(valNode->boolVal ? "true" : "false");
            } else if (strcmp(valNode->type, "object") == 0 || strcmp(valNode->type, "array") == 0) {
                row->values[0] = strdup(valNode->type);
            } else {
                row->values[0] = strdup("null");
            }

            addRow(table, row);

            if (strcmp(valNode->type, "object") == 0 || strcmp(valNode->type, "array") == 0) {
                walkAST(valNode, table->name, row->id);
            }

            free(schemaKey);
        }
    }

    else if (strcmp(node->type, "array") == 0) {
        printf("Debug: Found an array node. Creating parent row.\n");

        // Create a row for the array itself
        Table* arrayTable = findOrCreateTable((char*)parentTable);  // Use parentTable name (e.g., "items")
        Row* arrayRow = (Row*)malloc(sizeof(Row));
        if (!arrayRow) {
            printf("Error: Failed to allocate memory for array row.\n");
            return;
        }

        arrayRow->id = idCounter++;
        arrayRow->parentId = parentId;
        arrayRow->tableName = strdup(arrayTable->name);
        arrayRow->keyCount = 1;
        arrayRow->keys = (char**)malloc(sizeof(char*));
        arrayRow->values = (char**)malloc(sizeof(char*));

        arrayRow->keys[0] = strdup(parentTable);  // e.g., "items"
        arrayRow->values[0] = strdup("array");

        addRow(arrayTable, arrayRow);
        printf("Debug: Added array row with ID %d to table %s\n", arrayRow->id, arrayTable->name);

        // Recurse into each element of the array with new parentId
        for (int i = 0; i < node->childCount; i++) {
            walkAST(node->children[i], arrayTable->name, arrayRow->id);
        }
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
