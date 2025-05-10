#ifndef AST_H
#define AST_H

typedef struct ASTNode {
    char* type;
    char* strVal;
    int intVal;
    int boolVal;
    int hasInt;
    int hasBool;

    struct ASTNode** children;
    int childCount;
    int childCapacity;
} ASTNode;

typedef struct Row {
    int id;
    int parentId;
    char** values;
    int columnCount;           // Track how many columns are in the row
    struct Row* next;          // Linked list for rows
} Row;

typedef struct Table {
    char* name;                // Table name (e.g., object name)
    char** columns;            // Column names (e.g., object keys)
    int columnCount;
    Row* rows;                 // Head of linked list of rows
    int rowCount;
    struct Table* next;        // For linked list of tables
} Table;

int getNextId(const char* tableName);
int getRowIndex(Table* table, Row* row);
getColumnIndex(Table* table, char* key);
char** extractKeys(ASTNode* node);
char* getTableName(char** keys);
Table* getOrCreateTable(const char* tableName, char** keys);
void fillRowWithValues(Table* table, ASTNode* node, int rowIndex);
int getColumnIndex(Table* table, const char* key);
Table* findOrCreateTable(const char* name, char** columns, int columnCount);
void addRowToTable(Table* table, char** values);
void extractTables(ASTNode* node, const char* parentTable, int parentId);
void printTables();
ASTNode* createNode(const char* type);
ASTNode* createStrNode(const char* type, char* val);
ASTNode* createIntNode(const char* type, int val);
ASTNode* createBoolNode(const char* type, int val);
void addChild(ASTNode* parent, ASTNode* child);
void printAST(ASTNode* node, int indent, int isLast);

#endif
