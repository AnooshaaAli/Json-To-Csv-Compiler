#include <string.h>
#include <stdlib.h>
#include "ast.h"

extern Table* tables;

Table* findOrCreateTable(const char* name, char** columns, int columnCount) {
    Table* current = tables;

    // Check if table with same name and same columns already exists
    while (current) {
        if (strcmp(current->name, name) == 0 && current->columnCount == columnCount) {
            int match = 1;
            for (int i = 0; i < columnCount; i++) {
                if (strcmp(current->columns[i], columns[i]) != 0) {
                    match = 0;
                    break;
                }
            }
            if (match) return current;  // Found existing match
        }
        current = current->next;
    }

    // Create a new table
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
        newRow->values[i] = strdup(values[i]);  // Deep copy
    }

    newRow->columnCount = table->columnCount;
    newRow->next = NULL;

    // Add to front of row list
    newRow->next = table->rows;
    table->rows = newRow;
    table->rowCount++;
}


void extractTables(ASTNode* node, const char* parentTable, int parentId) {
    if (!node) return;

    if (strcmp(node->type, "object") == 0) {
        // Extract keys and form schema
        char** keys = extractKeys(node);
        char* tableName = getTableName(keys); // Use hash or canonical name
        int currentId = getNextId(tableName);

        // Create row and populate values
        Table* table = getOrCreateTable(tableName, keys);
        Row* row = createRow(table, currentId, parentTable, parentId);
        fillRowWithValues(row, node);

        // Recurse on children to find nested objects/arrays
        for (int i = 0; i < node->childCount; i++) {
            ASTNode* pair = node->children[i];
            ASTNode* keyNode = pair; // Type: pair
            ASTNode* valNode = pair->children[0];
            if (strcmp(valNode->type, "object") == 0 || strcmp(valNode->type, "array") == 0) {
                extractTables(valNode, tableName, currentId);
            }
        }
    }

    // Recurse if array
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
