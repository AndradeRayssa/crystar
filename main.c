#include <stdio.h>
#include "ast.h"

// Lexer
void initLexer(const char *filename);
void closeLexer();

// Parser
ASTNode* parseProgram();

// Semantic
void semantic(ASTNode *root);

void printAST(ASTNode *node, int level) {

    if (node == NULL)
        return;

    for (int i = 0; i < level; i++)
        printf("    ");

    printf("%d", node->type);

    if (node->value[0] != '\0')
        printf(" (%s)", node->value);
        
    printf("\n");

    printAST(node->child1, level + 1);
    printAST(node->child2, level + 1);
    printAST(node->child3, level + 1);
    printAST(node->child4, level + 1);

    printAST(node->next, level);
}

// MAIN

int main() {

    printf("      COMPILADOR CRYSTAR\n");

    // Inicializa o lexer
    initLexer("codigo.star");

    printf("[1] Iniciando analise sintatica...\n");

    ASTNode *root = parseProgram();

    printf("[OK] Analise sintatica concluida.\n\n");

    printf("             AST\n");

    printAST(root, 0);

    printf("\n");

    printf("      ANALISE SEMANTICA\n");

    semantic(root);

    printf("[OK] Analise semantica concluida.\n\n");

    closeLexer();
    
    printf(" COMPILACAO FINALIZADA SEM ERROS\n");

    return 0;
}
