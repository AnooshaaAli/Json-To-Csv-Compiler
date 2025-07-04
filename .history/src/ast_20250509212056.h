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
} Row;

typedef struct Table {
    char* name;
    char** columns;
    int columnCount;
    Row** rows;
    int rowCount;
} Table;

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
