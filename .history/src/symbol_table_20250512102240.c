#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

Table* tables[MAX_TABLES] = {0}; // Definition of the tables array
int tableCount = 0;
int idCounter = 1; // Start IDs at 1 per assignment examples

void report_error(const char* message, const char* context, const char* node_type) {
    fprintf(stderr, "Error: %s (Context: %s, Node Type: %s)\n",
            message, context ? context : "unknown", node_type ? node_type : "unknown");
}

char* generateSchemaKey(ASTNode* node) {
    if (!node || !node->type) {
        report_error("NULL node or type", "generateSchemaKey", NULL);
        return strdup("default");
    }

    if (strcmp(node->type, "object") != 0) {
        return strdup("default");
    }

    // Collect all keys in the object
    char* keys = NULL;
    size_t keys_len = 0;
    for (int i = 0; i < node->childCount; i++) {
        ASTNode* child = node->children[i];
        if (child && strcmp(child->type, "pair") == 0 && child->strVal) {
            size_t new_len = keys_len + strlen(child->strVal) + 2; // +2 for comma and null
            char* new_keys = realloc(keys, new_len);
            if (!new_keys) {
                report_error("Memory allocation failed", "generateSchemaKey", node->type);
                free(keys);
                return strdup("default");
            }
            keys = new_keys;
            if (keys_len > 0) {
                strcat(keys, ",");
                keys_len++;
            }
            strcat(keys, child->strVal);
            keys_len = strlen(keys);
        }
    }

    if (!keys) {
        return strdup("default");
    }
    return keys;
}

Table* findOrCreateTable(const char* schemaKey, const char* tableName, const char* parentName) {
    if (!schemaKey) {
        report_error("NULL schemaKey", "findOrCreateTable", NULL);
        schemaKey = "default";
    }

    // Check if table exists with same schemaKey
    for (int i = 0; i < tableCount; i++) {
        if (tables[i] && tables[i]->schemaKey && strcmp(tables[i]->schemaKey, schemaKey) == 0) {
            return tables[i];
        }
    }

    if (tableCount >= MAX_TABLES) {
        report_error("Maximum table limit reached", "findOrCreateTable", NULL);
        return NULL;
    }

    Table* table = malloc(sizeof(Table));
    if (!table) {
        report_error("Memory allocation failed", "findOrCreateTable", NULL);
        return NULL;
    }

    table->schemaKey = strdup(schemaKey);
    table->name = strdup(tableName ? tableName : schemaKey);
    table->parentName = parentName ? strdup(parentName) : NULL;
    table->rows = malloc(sizeof(Row*) * 10);
    if (!table->schemaKey || !table->name || !table->rows || (parentName && !table->parentName)) {
        report_error("Memory allocation failed", "findOrCreateTable", NULL);
        free(table->schemaKey);
        free(table->name);
        free(table->parentName);
        free(table->rows);
        free(table);
        return NULL;
    }

    table->rowCount = 0;
    table->rowCap = 10;
    tables[tableCount++] = table;
    return table;
}

void addRow(Table* t, Row* row) {
    if (!t || !row) {
        report_error("NULL table or row", "addRow", NULL);
        return;
    }

    if (t->rowCount >= t->rowCap) {
        t->rowCap *= 2;
        Row** newRows = realloc(t->rows, sizeof(Row*) * t->rowCap);
        if (!newRows) {
            report_error("Memory allocation failed", "addRow", NULL);
            return;
        }
        t->rows = newRows;
    }
    t->rows[t->rowCount++] = row;
}

