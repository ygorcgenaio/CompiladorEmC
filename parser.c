#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lex.h"

#define MAX_ERRORS 100

FILE *arquivo;
Token currentToken;

char* errors[MAX_ERRORS];
int errorCount = 0;

// ================= AST =================
typedef struct ASTNode {
    char *label;
    struct ASTNode **children;
    int childCount;
} ASTNode;

ASTNode* createNode(const char *label) {
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    node->label = strdup(label);
    node->children = NULL;
    node->childCount = 0;
    return node;
}

void addChild(ASTNode *parent, ASTNode *child) {
    parent->children = realloc(parent->children, sizeof(ASTNode*) * (parent->childCount + 1));
    parent->children[parent->childCount++] = child;
}

void printAST(ASTNode *node, int depth) {
    for(int i = 0; i < depth; i++) printf("  ");
    printf("%s\n", node->label);
    for(int i = 0; i < node->childCount; i++) {
        printAST(node->children[i], depth + 1);
    }
}

// ================= ERRO =================
void reportError(const char* msg, int linha, int coluna) {
    if (errorCount < MAX_ERRORS) {
        char buffer[200];
        snprintf(buffer, sizeof(buffer),
                 "Erro na linha %d, coluna %d: %s",
                 linha, coluna, msg);
        errors[errorCount++] = strdup(buffer);
    }
}

// ================= CONTROLE =================
void advance() {
    currentToken = getNextToken(arquivo);
    while (currentToken.tipo == COMENTARIO) {
        currentToken = getNextToken(arquivo);
    }
}

void synchronize() {
    while (currentToken.tipo != SEMICOLON &&
           currentToken.tipo != RBRACE &&
           currentToken.tipo != EOF_TOKEN) {
        advance();
    }
    if (currentToken.tipo == SEMICOLON) {
        advance();
    }
}

void match(TokenTipo esperado) {
    if (currentToken.tipo == esperado) {
        advance();
    } else {
        reportError("Token inesperado",
                    currentToken.location.linha,
                    currentToken.location.coluna);
        synchronize();
    }
}

// ================= PROTÓTIPOS =================
ASTNode* program();
ASTNode* classNode();
ASTNode* feature();
ASTNode* formal();
ASTNode* expr();
ASTNode* args();
ASTNode* expr_rel();
ASTNode* expr_arit();
ASTNode* term();
ASTNode* factor();

// ================= PROGRAMA =================
ASTNode* program() {
    ASTNode *root = createNode("Program");
    while (currentToken.tipo == CLASS) {
        addChild(root, classNode());
    }
    return root;
}

// ================= CLASS =================
ASTNode* classNode() {
    ASTNode *node = createNode("Class");
    match(CLASS);
    match(IDENTIFICADOR);

    if (currentToken.tipo == INHERITS) {
        match(INHERITS);
        match(IDENTIFICADOR);
    }

    match(LBRACE);
    while (currentToken.tipo == IDENTIFICADOR) {
        addChild(node, feature());
    }
    match(RBRACE);
    match(SEMICOLON);
    return node;
}

// ================= FORMAL =================
ASTNode* formal() {
    ASTNode *node = createNode("Formal");
    match(IDENTIFICADOR);
    match(COLON);
    match(IDENTIFICADOR);
    return node;
}

// ================= FEATURE =================
ASTNode* feature() {
    ASTNode *node = createNode("Feature");
    match(IDENTIFICADOR);

    if (currentToken.tipo == LPAREN) {
        ASTNode *methodNode = createNode("Method");
        match(LPAREN);
        if (currentToken.tipo == IDENTIFICADOR) {
            addChild(methodNode, formal());
            while (currentToken.tipo == COMMA) {
                match(COMMA);
                addChild(methodNode, formal());
            }
        }
        match(RPAREN);
        match(COLON);
        match(IDENTIFICADOR);
        match(LBRACE);
        addChild(methodNode, expr()); // corpo do método
        match(RBRACE);
        addChild(node, methodNode);
    } else {
        ASTNode *attrNode = createNode("Attribute");
        match(COLON);
        match(IDENTIFICADOR);
        if (currentToken.tipo == ATRIBUI) {
            match(ATRIBUI);
            addChild(attrNode, expr());
        }
        addChild(node, attrNode);
    }
    match(SEMICOLON);
    return node;
}

