#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

int tableCount = 0;
int idCounter = 0;

// Helper function to report errors with context
static void report_error(const char* message, const char* context, const char* node_type) {
    fprintf(stderr, "Error: %s (Context: %s, Node Type: %s)\n", 
            message, context ? context : "unknown", node_type ? node_type : "unknown");
}

char* generateSchemaKey(ASTNode* node) {
    if (!node) {
        report_error("NULL node provided", "generateSchemaKey", NULL);
        return strdup("defaultKey");
    }
    if (!node->type) {
        report_error("Node has NULL type", "generateSchemaKey", NULL);
        return strdup("defaultKey");
    }
    if (strcmp(node->type, "pair") == 0) {
        if (node->strVal != NULL) {
            char* key = strdup(node->strVal);
            if (!key) {
                report_error("Memory allocation failed for schema key", "generateSchemaKey", node->type);
                return strdup("defaultKey");
            }
            return key;
        } else {
            report_error("Null key in pair node", "generateSchemaKey", node->type);
            return strdup("defaultKey");
        }
    }
    return strdup("defaultKey");
}

Table* findOrCreateTable(const char* schemaKey) {
    if (!schemaKey) {
        report_error("NULL schemaKey provided", "findOrCreateTable", NULL);
        schemaKey = "defaultKey";
    }

    for (int i = 0; i < tableCount; i++) {
        if (tables[i] && tables[i]->schemaKey && strcmp(tables[i]->schemaKey, schemaKey) == 0) {
            return tables[i];
        }
    }

    if (tableCount >= MAX_TABLES) {
        report_error("Maximum table limit reached", "findOrCreateTable", NULL);
        return NULL;
    }

    Table* newTable = (Table*)malloc(sizeof(Table));
    if (!newTable) {
        report_error("Memory allocation failed for new table", "findOrCreateTable", NULL);
        return NULL;
    }

    newTable->schemaKey = strdup(schemaKey);
    if (!newTable->schemaKey) {
        report_error("Memory allocation failed for schemaKey", "findOrCreateTable", NULL);
        free(newTable);
        return NULL;
    }

    newTable->name = strdup(schemaKey);
    if (!newTable->name) {
        report_error("Memory allocation failed for table name", "findOrCreateTable", NULL);
        free(newTable->schemaKey);
        free(newTable);
        return NULL;
    }

    newTable->rows = (Row**)malloc(sizeof(Row*) * 10);
    if (!newTable->rows) {
        report_error("Memory allocation failed for table rows", "findOrCreateTable", NULL);
        free(newTable->schemaKey);
        free(newTable->name);
        free(newTable);
        return NULL;
    }

    newTable->rowCount = 0;
    newTable->rowCap = 10;
    tables[tableCount++] = newTable;
    return newTable;
}

void addRow(Table* t, Row* row) {
    if (!t) {
        report_error("NULL table provided", "addRow", NULL);
        return;
    }
    if (!row) {
        report_error("NULL row provided", "addRow", NULL);
        return;
    }

    if (t->rowCount >= t->rowCap) {
        t->rowCap *= 2;
        Row** newRows = (Row**)realloc(t->rows, sizeof(Row*) * t->rowCap);
        if (!newRows) {
            report_error("Memory allocation failed for row array expansion", "addRow", NULL);
            return;
        }
        t->rows = newRows;
    }
    t->rows[t->rowCount++] = row;
}

