#ifndef SYMBOL_TABLE
#define AST_H

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

void walkAST(ASTNode* node, const char* parentTable, int parentId) {
    if (!node) return;

    if (strcmp(node->type, "OBJECT") == 0) {
        char* schemaKey = generateSchemaKey(node);
        Table* table = findOrCreateTable(schemaKey);

        Row* row = malloc(sizeof(Row));
        row->keyCount = node->childCount / 2;
        row->keys = malloc(sizeof(char*) * row->keyCount);
        row->values = malloc(sizeof(char*) * row->keyCount);
        row->id = idCounter++;
        row->parentId = parentId;
        row->tableName = table->name;

        for (int i = 0, j = 0; i < node->childCount; i += 2, j++) {
            ASTNode* keyNode = node->children[i];
            ASTNode* valNode = node->children[i + 1];
            row->keys[j] = keyNode->strVal;

            if (strcmp(valNode->type, "STRING") == 0)
                row->values[j] = valNode->strVal;
            else if (strcmp(valNode->type, "NUMBER") == 0) {
                row->values[j] = malloc(32);
                sprintf(row->values[j], "%d", valNode->intVal);
            } else {
                row->values[j] = strdup(""); // placeholder for nested
                walkAST(valNode, table->name, row->id);
            }
        }
        addRow(table, row);

    } else if (strcmp(node->type, "ARRAY") == 0) {
        for (int i = 0; i < node->childCount; i++)
            walkAST(node->children[i], parentTable, parentId);
    }
}

void printSymbolTables() {
    for (int i = 0; i < tableCount; i++) {
        Table* t = tables[i];
        printf("Table: %s\n", t->name);
        printf("id");
        if (t->rows[0]->parentId != 0)
            printf(", %s_id", t->rows[0]->tableName); // parent name
        for (int k = 0; k < t->rows[0]->keyCount; k++)
            printf(", %s", t->rows[0]->keys[k]);
        printf("\n");

        for (int r = 0; r < t->rowCount; r++) {
            printf("%d", t->rows[r]->id);
            if (t->rows[r]->parentId != 0)
                printf(", %d", t->rows[r]->parentId);
            for (int k = 0; k < t->rows[r]->keyCount; k++)
                printf(", %s", t->rows[r]->values[k]);
            printf("\n");
        }
        printf("\n");
    }
}