// ================= EXPRESSÕES =================
ASTNode* expr() {
    if (currentToken.tipo == IF) {
        ASTNode *node = createNode("If");
        match(IF);
        addChild(node, expr());
        match(THEN);
        addChild(node, expr());
        match(ELSE);
        addChild(node, expr());
        match(FI);
        return node;
    }
    else if (currentToken.tipo == WHILE) {
        ASTNode *node = createNode("While");
        match(WHILE);
        addChild(node, expr());
        match(LOOP);
        addChild(node, expr());
        match(POOL);
        return node;
    }
    else if (currentToken.tipo == NUMERO) {
        ASTNode *node = createNode("Numero");
        match(NUMERO);
        return node;
    }
    else if (currentToken.tipo == IDENTIFICADOR) {
        ASTNode *node = createNode("Identificador");
        match(IDENTIFICADOR);
        return node;
    }
    else {
        return expr_rel();
    }
}

// ================= RELACIONAL =================
ASTNode* expr_rel() {
    ASTNode *node = createNode("RelExpr");
    addChild(node, expr_arit());
    if (currentToken.tipo == MENOR ||
        currentToken.tipo == MENOROUIGUAL ||
        currentToken.tipo == MAIOR ||
        currentToken.tipo == MAIOROUIGUAL ||
        currentToken.tipo == IGUAL) {
        advance();
        addChild(node, expr_arit());
    }
    return node;
}

// ================= ARITMÉTICA =================
ASTNode* expr_arit() {
    ASTNode *node = createNode("AritExpr");
    addChild(node, term());
    while (currentToken.tipo == MAIS || currentToken.tipo == MENOS) {
        advance();
        addChild(node, term());
    }
    return node;
}

ASTNode* term() {
    ASTNode *node = createNode("Term");
    addChild(node, factor());
    while (currentToken.tipo == MULTIPLICACAO || currentToken.tipo == DIVISAO) {
        advance();
        addChild(node, factor());
    }
    return node;
}

ASTNode* factor() {
    if (currentToken.tipo == NUMERO) {
        ASTNode *node = createNode("Numero");
        match(NUMERO);
        return node;
    }
    else if (currentToken.tipo == IDENTIFICADOR) {
        ASTNode *node = createNode("Identificador");
        match(IDENTIFICADOR);
        return node;
    }
    else if (currentToken.tipo == LPAREN) {
        match(LPAREN);
        ASTNode *node = expr();
        match(RPAREN);
        return node;
    }
    else if (currentToken.tipo == STRING) {
        ASTNode *node = createNode("String");
        match(STRING);
        return node;
    }
    else if (currentToken.tipo == TRUE) {
        ASTNode *node = createNode("True");
        match(TRUE);
        return node;
    }
    else if (currentToken.tipo == FALSE) {
        ASTNode *node = createNode("False");
        match(FALSE);
        return node;
    }
    else {
        reportError("Factor inválido", currentToken.location.linha, currentToken.location.coluna);
        synchronize();
        return createNode("Erro");
    }
}

// ================= ARGUMENTOS =================
ASTNode* args() {
    ASTNode *node = createNode("Args");
    if (currentToken.tipo == RPAREN) return node;
    addChild(node, expr());
    while (currentToken.tipo == COMMA) {
        match(COMMA);
        addChild(node, expr());
    }
    return node;
}

// ================= MAIN =================
int main() {
    arquivo = fopen("teste.txt", "r");
    if (!arquivo) {
        printf("Erro ao abrir arquivo\n");
        return 1;
    }

    advance();
    ASTNode *root = program();

    if (errorCount > 0) {
        for (int i = 0; i < errorCount; i++) {
            printf("%s\n", errors[i]);
            free(errors[i]);
        }
    } else {
        printf("Compilação concluída sem erros!\n");
        printAST(root, 0);
    }

    fclose(arquivo);
    return 0;
}
