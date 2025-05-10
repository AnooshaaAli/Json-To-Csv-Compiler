#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"

Table* tables = NULL;

int getNextId(const char* tableName) {
    static int globalId = 1;
    return globalId++;
}

char** extractKeys(ASTNode* node) {
    if (!node || strcmp(node->type, "object") != 0) return NULL;

    char** keys = malloc(sizeof(char*) * node->childCount);
    for (int i = 0; i < node->childCount; i++) {
        ASTNode* pair = node->children[i];
        if (pair && pair->type && pair->strVal) {
            keys[i] = strdup(pair->strVal);
        }
    }
    return keys;
}

char* getTableName(char** keys) {
    return strdup("default_table");  // Can customize this logic
}

Table* getOrCreateTable(const char* tableName, char** keys) {
    // Count columns
    int count = 0;
    while (keys[count] != NULL) count++;
    keys[node->childCount] = NULL;
    char** keys = malloc(sizeof(char*) * (node->childCount + 1));
    return findOrCreateTable(tableName, keys, count);
}

Row* createRow(Table* table, int id, const char* parentTable, int parentId) {
    Row* row = malloc(sizeof(Row));
    row->id = id;
    row->parentId = parentId;
    row->columnCount = table->columnCount;
    row->values = malloc(sizeof(char*) * table->columnCount);
    for (int i = 0; i < table->columnCount; i++) {
        row->values[i] = strdup("");  // Initialize with empty strings
    }
    row->next = table->rows;
    table->rows = row;
    table->rowCount++;
    return row;
}

void fillRowWithValues(Row* row, Table* table, ASTNode* objectNode) {
    if (!row || !objectNode || strcmp(objectNode->type, "object") != 0) return;

    for (int i = 0; i < objectNode->childCount; i++) {
        ASTNode* pair = objectNode->children[i];
        char* key = pair->strVal;
        ASTNode* val = pair->children[0];

        for (int j = 0; j < row->columnCount; j++) {
            if (strcmp(key, table->columns[j]) == 0) {
                if (val->strVal) {
                    row->values[j] = strdup(val->strVal);
                } else if (val->hasInt) {
                    char buffer[20];
                    sprintf(buffer, "%d", val->intVal);
                    row->values[j] = strdup(buffer);
                } else if (val->hasBool) {
                    row->values[j] = strdup(val->boolVal ? "true" : "false");
                } else {
                    row->values[j] = strdup("null");
                }
                break;
            }
        }
    }
}

Table* findOrCreateTable(const char* name, char** columns, int columnCount) {
    Table* current = tables;

    while (current) {
        if (strcmp(current->name, name) == 0 && current->columnCount == columnCount) {
            int match = 1;
            for (int i = 0; i < columnCount; i++) {
                if (strcmp(current->columns[i], columns[i]) != 0) {
                    match = 0;
                    break;
                }
            }
            if (match) return current;  
        }
        current = current->next;
    }

    Table* newTable = (Table*)malloc(sizeof(Table));
    if (!newTable) return NULL;

    newTable->name = strdup(name);
    newTable->columns = (char**)malloc(sizeof(char*) * columnCount);
    for (int i = 0; i < columnCount; i++) {
        newTable->columns[i] = strdup(columns[i]);
    }

    newTable->columnCount = columnCount;
    newTable->rows = NULL;
    newTable->rowCount = 0;
    newTable->next = tables;
    tables = newTable;

    return newTable;
}

void addRowToTable(Table* table, char** values) {
    if (!table) return;

    Row* newRow = (Row*)malloc(sizeof(Row));
    if (!newRow) return;

    newRow->values = (char**)malloc(sizeof(char*) * table->columnCount);
    for (int i = 0; i < table->columnCount; i++) {
        newRow->values[i] = strdup(values[i]);  
    }

    newRow->columnCount = table->columnCount;
    newRow->next = NULL;

    newRow->next = table->rows;
    table->rows = newRow;
    table->rowCount++;
}

void extractTables(ASTNode* node, const char* parentTable, int parentId) {
    if (!node) return;

    if (strcmp(node->type, "object") == 0) {
        char** keys = extractKeys(node);
        char* tableName = getTableName(keys); 
        int currentId = getNextId(tableName);

        Table* table = getOrCreateTable(tableName, keys);
        Row* row = createRow(table, currentId, parentTable, parentId);
        fillRowWithValues(row, table, node);


        for (int i = 0; i < node->childCount; i++) {
            ASTNode* pair = node->children[i];
            ASTNode* keyNode = pair; 
            ASTNode* valNode = pair->children[0];
            if (strcmp(valNode->type, "object") == 0 || strcmp(valNode->type, "array") == 0) {
                extractTables(valNode, tableName, currentId);
            }
        }
    }

    else if (strcmp(node->type, "array") == 0) {
        for (int i = 0; i < node->childCount; i++) {
            extractTables(node->children[i], parentTable, parentId);
        }
    }
}

void printTables() {
    Table* t = tables;
    while (t) {
        printf("\nTable: %s\n", t->name);
        for (int i = 0; i < t->columnCount; i++) {
            printf("%s,", t->columns[i]);
        }
        printf("\n");
        Row* r = t->rows;
        while (r) {
            for (int i = 0; i < r->columnCount; i++) {
                printf("%s,", r->values[i]);
            }
            printf("\n");
            r = r->next;
        }
        t = t->next;
    }
}

ASTNode* createNode(const char* type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = strdup(type);
    node->strVal = NULL;
    node->intVal = 0;
    node->boolVal = 0;
    node->hasInt = 0;
    node->hasBool = 0;
    node->childCount = 0;
    node->childCapacity = 4;
    node->children = malloc(sizeof(ASTNode*) * node->childCapacity);
    return node;
}

void addChild(ASTNode* parent, ASTNode* child) {
    if (parent->childCount >= parent->childCapacity) {
        parent->childCapacity *= 2;
        parent->children = realloc(parent->children, sizeof(ASTNode*) * parent->childCapacity);
    }
    parent->children[parent->childCount++] = child;
}

ASTNode* createStrNode(const char* type, char* val) {
    ASTNode* node = createNode(type);
    node->strVal = strdup(val);
    return node;
}

ASTNode* createIntNode(const char* type, int val) {
    ASTNode* node = createNode(type);
    node->intVal = val;
    node->hasInt = 1;
    return node;
}

ASTNode* createBoolNode(const char* type, int val) {
    ASTNode* node = createNode(type);
    node->boolVal = val;
    node->hasBool = 1;
    return node;
}

void printAST(ASTNode* node, int indent, int isLast) {
    if (!node) return;

    for (int i = 0; i < indent - 1; i++) {
        printf("│   ");
    }

    if (indent > 0) {
        printf(isLast ? "└── " : "├── ");
    }

    printf("Type: %s", node->type);
    if (node->strVal) printf(", StrVal: %s", node->strVal);
    if (node->hasInt) printf(", IntVal: %d", node->intVal);
    if (node->hasBool) printf(", BoolVal: %s", node->boolVal ? "true" : "false");
    printf("\n");

    for (int i = 0; i < node->childCount; i++) {
        int isLastChild = (i == node->childCount - 1);
        printAST(node->children[i], indent + 1, isLastChild);
    }
}
