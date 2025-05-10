#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

int tableCount = 0;
int idCounter = 1;

char* generateSchemaKey(ASTNode* node) {
    if (strcmp(node->type, "pair") == 0) {
        if (node->strVal != NULL) {
            return strdup(node->strVal); 
        } else {
            printf("Warning: Null key in pair node\n");
            return strdup("defaultKey"); 
        }
    }
    return strdup("defaultKey");
}

Table* findOrCreateTable(const char* schemaKey) {
    for (int i = 0; i < tableCount; i++) {
        printf("%s\n", schemaKey);
        if (strcmp(tables[i]->schemaKey, schemaKey) == 0) {
            return tables[i]; 
        }
    }

    Table* newTable = (Table*)malloc(sizeof(Table));
    newTable->schemaKey = strdup(schemaKey);
    newTable->name = strdup(schemaKey); 
    newTable->rows = (Row**)malloc(sizeof(Row*) * 10);  
    newTable->rowCount = 0;
    newTable->rowCap = 10;

    tables[tableCount++] = newTable;  
    return newTable;
}

void addRow(Table* t, Row* row) {
    if (t->rowCount >= t->rowCap) {
        t->rowCap *= 2;
        t->rows = (Row**)realloc(t->rows, sizeof(Row*) * t->rowCap);
    }
    t->rows[t->rowCount++] = row;
    printf("Debug: Row added to table: %s, Row ID: %d\n", t->name, row->id); 
}

void walkAST(ASTNode* node, const char* parentTable, int parentId) {
    if (node == NULL || node->type == NULL) {
        printf("Debug: Node or node->type is NULL. Returning.\n");
        return;
    }

    printf("Debug: Walking node of type: %s\n", node->type);

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
        if (node->children == NULL || node->childCount != 1) {
            printf("Debug: Invalid pair. Skipping...\n");
            return;
        }

        char* schemaKey = generateSchemaKey(node);  
        Table* table = findOrCreateTable(schemaKey); 

        Row* row = (Row*)malloc(sizeof(Row));
        if (row == NULL) {
            printf("Debug: Memory allocation failed for row. Exiting.\n");
            exit(1);
        }

        row->id = idCounter++;
        row->parentId = parentId;
        row->tableName = strdup(table->name);  
        row->keyCount = 1;
        row->keys = (char**)malloc(sizeof(char*));
        row->keys[0] = strdup(node->strVal);
        row->values = (char**)malloc(sizeof(char*));

        ASTNode* valNode = node->children[0];
        int isComplex = strcmp(valNode->type, "object") == 0 || strcmp(valNode->type, "array") == 0;

        if (valNode->strVal) {
            row->values[0] = strdup(valNode->strVal);
        } else if (valNode->hasInt) {
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "%d", valNode->intVal);
            row->values[0] = strdup(buffer);
        } else if (valNode->hasBool) {
            row->values[0] = strdup(valNode->boolVal ? "true" : "false");
        } else if (isComplex) {
            row->values[0] = strdup(valNode->type);  // Save "object"/"array"
        } else {
            row->values[0] = strdup("null");
        }

        addRow(table, row);

        // Recurse with new parentId if complex type
        if (isComplex) {
            walkAST(valNode, table->name, row->id);  // use current row's ID as new parent
        }

        free(schemaKey);
    }
    else if (strcmp(node->type, "array") == 0) {
        printf("Debug: Array with %d children\n", node->childCount);

        if (node->childCount == 0) {
            printf("Debug: Empty array, skipping processing.\n");
        } else {
            for (int i = 0; i < node->childCount; i++) {
                ASTNode* child = node->children[i];
                if (child == NULL) {
                    printf("Debug: Found NULL child at index %d in array. Skipping.\n", i);
                    continue;
                }

                printf("Debug: Walking array element %d, type: %s\n", i, child->type);

                char* schemaKey = strdup(parentTable);  // ← Fix: use parent table name
                Table* table = findOrCreateTable(schemaKey);

                Row* row = (Row*)malloc(sizeof(Row));
                if (row == NULL) {
                    printf("Debug: Memory allocation failed for row. Exiting.\n");
                    exit(1);
                }

                row->id = idCounter++;
                row->parentId = parentId;
                row->tableName = strdup(table->name);
                row->keyCount = 1;
                row->keys = (char**)malloc(sizeof(char*));
                row->keys[0] = strdup("item");
                row->values = (char**)malloc(sizeof(char*));

                if (strcmp(child->type, "number") == 0) {
                    char buffer[20];
                    snprintf(buffer, sizeof(buffer), "%d", child->intVal);
                    row->values[0] = strdup(buffer);
                } else if (strcmp(child->type, "string") == 0) {
                    row->values[0] = strdup(child->strVal);
                } else if (strcmp(child->type, "bool") == 0) {
                    row->values[0] = strdup(child->boolVal ? "true" : "false");
                } else if (strcmp(child->type, "null") == 0) {
                    row->values[0] = strdup("null");
                } else {
                    row->values[0] = strdup(child->type);
                }

                addRow(table, row);
                free(schemaKey);
            }
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
