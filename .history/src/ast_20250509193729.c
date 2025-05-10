#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

typedef struct Row {
    char** values; 
    int columnCount;
    struct Row* next;
} Row;

// Data structure for a table
typedef struct Table {
    char* name;
    char** columns;  // Column names
    int columnCount;
    Row* rows;
    int rowCount;
    struct Table* next;
} Table;

Table* tables = NULL;
int globalId = 1;

// Helper to create or find a table by key signature
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

    Table* newTable = malloc(sizeof(Table));
    newTable->name = strdup(name);
    newTable->columns = malloc(sizeof(char*) * columnCount);
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

// Helper to add a row to a table
void addRowToTable(Table* table, char** values) {
    Row* newRow = malloc(sizeof(Row));
    newRow->values = malloc(sizeof(char*) * table->columnCount);
    for (int i = 0; i < table->columnCount; i++) {
        newRow->values[i] = strdup(values[i]);
    }
    newRow->columnCount = table->columnCount;
    newRow->next = table->rows;
    table->rows = newRow;
    table->rowCount++;
}

void extractTables(ASTNode* node, const char* parentTable, const char* parentIdKey, const char* parentIdValue) {
    if (!node) return;

    if (strcmp(node->type, "object") == 0) {
        char* keys[64];
        char* values[64];
        int count = 0;
        char childKey[64];
        
        for (int i = 0; i < node->childCount; i++) {
            ASTNode* pair = node->children[i];
            if (strcmp(pair->type, "pair") == 0) {
                keys[count] = pair->strVal;
                ASTNode* valNode = pair->children[0];
                if (strcmp(valNode->type, "number") == 0 && valNode->hasInt) {
                    char buffer[32];
                    sprintf(buffer, "%d", valNode->intVal);
                    values[count] = strdup(buffer);
                } else if (strcmp(valNode->type, "string") == 0) {
                    values[count] = valNode->strVal;
                } else if (strcmp(valNode->type, "object") == 0) {
                    sprintf(childKey, "%s_id", parentTable);
                    extractTables(valNode, pair->strVal, childKey, NULL); // Recursively process nested object
                    values[count] = strdup("<object>");
                } else {
                    values[count] = strdup("<other>");
                }
                count++;
            }
        }

        if (parentTable && parentIdKey && parentIdValue) {
            keys[count] = strdup(parentIdKey);
            values[count] = strdup(parentIdValue);
            count++;
        }

        char idBuffer[16];
        sprintf(idBuffer, "%d", globalId++);
        keys[count] = strdup("id");
        values[count] = strdup(idBuffer);
        count++;

        Table* table = findOrCreateTable(parentTable ? parentTable : "root", keys, count);
        addRowToTable(table, values);
    }

    for (int i = 0; i < node->childCount; i++) {
        extractTables(node->children[i], parentTable, parentIdKey, parentIdValue);
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