void walkAST(ASTNode* node, const char* parentTable, int parentId) {
    if (!node || !node->type) {
        report_error("NULL node or node type", "walkAST", node ? node->type : NULL);
        return;
    }

    if (strcmp(node->type, "object") == 0) {
        for (int i = 0; i < node->childCount; i++) {
            ASTNode* child = node->children[i];
            if (!child) {
                report_error("NULL child node encountered", "walkAST:object", node->type);
                continue;
            }
            if (strcmp(child->type, "members") == 0) {
                for (int j = 0; j < child->childCount; j++) {
                    if (!child->children[j]) {
                        report_error("NULL member node encountered", "walkAST:members", child->type);
                        continue;
                    }
                    walkAST(child->children[j], parentTable, parentId);
                }
            }
        }
    }
    else if (strcmp(node->type, "pair") == 0) {
        if (!node->children || node->childCount != 1) {
            report_error("Invalid pair node (missing or incorrect children)", "walkAST:pair", node->type);
            return;
        }

        char* schemaKey = generateSchemaKey(node);
        if (!schemaKey) {
            report_error("Failed to generate schema key", "walkAST:pair", node->type);
            return;
        }

        Table* table = findOrCreateTable(schemaKey);
        if (!table) {
            report_error("Failed to find or create table", "walkAST:pair", node->type);
            free(schemaKey);
            return;
        }

        Row* row = (Row*)malloc(sizeof(Row));
        if (!row) {
            report_error("Memory allocation failed for row", "walkAST:pair", node->type);
            free(schemaKey);
            return;
        }

        row->id = idCounter++;
        row->parentId = parentId;
        row->tableName = strdup(table->name);
        if (!row->tableName) {
            report_error("Memory allocation failed for table name", "walkAST:pair", node->type);
            free(row);
            free(schemaKey);
            return;
        }

        row->keyCount = 1;
        row->keys = (char**)malloc(sizeof(char*));
        if (!row->keys) {
            report_error("Memory allocation failed for row keys", "walkAST:pair", node->type);
            free(row->tableName);
            free(row);
            free(schemaKey);
            return;
        }

        row->keys[0] = strdup(node->strVal ? node->strVal : "null");
        if (!row->keys[0]) {
            report_error("Memory allocation failed for row key", "walkAST:pair", node->type);
            free(row->keys);
            free(row->tableName);
            free(row);
            free(schemaKey);
            return;
        }

        row->values = (char**)malloc(sizeof(char*));
        if (!row->values) {
            report_error("Memory allocation failed for row values", "walkAST:pair", node->type);
            free(row->keys[0]);
            free(row->keys);
            free(row->tableName);
            free(row);
            free(schemaKey);
            return;
        }

        ASTNode* valNode = node->children[0];
        if (!valNode) {
            report_error("NULL value node in pair", "walkAST:pair", node->type);
            free(row->keys[0]);
            free(row->keys);
            free(row->values);
            free(row->tableName);
            free(row);
            free(schemaKey);
            return;
        }

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
            row->values[0] = strdup(valNode->type);
        } else {
            row->values[0] = strdup("null");
        }

        if (!row->values[0]) {
            report_error("Memory allocation failed for row value", "walkAST:pair", node->type);
            free(row->keys[0]);
            free(row->keys);
            free(row->values);
            free(row->tableName);
            free(row);
            free(schemaKey);
            return;
        }

        addRow(table, row);

        if (isComplex) {
            walkAST(valNode, table->name, row->id);
        }

        free(schemaKey);
    }
    else if (strcmp(node->type, "array") == 0) {
        if (!parentTable) {
            report_error("NULL parentTable for array", "walkAST:array", node->type);
            return;
        }

        for (int i = 0; i < node->childCount; i++) {
            ASTNode* child = node->children[i];
            if (!child) {
                report_error("NULL child in array", "walkAST:array", node->type);
                continue;
            }

            char* schemaKey = strdup(parentTable);
            if (!schemaKey) {
                report_error("Memory allocation failed for schema key", "walkAST:array", node->type);
                continue;
            }

            Table* table = findOrCreateTable(schemaKey);
            if (!table) {
                report_error("Failed to find or create table", "walkAST:array", node->type);
                free(schemaKey);
                continue;
            }

            Row* row = (Row*)malloc(sizeof(Row));
            if (!row) {
                report_error("Memory allocation failed for row", "walkAST:array", node->type);
                free(schemaKey);
                continue;
            }

            row->id = idCounter++;
            row->parentId = parentId;
            row->tableName = strdup(table->name);
            if (!row->tableName) {
                report_error("Memory allocation failed for table name", "walkAST:array", node->type);
                free(row);
                free(schemaKey);
                continue;
            }

            row->keyCount = 1;
            row->keys = (char**)malloc(sizeof(char*));
            if (!row->keys) {
                report_error("Memory allocation failed for row keys", "walkAST:array", node->type);
                free(row->tableName);
                free(row);
                free(schemaKey);
                continue;
            }

            row->keys[0] = strdup("item");
            if (!row->keys[0]) {
                report_error("Memory allocation failed for row key", "walkAST:array", node->type);
                free(row->keys);
                free(row->tableName);
                free(row);
                free(schemaKey);
                continue;
            }

            row->values = (char**)malloc(sizeof(char*));
            if (!row->values) {
                report_error("Memory allocation failed for row values", "walkAST:array", node->type);
                free(row->keys[0]);
                free(row->keys);
                free(row->tableName);
                free(row);
                free(schemaKey);
                continue;
            }

            if (strcmp(child->type, "number") == 0) {
                char buffer[20];
                snprintf(buffer, sizeof(buffer), "%d", child->intVal);
                row->values[0] = strdup(buffer);
            } else if (strcmp(child->type, "string") == 0) {
                row->values[0] = strdup(child->strVal ? child->strVal : "null");
            } else if (strcmp(child->type, "bool") == 0) {
                row->values[0] = strdup(child->boolVal ? "true" : "false");
            } else if (strcmp(child->type, "null") == 0) {
                row->values[0] = strdup("null");
            } else {
                row->values[0] = strdup(child->type);
            }

            if (!row->values[0]) {
                report_error("Memory allocation failed for row value", "walkAST:array", node->type);
                free(row->keys[0]);
                free(row->keys);
                free(row->values);
                free(row->tableName);
                free(row);
                free(schemaKey);
                continue;
            }

            addRow(table, row);
            free(schemaKey);
        }
    }
}

void printSymbolTables() {
    if (tableCount == 0) {
        printf("No tables to print.\n");
        return;
    }
    for (int i = 0; i < tableCount; i++) {
        Table* table = tables[i];
        if (!table) {
            report_error("NULL table encountered", "printSymbolTables", NULL);
            continue;
        }
        printf("Table: %s\n", table->name ? table->name : "unnamed");
        for (int j = 0; j < table->rowCount; j++) {
            Row* row = table->rows[j];
            if (!row) {
                report_error("NULL row encountered", "printSymbolTables", NULL);
                continue;
            }
            printf("  Row %d (Parent ID: %d):\n", row->id, row->parentId);
            for (int k = 0; k < row->keyCount; k++) {
                printf("    Key: %s, Value: %s\n", 
                       row->keys[k] ? row->keys[k] : "null", 
                       row->values[k] ? row->values[k] : "null");
            }
        }
        printf("\n");
    }
}