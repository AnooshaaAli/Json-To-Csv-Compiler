#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "ast.h"

typedef struct {
    char** keys;        // Array of key names
    char** values;      // Array of corresponding values
    int keyCount;       // Number of key-value pairs
    int id;             // Primary key
    int parentId;       // Foreign key to parent
    char* tableName;    // Name of the table this row belongs to
} Row;

typedef struct Table {
    char* schemaKey;    // Unique key for objects with same structure
    char* name;         // Table name (e.g., "orders", "items")
    char* parentName;   // Name of parent table for foreign key (e.g., "orders" for items.order_id)
    Row** rows;         // Array of rows
    int rowCount;       // Number of rows
    int rowCap;         // Capacity of rows array
} Table;

#define MAX_TABLES 100
extern Table* tables[MAX_TABLES];
extern int tableCount;
extern int idCounter;

void report_error(const char* message, const char* context, const char* node_type);
char* generateSchemaKey(ASTNode* node);
Table* findOrCreateTable(const char* schemaKey, const char* tableName, const char* parentName);
void addRow(Table* t, Row* row);
void walkAST(ASTNode* node, const char* parentTable, int parentId);
void printSymbolTables();
void freeSymbolTables();

#endif