#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

typedef struct {
    char** keys;
    int keyCount;
    int id;
    int parentId;
    char* tableName;
    char** values;
} Row;

typedef struct Table {
    char* schemaKey; 
    char* name;    
    Row** rows;
    int rowCount;
    int rowCap;
} Table;

Table* tables[100];
int tableCount = 0;
int idCounter = 1;

char* generateSchemaKey(ASTNode* node) {
    if (strcmp(node->type, "OBJECT") != 0) return NULL;
    char* keyStr = malloc(1024); keyStr[0] = '\0';
    for (int i = 0; i < node->childCount; i += 2) {
        strcat(keyStr, node->children[i]->strVal); 
        if (i + 2 < node->childCount) strcat(keyStr, ",");
    }
    return keyStr;
}

Table* findOrCreateTable(const char* schemaKey) {
    for (int i = 0; i < tableCount; i++) {
        if (strcmp(tables[i]->schemaKey, schemaKey) == 0)
            return tables[i];
    }
    Table* t = malloc(sizeof(Table));
    t->schemaKey = strdup(schemaKey);
    t->name = malloc(32);
    sprintf(t->name, "table_%d", tableCount + 1);
    t->rows = malloc(sizeof(Row*) * 10);
    t->rowCount = 0;
    t->rowCap = 10;
    tables[tableCount++] = t;
    return t;
}

void addRow(Table* t, Row* row) {
    if (t->rowCount >= t->rowCap) {
        t->rowCap *= 2;
        t->rows = realloc(t->rows, sizeof(Row*) * t->rowCap);
    }
    t->rows[t->rowCount++] = row;
}

void walkAST(ASTNode* node, const char* parentTable, int parentId

void printSymbolTables();