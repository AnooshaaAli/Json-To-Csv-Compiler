#ifndef AST_H
#define AST_H

typedef struct Row {
    char** values; 
    int columnCount;
    struct Row* next;
} Row;

typedef struct Table {
    char* name;
    char** columns; 
    int columnCount;
    Row* rows;
    int rowCount;
    struct Table* next;
} Table;


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

unsigned int hash(char* str);
int addSymbol(char* key);
int isSymbolPresent(char* key);
void printSymbolTable();
ASTNode* createNode(const char* type);
ASTNode* createStrNode(const char* type, char* val);
ASTNode* createIntNode(const char* type, int val);
ASTNode* createBoolNode(const char* type, int val);
void addChild(ASTNode* parent, ASTNode* child);
void printAST(ASTNode* node, int indent, int isLast);

#endif
