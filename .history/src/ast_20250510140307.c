#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"

Table* tables = NULL;

int getNextId(const char* tableName) {
    static int globalId = 1;
    return globalId++;
}

int getRowIndex(Table* table, Row* row) {
    int index = 0;
    Row* currentRow = table->rows;
    
    // Traverse the rows to find the row
    while (currentRow != NULL) {
        if (currentRow == row) {
            return index;  // Return the index of the row
        }
        currentRow = currentRow->next;
        index++;
    }

    return -1;  // Return -1 if the row was not found
}

int getColumnIndex(Table* table, char* key) {
    for (int i = 0; i < table->columnCount; i++) {
        // Compare the column name with the provided key
        if (strcmp(table->columns[i], key) == 0) {
            return i;  // Return the index if the column name matches the key
        }
    }
    return -1;  // Return -1 if the column is not found
}

char** extractKeys(ASTNode* node) {
    if (!node || strcmp(node->type, "object") != 0 || node->childCount < 1) return NULL;

    ASTNode* members = node->children[0]; // object -> members
    if (!members || strcmp(members->type, "members") != 0) return NULL;

    char** keys = malloc(sizeof(char*) * (members->childCount + 1));
    for (int i = 0; i < members->childCount; i++) {
        ASTNode* pair = members->children[i];
        if (pair && strcmp(pair->type, "pair") == 0 && pair->strVal) {
            keys[i] = strdup(pair->strVal);
        } else {
            keys[i] = strdup("unknown");
        }
    }
    keys[members->childCount] = NULL;
    return keys;
}

char* getTableName(char** keys) {
    if (keys && keys[0]) return strdup(keys[0]); 
    return strdup("default_table");
}

Table* getOrCreateTable(const char* tableName, char** keys) {
    // Count columns
    int count = 0;
    while (keys[count] != NULL) count++;
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
    Row* r = table->rows;
    if (!r) {
        table->rows = row;
    } else {
        while (r->next) r = r->next;
        r->next = row;
    }
    table->rowCount++;
    return row;
}

void fillRowWithValues(Table* table, ASTNode* node, int rowIndex) {
    if (!node || strcmp(node->type, "object") != 0 || node->childCount < 1) return;

    ASTNode* members = node->children[0];
    if (!members || strcmp(members->type, "members") != 0) return;

    for (int i = 0; i < members->childCount; i++) {
        ASTNode* pair = members->children[i];
        if (!pair || strcmp(pair->type, "pair") != 0 || pair->childCount < 1) continue;

        char* key = pair->strVal;
        ASTNode* val = pair->children[0];
        printf("Processing value of type: %s\n", val->type);
        
        int colIndex = getColumnIndex(table, key); // assumes keys were added already
        if (colIndex == -1) continue;

        switch (val->type[0]) {
            case 'n': { // number
                char buf[32];
                snprintf(buf, sizeof(buf), "%d", val->intVal);
                table->rows[rowIndex].values[colIndex] = strdup(buf);
                break;
            }
            case 's': // string
                table->rows[rowIndex].values[colIndex] = strdup(val->strVal);
                break;
            case 'b': // boolean
                table->rows[rowIndex].values[colIndex] = strdup(val->boolVal ? "true" : "false");
                break;
            case 'o': { // object
                // Create a buffer to store the nested object's string representation
                // You can either use a recursive call to handle nested objects,
                // or create a string that represents the object in a simplified way
                char* nestedBuffer = malloc(256); // buffer size can be adjusted as needed
                snprintf(nestedBuffer, 256, "Object with %d members", val->childCount);
                
                // You can either recursively call `fillRowWithValues()` to handle the nested object,
                // or just store the simplified string representation.
                table->rows[rowIndex].values[colIndex] = nestedBuffer;
                break;
            }
            default:
                table->rows[rowIndex].values[colIndex] = strdup("unsupported");
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

    // Unwrap "members" nodes (they just contain children)
    if (strcmp(node->type, "members") == 0) {
        for (int i = 0; i < node->childCount; i++) {
            extractTables(node->children[i], parentTable, parentId);
        }
        return;
    }

    // Unwrap "pair" nodes
    if (strcmp(node->type, "pair") == 0 && node->childCount > 0) {
        extractTables(node->children[0], parentTable, parentId);  // pair.value
        return;
    }

    if (strcmp(node->type, "object") == 0) {
        char** keys = extractKeys(node);
        char* tableName = getTableName(keys); 
        int currentId = getNextId(tableName);

        Table* table = getOrCreateTable(tableName, keys);
        Row* row = createRow(table, currentId, parentTable, parentId);
        int rowIndex = getRowIndex(table, row); 
        fillRowWithValues(table, node, rowIndex);

        for (int i = 0; i < node->childCount; i++) {
            ASTNode* pair = node->children[i];
            if (!pair || pair->childCount == 0) continue;

            ASTNode* valNode = pair->children[0];
            if (strcmp(valNode->type, "object") == 0 || strcmp(valNode->type, "array") == 0 ||
                strcmp(valNode->type, "members") == 0 || strcmp(valNode->type, "pair") == 0) {
                extractTables(valNode, tableName, currentId);
            }
        }
    } else if (strcmp(node->type, "array") == 0) {
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
