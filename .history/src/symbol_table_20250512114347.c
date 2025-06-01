#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

Table* tables[MAX_TABLES] = {0};
int tableCount = 0;
int idCounter = 1;

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

    char** key_list = NULL;
    int key_count = 0;

    ASTNode* members = NULL;
    for (int i = 0; i < node->childCount; i++) {
        if (node->children[i] && strcmp(node->children[i]->type, "members") == 0) {
            members = node->children[i];
            break;
        }
    }

    if (!members) {
        return strdup("default");
    }

    for (int i = 0; i < members->childCount; i++) {
        ASTNode* child = members->children[i];
        if (child && strcmp(child->type, "pair") == 0 && child->strVal) {
            key_list = realloc(key_list, sizeof(char*) * (key_count + 1));
            key_list[key_count++] = strdup(child->strVal);
        }
    }

    for (int i = 0; i < key_count - 1; i++) {
        for (int j = i + 1; j < key_count; j++) {
            if (strcmp(key_list[i], key_list[j]) > 0) {
                char* temp = key_list[i];
                key_list[i] = key_list[j];
                key_list[j] = temp;
            }
        }
    }

    char* keys = NULL;
    size_t keys_len = 0;
    for (int i = 0; i < key_count; i++) {
        size_t new_len = keys_len + strlen(key_list[i]) + 2;
        char* new_keys = realloc(keys, new_len);
        if (!new_keys) {
            report_error("Memory allocation failed", "generateSchemaKey", node->type);
            for (int j = 0; j < key_count; j++) free(key_list[j]);
            free(key_list);
            free(keys);
            return strdup("default");
        }
        keys = new_keys;
        if (keys_len > 0) {
            strcat(keys, ",");
            keys_len++;
        }
        strcat(keys, key_list[i]);
        keys_len = strlen(keys);
        free(key_list[i]);
    }
    free(key_list);

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
    static int objectCount = 0; // Track number of top-level objects processed
    if (!node || !node->type) {
        report_error("NULL node or type", "walkAST", NULL);
        return;
    }

    printf("Debug: Processing node type=%s, parentTable=%s, parentId=%d\n",
           node->type, parentTable ? parentTable : "none", parentId);

    if (strcmp(node->type, "object") == 0) {
        if (parentTable == NULL) {
            objectCount++;
            printf("Debug: Processing top-level object #%d\n", objectCount);
            if (objectCount > 1) {
                fprintf(stderr, "Warning: Multiple top-level objects detected\n");
            }
        }

        ASTNode* members = NULL;
        for (int i = 0; i < node->childCount; i++) {
            if (node->children[i] && strcmp(node->children[i]->type, "members") == 0) {
                members = node->children[i];
                break;
            }
        }

        if (!members) {
            printf("Debug: No members node found for object\n");
            return;
        }

        char* schemaKey = generateSchemaKey(node);
        if (!schemaKey) {
            report_error("Failed to generate schema key", "walkAST", node->type);
            return;
        }

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

        for (int i = 0; i < members->childCount; i++) {
            ASTNode* child = members->children[i];
            if (!child || strcmp(child->type, "pair") != 0) {
                printf("Debug: Skipping non-pair child at index %d, type=%s\n",
                       i, child ? child->type : "null");
                continue;
            }

            if (!child->strVal || child->childCount != 1) {
                report_error("Invalid pair node (missing strVal or child)", "walkAST", child->type);
                continue;
            }

            ASTNode* valNode = child->children[0];
            if (!valNode) {
                report_error("NULL value node in pair", "walkAST", child->type);
                continue;
            }

            printf("Debug: Processing pair key=%s, value type=%s\n",
                   child->strVal, valNode->type);

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

            if (strcmp(valNode->type, "object") == 0 || strcmp(valNode->type, "array") == 0) {
                row->values[row->keyCount] = strdup("");
                walkAST(valNode, child->strVal, row->id);
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

        if (row->keyCount == 0) {
            printf("Debug: No key-value pairs added to row ID %d in table %s\n",
                   row->id, table->name);
            free(row->keys);
            free(row->values);
            free(row->tableName);
            free(row);
        } else {
            addRow(table, row);
        }
        free(schemaKey);
        free(tableName);
    } else if (strcmp(node->type, "array") == 0) {
        if (!parentTable) {
            report_error("NULL parentTable for array", "walkAST", node->type);
            return;
        }

        int isObjectArray = 0;
        for (int i = 0; i < node->childCount; i++) {
            if (node->children[i] && strcmp(node->children[i]->type, "object") == 0) {
                isObjectArray = 1;
                break;
            }
        }

        printf("Debug: Processing array, isObjectArray=%d, childCount=%d\n",
               isObjectArray, node->childCount);

        if (isObjectArray) {
            for (int i = 0; i < node->childCount; i++) {
                ASTNode* child = node->children[i];
                if (!child || strcmp(child->type, "object") != 0) {
                    printf("Debug: Skipping non-object child at index %d, type=%s\n",
                           i, child ? child->type : "null");
                    continue;
                }

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

                row->keys = realloc(row->keys, sizeof(char*) * (row->keyCount + 1));
                row->values = realloc(row->values, sizeof(char*) * (row->keyCount + 1));
                if (!row->keys || !row->values) {
                    free(row->keys);
                    free(row->values);
                    free(row->tableName);
                    free(row);
                    free(schemaKey);
                    continue;
                }
                row->keys[row->keyCount] = strdup("seq");
                char seq_buffer[32];
                snprintf(seq_buffer, sizeof(seq_buffer), "%d", i);
                row->values[row->keyCount] = strdup(seq_buffer);
                if (!row->keys[row->keyCount] || !row->values[row->keyCount]) {
                    free(row->keys[row->keyCount]);
                    free(row->values[row->keyCount]);
                    free(row->keys);
                    free(row->values);
                    free(row->tableName);
                    free(row);
                    free(schemaKey);
                    continue;
                }
                row->keyCount++;

                for (int j = 0; j < child->childCount; j++) {
                    ASTNode* pair = child->children[j];
                    if (!pair || strcmp(pair->type, "pair") != 0 || !pair->strVal || pair->childCount != 1) {
                        printf("Debug: Skipping invalid pair at index %d, type=%s\n",
                               j, pair ? pair->type : "null");
                        continue;
                    }

                    ASTNode* valNode = pair->children[0];
                    row->keys = realloc(row->keys, sizeof(char*) * (row->keyCount + 1));
                    row->values = realloc(row->values, sizeof(char*) * (row->keyCount + 1));
                    if (!row->keys || !row->values) {
                        free(row->keys);
                        free(row->values);
                        free(row->tableName);
                        free(row);
                        free(schemaKey);
                        continue;
                    }
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
                    if (!row->keys[row->keyCount] || !row->values[row->keyCount]) {
                        free(row->keys[row->keyCount]);
                        free(row->values[row->keyCount]);
                        free(row->keys);
                        free(row->values);
                        free(row->tableName);
                        free(row);
                        free(schemaKey);
                        continue;
                    }
                    row->keyCount++;
                }

                if (row->keyCount == 1) {
                    printf("Debug: No fields added to array object row ID %d in table %s\n",
                           row->id, table->name);
                    free(row->keys[0]);
                    free(row->values[0]);
                    free(row->keys);
                    free(row->values);
                    free(row->tableName);
                    free(row);
                } else {
                    addRow(table, row);
                }
                free(schemaKey);
            }
        } else {
            char* schemaKey = strdup(parentTable);
            const char* grandparentName = NULL;
            for (int i = 0; i < tableCount; i++) {
                if (tables[i] && tables[i]->name && strcmp(tables[i]->name, parentTable) == 0) {
                    grandparentName = tables[i]->parentName;
                    break;
                }
            }
            Table* table = findOrCreateTable(schemaKey, parentTable, grandparentName ? grandparentName : "objects");
            if (!table) {
                free(schemaKey);
                return;
            }

            for (int i = 0; i < node->childCount; i++) {
                ASTNode* child = node->children[i];
                if (!child) {
                    printf("Debug: Skipping NULL child at index %d in array\n", i);
                    continue;
                }

                printf("Debug: Processing scalar array element at index %d, type=%s\n",
                       i, child->type);

                Row* row = malloc(sizeof(Row));
                if (!row) continue;

                row->id = idCounter++;
                row->parentId = parentId;
                row->tableName = strdup(table->name);
                row->keyCount = 2;
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

                if (!row->keys[0] || !row->values[0] || !row->keys[1] || !row->values[1]) {
                    free(row->keys[0]);
                    free(row->values[0]);
                    free(row->keys[1]);
                    free(row->values[1]);
                    free(row->keys);
                    free(row->values);
                    free(row->tableName);
                    free(row);
                    continue;
                }

                addRow(table, row);
            }
            free(schemaKey);
        }
    } else {
        printf("Debug: Skipping node type=%s\n", node->type);
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
            if (row->keyCount == 0) {
                printf("    (No key-value pairs)\n");
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