#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

unsigned int hash(char* str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash << 5) + *str++;
    }
    return hash % TABLE_SIZE;
}

int addSymbol(char* key) {
    unsigned int index = hash(key);
    SymbolTable* newEntry = malloc(sizeof(SymbolTable));
    if (!newEntry) {
        return 0;
    }
    newEntry->key = strdup(key);
    newEntry->next = symbolTable[index];
    symbolTable[index] = newEntry;
    return 1;
}

int isSymbolPresent(char* key) {
    unsigned int index = hash(key);
    SymbolTable* entry = symbolTable[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return 1; 
        }
        entry = entry->next;
    }
    return 0; 
}

void printSymbolTable() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        SymbolTable* entry = symbolTable[i];
        while (entry) {
            printf("Key: %s\n", entry->key);
            entry = entry->next;
        }
    }
}

ASTNode* createNode(const char* type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = strdup(type);
    node->strVal = NULL;
    node->intVal = 0;
    node->boolVal = 0;
    node->hasInt = 0;
    node->hasBool = 0;
    node->childCount = 0;
    node->childCapacity = 4;
    node->children = malloc(sizeof(ASTNode*) * node->childCapacity);
    return node;
}

void addChild(ASTNode* parent, ASTNode* child) {
    if (parent->childCount >= parent->childCapacity) {
        parent->childCapacity *= 2;
        parent->children = realloc(parent->children, sizeof(ASTNode*) * parent->childCapacity);
    }
    parent->children[parent->childCount++] = child;
}

ASTNode* createStrNode(const char* type, char* val) {
    ASTNode* node = createNode(type);
    node->strVal = strdup(val);
    return node;
}

ASTNode* createIntNode(const char* type, int val) {
    ASTNode* node = createNode(type);
    node->intVal = val;
    node->hasInt = 1;
    return node;
}

ASTNode* createBoolNode(const char* type, int val) {
    ASTNode* node = createNode(type);
    node->boolVal = val;
    node->hasBool = 1;
    return node;
}

void printAST(ASTNode* node, int indent, int isLast) {
    if (!node) return;

    for (int i = 0; i < indent - 1; i++) {
        printf("│   ");
    }

    if (indent > 0) {
        printf(isLast ? "└── " : "├── ");
    }

    printf("Type: %s", node->type);
    if (node->strVal) printf(", StrVal: %s", node->strVal);
    if (node->hasInt) printf(", IntVal: %d", node->intVal);
    if (node->hasBool) printf(", BoolVal: %s", node->boolVal ? "true" : "false");
    printf("\n");

    for (int i = 0; i < node->childCount; i++) {
        int isLastChild = (i == node->childCount - 1);
        printAST(node->children[i], indent + 1, isLastChild);
    }
}
