#ifndef AST_H
#define AST_H

typedef struct ASTNode {
    char* type;
    char* strVal;
    int intVal;
    int boolVal;
    struct ASTNode** children;
    int childCount;
} ASTNode;

ASTNode* createNode(const char* type);
ASTNode* createStrNode(const char* type, char* val);
ASTNode* createIntNode(const char* type, int val);
ASTNode* createBoolNode(const char* type, int val);
void printAST(ASTNode* node, int indent); 

#endif