void walkAST(ASTNode* node, const char* parentTable, int parentId) {
    if (!node || !node->type) {
        report_error("NULL node or type", "walkAST", NULL);
        return;
    }

    if (strcmp(node->type, "object") == 0) {
        // Generate schema key based on object keys
        char* schemaKey = generateSchemaKey(node);
        if (!schemaKey) {
            report_error("Failed to generate schema key", "walkAST", node->type);
            return;
        }

        // Determine table name (use parentTable or a default)
        char* tableName = parentTable ? strdup(parentTable) : strdup("objects");
        if (!tableName) {
            report_error("Memory allocation failed for table name", "walkAST", node->type);
            free(schemaKey);
            return;
        }

        Table* table = findOrCreateTable(schemaKey, tableName, parentTable);
        if (!table) {
            report_error("Failed to create table", "walkAST", node->type);
            free(schemaKey);
            free(tableName);
            return;
        }

        // Create a row for the object
        Row* row = malloc(sizeof(Row));
        if (!row) {
            report_error("Memory allocation failed for row", "walkAST", node->type);
            free(schemaKey);
            free(tableName);
            return;
        }

        row->id = idCounter++;
        row->parentId = parentId;
        row->tableName = strdup(table->name);
        row->keyCount = 0;
        row->keys = NULL;
        row->values = NULL;

        // Collect keys and values
        for (int i = 0; i < node->childCount; i++) {
            ASTNode* child = node->children[i];
            if (child && strcmp(child->type, "pair") == 0 && child->strVal && child->childCount == 1) {
                ASTNode* valNode = child->children[0];
                if (!valNode) continue;

                // Resize keys and values arrays
                row->keys = realloc(row->keys, sizeof(char*) * (row->keyCount + 1));
                row->values = realloc(row->values, sizeof(char*) * (row->keyCount + 1));
                if (!row->keys || !row->values) {
                    report_error("Memory allocation failed for keys/values", "walkAST", node->type);
                    free(row->keys);
                    free(row->values);
                    free(row->tableName);
                    free(row);
                    free(schemaKey);
                    free(tableName);
                    return;
                }

                // Store key
                row->keys[row->keyCount] = strdup(child->strVal);
                if (!row->keys[row->keyCount]) {
                    report_error("Memory allocation failed for key", "walkAST", node->type);
                    free(row->keys);
                    free(row->values);
                    free(row->tableName);
                    free(row);
                    free(schemaKey);
                    free(tableName);
                    return;
                }

                // Store value
                if (strcmp(valNode->type, "object") == 0 || strcmp(valNode->type, "array") == 0) {
                    row->values[row->keyCount] = strdup("");
                    walkAST(valNode, child->strVal, row->id); // Recurse with key as new parent table
                } else {
                    if (valNode->strVal) {
                        row->values[row->keyCount] = strdup(valNode->strVal);
                    } else if (valNode->hasInt) {
                        char buffer[32];
                        snprintf(buffer, sizeof(buffer), "%d", valNode->intVal);
                        row->values[row->keyCount] = strdup(buffer);
                    } else if (valNode->hasBool) {
                        row->values[row->keyCount] = strdup(valNode->boolVal ? "true" : "false");
                    } else {
                        row->values[row->keyCount] = strdup("");
                    }
                }

                if (!row->values[row->keyCount]) {
                    report_error("Memory allocation failed for value", "walkAST", node->type);
                    free(row->keys[row->keyCount]);
                    free(row->keys);
                    free(row->values);
                    free(row->tableName);
                    free(row);
                    free(schemaKey);
                    free(tableName);
                    return;
                }

                row->keyCount++;
            }
        }

        addRow(table, row);
        free(schemaKey);
        free(tableName);
    }
    else if (strcmp(node->type, "array") == 0) {
        if (!parentTable) {
            report_error("NULL parentTable for array", "walkAST", node->type);
            return;
        }

        // Determine if array contains objects or scalars
        int isObjectArray = 0;
        for (int i = 0; i < node->childCount; i++) {
            if (node->children[i] && strcmp(node->children[i]->type, "object") == 0) {
                isObjectArray = 1;
                break;
            }
        }

        if (isObjectArray) {
            // R2: Array of objects -> child table
            for (int i = 0; i < node->childCount; i++) {
                ASTNode* child = node->children[i];
                if (child && strcmp(child->type, "object") == 0) {
                    char* schemaKey = generateSchemaKey(child);
                    Table* table = findOrCreateTable(schemaKey, parentTable, parentTable);
                    if (!table) {
                        free(schemaKey);
                        continue;
                    }

                    Row* row = malloc(sizeof(Row));
                    if (!row) {
                        free(schemaKey);
                        continue;
                    }

                    row->id = idCounter++;
                    row->parentId = parentId;
                    row->tableName = strdup(table->name);
                    row->keyCount = 0;
                    row->keys = NULL;
                    row->values = NULL;

                    // Add seq column for array index
                    row->keys = realloc(row->keys, sizeof(char*) * (row->keyCount + 1));
                    row->values = realloc(row->values, sizeof(char*) * (row->keyCount + 1));
                    row->keys[row->keyCount] = strdup("seq");
                    char seq_buffer[32];
                    snprintf(seq_buffer, sizeof(seq_buffer), "%d", i);
                    row->values[row->keyCount] = strdup(seq_buffer);
                    row->keyCount++;

                    // Process object fields
                    for (int j = 0; j < child->childCount; j++) {
                        ASTNode* pair = child->children[j];
                        if (pair && strcmp(pair->type, "pair") == 0 && pair->strVal && pair->childCount == 1) {
                            ASTNode* valNode = pair->children[0];
                            row->keys = realloc(row->keys, sizeof(char*) * (row->keyCount + 1));
                            row->values = realloc(row->values, sizeof(char*) * (row->keyCount + 1));
                            row->keys[row->keyCount] = strdup(pair->strVal);
                            if (strcmp(valNode->type, "object") == 0 || strcmp(valNode->type, "array") == 0) {
                                row->values[row->keyCount] = strdup("");
                                walkAST(valNode, pair->strVal, row->id);
                            } else {
                                if (valNode->strVal) {
                                    row->values[row->keyCount] = strdup(valNode->strVal);
                                } else if (valNode->hasInt) {
                                    char buffer[32];
                                    snprintf(buffer, sizeof(buffer), "%d", valNode->intVal);
                                    row->values[row->keyCount] = strdup(buffer);
                                } else if (valNode->hasBool) {
                                    row->values[row->keyCount] = strdup(valNode->boolVal ? "true" : "false");
                                } else {
                                    row->values[row->keyCount] = strdup("");
                                }
                            }
                            row->keyCount++;
                        }
                    }

                    addRow(table, row);
                    free(schemaKey);
                }
            }
        } else {
            // R3: Array of scalars -> junction table
            char* schemaKey = strdup(parentTable);
            Table* table = findOrCreateTable(schemaKey, parentTable, parentTable);
            if (!table) {
                free(schemaKey);
                return;
            }

            for (int i = 0; i < node->childCount; i++) {
                ASTNode* child = node->children[i];
                if (!child) continue;

                Row* row = malloc(sizeof(Row));
                if (!row) continue;

                row->id = idCounter++;
                row->parentId = parentId;
                row->tableName = strdup(table->name);
                row->keyCount = 2; // index, value
                row->keys = malloc(sizeof(char*) * 2);
                row->values = malloc(sizeof(char*) * 2);
                if (!row->tableName || !row->keys || !row->values) {
                    free(row->tableName);
                    free(row->keys);
                    free(row->values);
                    free(row);
                    continue;
                }

                row->keys[0] = strdup("index");
                char index_buffer[32];
                snprintf(index_buffer, sizeof(index_buffer), "%d", i);
                row->values[0] = strdup(index_buffer);

                row->keys[1] = strdup("value");
                if (child->strVal) {
                    row->values[1] = strdup(child->strVal);
                } else if (child->hasInt) {
                    char buffer[32];
                    snprintf(buffer, sizeof(buffer), "%d", child->intVal);
                    row->values[1] = strdup(buffer);
                } else if (child->hasBool) {
                    row->values[1] = strdup(child->boolVal ? "true" : "false");
                } else {
                    row->values[1] = strdup("");
                }

                addRow(table, row);
            }
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
        if (!table) continue;
        printf("Table: %s (Parent: %s)\n", table->name, table->parentName ? table->parentName : "none");
        for (int j = 0; j < table->rowCount; j++) {
            Row* row = table->rows[j];
            if (!row) continue;
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

void freeSymbolTables() {
    for (int i = 0; i < tableCount; i++) {
        Table* table = tables[i];
        if (!table) continue;
        for (int j = 0; j < table->rowCount; j++) {
            Row* row = table->rows[j];
            if (!row) continue;
            for (int k = 0; k < row->keyCount; k++) {
                free(row->keys[k]);
                free(row->values[k]);
            }
            free(row->keys);
            free(row->values);
            free(row->tableName);
            free(row);
        }
        free(table->rows);
        free(table->schemaKey);
        free(table->name);
        free(table->parentName);
        free(table);
    }
    tableCount = 0;
    idCounter = 1;
}