#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"
#include "semantic.h"
#include "ast.h"

// LEXER
Token getNextToken();

// token atual
Token currentToken;

ASTNode *astRoot = NULL;

// UTIL
void syntaxError(const char *msg) {

    printf("\nERRO SINTATICO\n");
    printf("%s\n", msg);
    printf("Token encontrado: %s\n", currentToken.lexeme);
    exit(1);
}

void advance() {
    currentToken = getNextToken();

// ERRO LEXICO
    if (currentToken.type == TOK_ERROR) {
        printf("\nERRO LEXICO\n");
        printf("Lexema invalido: %s\n", currentToken.lexeme);
        exit(1);
    }
}

void match(TokenType expected) {

    if (currentToken.type == expected) {
        advance();
    }
    else {
        syntaxError("token inesperado");
    }
}

// PROTOTIPOS

ASTNode* programa();
ASTNode* listaDeclaracoes();
ASTNode* declaracao();

ASTNode* declaracaoVariavel();
void tipo();

ASTNode* comando();
ASTNode* atribuicao();

ASTNode* comandoIf();
ASTNode* comandoFor();

ASTNode* bloco();

ASTNode* expressao();
ASTNode* expressaoRelacional();
ASTNode* expressaoAritmetica();

ASTNode* termo();
ASTNode* fator();

void opRelacional();
void literal();

// PROGRAMA

void parseProgram() {

    semBeginProgram();

    advance();

    if (currentToken.type == TOK_EOF) {
        syntaxError("programa vazio");
    }

    astRoot = programa();
    if (currentToken.type != TOK_EOF) {
        syntaxError("tokens apos fim do programa");
    }
    semEndProgram();
    printf("\nAnalise concluida com sucesso\n");
}

ASTNode* programa() {
    ASTNode *program =
        createNode(AST_PROGRAM, "PROGRAM");
    program->left = listaDeclaracoes();
    return program;
}

// GRAMATICA

ASTNode* listaDeclaracoes() {

    ASTNode *first = NULL;
    ASTNode *last = NULL;

// listaDeclaracoes ::= declaracao*
    while (
        currentToken.type == TOK_INTEGER ||
        currentToken.type == TOK_REAL ||
        currentToken.type == TOK_CHAR ||
        currentToken.type == TOK_LITERAL ||
        currentToken.type == TOK_BOOL ||
        currentToken.type == TOK_IF ||
        currentToken.type == TOK_FOR ||
        currentToken.type == TOK_LEFT_BRACE ||
        currentToken.type == TOK_ID
    ) {
        ASTNode *node = declaracao();

        if (first == NULL) {
            first = node;
            last = node;
        }
        else {
            last->next = node;
            last = node;
        }
    }

    return first;
}

// declaracao
ASTNode* declaracao() {
    if (
        currentToken.type == TOK_INTEGER ||
        currentToken.type == TOK_REAL ||
        currentToken.type == TOK_CHAR ||
        currentToken.type == TOK_LITERAL ||
        currentToken.type == TOK_BOOL
    ) {
        return declaracaoVariavel();
    }
    return comando();
}

// tipo
void tipo() {

    switch (currentToken.type) {

        case TOK_INTEGER: 
        match(TOK_INTEGER); 
        break;
        
        case TOK_REAL: 
        match(TOK_REAL); 
        break;
        
        case TOK_CHAR: 
        match(TOK_CHAR); 
        break;
        
        case TOK_LITERAL: 
        match(TOK_LITERAL); 
        break;
        
        case TOK_BOOL: 
        match(TOK_BOOL); 
        break;

        default:
            syntaxError("tipo invalido");
    }
}

// declaracao_variavel ::= tipo id = expressao ;
ASTNode* declaracaoVariavel() {
    TokenType tipoVar = currentToken.type;

    tipo();
    char nome[MAX_LEXEMA];
    strcpy(nome, currentToken.lexeme);

    match(TOK_ID);
    semDeclareVariable(nome, tipoVar);

    match(TOK_ASSIGN);
    
    ASTNode *expr = expressao();

    match(TOK_SEMICOLON);

    ASTNode *decl =
        createNode(AST_VAR_DECL, nome);

    decl->left = expr;

    return decl;
}

//comando
ASTNode* comando() {

    switch (currentToken.type) {

        case TOK_IF:
            comandoIf();
            break;

        case TOK_FOR:
            comandoFor();
            break;

        case TOK_LEFT_BRACE:
            bloco();
            break;

        case TOK_ID:
            atribuicao();
            break;

        default:
            syntaxError("comando invalido");
    }
}

// atribuicao ::= id = expressao ;
ASTNode* atribuicao() {

    char nome[MAX_LEXEMA];
    strcpy(nome, currentToken.lexeme);

    match(TOK_ID);

    semAssign(nome);

    match(TOK_ASSIGN);

    ASTNode *expr = expressao();

    match(TOK_SEMICOLON);

    ASTNode *assign =
        createNode(AST_ASSIGN, nome);

    assign->left = expr;

    return assign;
}

// if
ASTNode* comandoIf() {

    match(TOK_IF);
    match(TOK_LEFT_PAREN);

    ASTNode *condition = expressao();
    semCheckCondition();

    match(TOK_RIGHT_PAREN);

    ASTNode *thenBlock = bloco();

    ASTNode *elseBlock = NULL;

    if (currentToken.type == TOK_ELSE) {
        match(TOK_ELSE);
        elseBlock = bloco();
    }

    ASTNode *node =
        createNode(AST_IF, "IF");

    node->left = condition;
    node->right = thenBlock;
    node->third = elseBlock;

    return node;
}

