#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "ast.h"

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
extern int tableCount;
extern int idCounter;

char* generateSchemaKey(ASTNode* node);
Table* findOrCreateTable(const char* schemaKey);
void addRow(Table* t, Row* row);
void walkAST(ASTNode* node, const char* parentTable, int parentId);
void printSymbolTables();

#endif