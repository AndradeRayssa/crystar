#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

typedef enum {

    AST_PROGRAM,

    AST_VAR_DECL,
    AST_ASSIGN,

    AST_IF,
    AST_FOR,
    AST_BLOCK,

    AST_BINARY_OP,

    AST_IDENTIFIER,
    AST_LITERAL

} ASTNodeType;

typedef struct ASTNode {

    ASTNodeType type;

    char value[MAX_LEXEMA];

    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *third;

} ASTNode;

ASTNode* createNode(ASTNodeType type, const char *value) {

    ASTNode *node = malloc(sizeof(ASTNode));

    if (node == NULL) {
        printf("Erro ao alocar memoria para AST\n");
        exit(1);
    }

    node->type = type;

    strcpy(node->value, value);

    node->left = NULL;
    node->right = NULL;
    node->third = NULL;

    return node;
}

void printAST(ASTNode *node, int level) {

    if (node == NULL) {
        return;
    }

    for (int i = 0; i < level; i++) {
        printf("   ");
    }

    switch (node->type) {

        case AST_PROGRAM:
            printf("PROGRAM");
            break;

        case AST_VAR_DECL:
            printf("VAR_DECL(%s)", node->value);
            break;

        case AST_ASSIGN:
            printf("ASSIGN(%s)", node->value);
            break;

        case AST_IF:
            printf("IF");
            break;

        case AST_FOR:
            printf("FOR");
            break;

        case AST_BLOCK:
            printf("BLOCK");
            break;

        case AST_BINARY_OP:
            printf("OP(%s)", node->value);
            break;

        case AST_IDENTIFIER:
            printf("ID(%s)", node->value);
            break;

        case AST_LITERAL:
            printf("LITERAL(%s)", node->value);
            break;

        default:
            printf("UNKNOWN");
    }

    printf("\n");

    printAST(node->left, level + 1);
    printAST(node->right, level + 1);
    printAST(node->third, level + 1);
}

void freeAST(ASTNode *node) {

    if (node == NULL) {
        return;
    }

    freeAST(node->left);
    freeAST(node->right);
    freeAST(node->third);

    free(node);
}

#endif