// for
ASTNode* comandoFor() {

    match(TOK_FOR);
    match(TOK_LEFT_PAREN);

    ASTNode *init = atribuicao();

    ASTNode *condition = expressao();
    semCheckCondition();

    match(TOK_SEMICOLON);

    char nome[MAX_LEXEMA];
    strcpy(nome, currentToken.lexeme);

    match(TOK_ID);
    semAssign(nome);

    match(TOK_ASSIGN);

    ASTNode *increment = expressao();

    semCheckIncrement(nome);

    match(TOK_RIGHT_PAREN);

    ASTNode *body = bloco();

    ASTNode *node =
        createNode(AST_FOR, "FOR");

    node->left = init;
    node->right = condition;
    node->third = increment;

    increment->next = body;

    return node;
}

// bloco ::= { declaracao }
ASTNode* bloco() {

    match(TOK_LEFT_BRACE);

    semBeginBlock();

    ASTNode *block =
        createNode(AST_BLOCK, "BLOCK");

    block->left = listaDeclaracoes();

    semEndBlock();

    match(TOK_RIGHT_BRACE);

    return block;
}

// expressao
ASTNode* expressao() {
    return expressaoRelacional();
}

// relacional
ASTNode* expressaoRelacional() {

    ASTNode *left =
        expressaoAritmetica();

    if (
        currentToken.type == TOK_GT ||
        currentToken.type == TOK_GTE ||
        currentToken.type == TOK_LT ||
        currentToken.type == TOK_LTE ||
        currentToken.type == TOK_EQ ||
        currentToken.type == TOK_NE
    ) {
        char op[MAX_LEXEMA];
        strcpy(op, currentToken.lexeme);
        opRelacional();

        ASTNode *right =
            expressaoAritmetica();

        ASTNode *node =
            createNode(AST_BINARY_OP, op);

        node->left = left;
        node->right = right;

        return node;
    }

    return left;
}

// aritmetica
ASTNode* expressaoAritmetica() {

    ASTNode *left = termo();

    while (
        currentToken.type == TOK_PLUS ||
        currentToken.type == TOK_MINUS
    ) {
        char op[MAX_LEXEMA];
        strcpy(op, currentToken.lexeme);

        if (currentToken.type == TOK_PLUS) {
            match(TOK_PLUS);
        }
        else {
            match(TOK_MINUS);
        }

        ASTNode *right = termo();

        ASTNode *node =
            createNode(AST_BINARY_OP, op);

        node->left = left;
        node->right = right;

        left = node;
    }

    return left;
}

// termo
ASTNode* termo() {

    ASTNode *left = fator();

    while (
        currentToken.type == TOK_MULT ||
        currentToken.type == TOK_DIV
    ) {
        char op[MAX_LEXEMA];
        strcpy(op, currentToken.lexeme);

        if (currentToken.type == TOK_MULT) {
            match(TOK_MULT);
        }
        else {
            match(TOK_DIV);
        }

        ASTNode *right = fator();

        ASTNode *node =
            createNode(AST_BINARY_OP, op);

        node->left = left;
        node->right = right;

        left = node;
    }

    return left;
}

// fator
ASTNode* fator() {

    ASTNode *node;

    if (currentToken.type == TOK_ID) {
        semUseVariable(currentToken.lexeme);
        node =
            createNode(AST_IDENTIFIER,
                       currentToken.lexeme);

        match(TOK_ID);

        return node;
    }

    if (
        currentToken.type == TOK_INT_LITERAL ||
        currentToken.type == TOK_REAL_LITERAL ||
        currentToken.type == TOK_CHAR_LITERAL ||
        currentToken.type == TOK_STRING_LITERAL ||
        currentToken.type == TOK_TRUE ||
        currentToken.type == TOK_FALSE
    ) {
        node =
            createNode(AST_LITERAL,
                       currentToken.lexeme);
        literal();

        return node;
    }

    if (currentToken.type == TOK_LEFT_PAREN) {

        match(TOK_LEFT_PAREN);
        node = expressao();
        match(TOK_RIGHT_PAREN);

        return node;
    }

    syntaxError("fator invalido");

    return NULL;
}

// operadores relacionais
void opRelacional() {

    switch (currentToken.type) {

        case TOK_GT:
            match(TOK_GT);
            break;

        case TOK_GTE:
            match(TOK_GTE);
            break;

        case TOK_LT:
            match(TOK_LT);
            break;

        case TOK_LTE:
            match(TOK_LTE);
            break;

        case TOK_EQ:
            match(TOK_EQ);
            break;

        case TOK_NE:
            match(TOK_NE);
            break;

        default:
            syntaxError("operador relacional invalido");
    }
}

// literais
void literal() {

    switch (currentToken.type) {

        case TOK_INT_LITERAL:
            match(TOK_INT_LITERAL);
            break;

        case TOK_REAL_LITERAL:
            match(TOK_REAL_LITERAL);
            break;

        case TOK_CHAR_LITERAL:
            match(TOK_CHAR_LITERAL);
            break;

        case TOK_STRING_LITERAL:
            match(TOK_STRING_LITERAL);
            break;

        case TOK_TRUE:
            match(TOK_TRUE);
            break;

        case TOK_FALSE:
            match(TOK_FALSE);
            break;

        default:
            syntaxError("literal invalido");
    }
}
