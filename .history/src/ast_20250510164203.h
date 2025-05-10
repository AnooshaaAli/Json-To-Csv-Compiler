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
    struct ASTNode* parent;
} ASTNode;

typedef struct Row {
    int id;
    int parentId;
    char** values;
    int columnCount;           
    struct Row* next;          
} Row;

typedef struct Table {
    char* name;               
    char** columns;            // Column names (e.g., object keys)
    int columnCount;
    Row* rows;                 // Head of linked list of rows
    int rowCount;
    int nextId; 
    struct Table* next;        // For linked list of tables
} Table;

typedef struct TableList {
    Table* head;
} TableList;

typedef struct RowList {
    Row* head;
} RowList;

ASTNode* createNode(const char* type);
ASTNode* createStrNode(const char* type, char* val);
ASTNode* createIntNode(const char* type, int val);
ASTNode* createBoolNode(const char* type, int val);
void addChild(ASTNode* parent, ASTNode* child);
void printAST(ASTNode* node, int indent, int isLast);

#endif
