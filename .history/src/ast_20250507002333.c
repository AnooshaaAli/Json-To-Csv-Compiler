#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode* createNode(const char* type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = strdup(type);
    node->strVal = NULL;
    node->intVal = 0;
    node->boolVal = 0;
    node->children = NULL;
    node->childCount = 0;
    return node;
}

ASTNode* createStrNode(const char* type, char* val) {
    ASTNode* node = createNode(type);
    node->strVal = strdup(val);
    return node;
}

ASTNode* createIntNode(const char* type, int val) {
    ASTNode* node = createNode(type);
    node->intVal = val;
    return node;
}

ASTNode* createBoolNode(const char* type, int val) {
    ASTNode* node = createNode(type);
    node->boolVal = val;
    return node;
}


void printAST(ASTNode* node, int indent) {
    if (!node) return;

    for (int i = 0; i < indent; i++) printf("  "); // Indent

    printf("Type: %s", node->type);
    if (node->strVal) printf(", StrVal: %s", node->strVal);
    if (node->hasInt) printf(", IntVal: %d", node->intVal);
    if (node->hasBool) printf(", BoolVal: %s", node->boolVal ? "true" : "false");
    printf("\n");

    // Recursively print children
    for (int i = 0; i < node->childCount; i++) {
        printAST(node->children[i], indent + 1);
    }
}

