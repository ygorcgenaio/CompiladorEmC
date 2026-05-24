#include "parser.h"
#include <stdio.h>
#include <string.h>

// Função que percorre a AST e retorna o tipo da expressão
const char* checkExpr(ASTNode* node) {
    if(strcmp(node->label, "If") == 0) {
        const char* condType = checkExpr(node->children[0]);
        if(strcmp(condType, "Bool") != 0) {
            printf("Erro semântico: condição de IF deve ser booleana\n");
        }
        const char* thenType = checkExpr(node->children[1]);
        const char* elseType = checkExpr(node->children[2]);
        // simplificação: se tipos diferentes, erro
        if(strcmp(thenType, elseType) != 0) {
            printf("Erro semântico: tipos do THEN e ELSE incompatíveis\n");
        }
        return thenType; // tipo resultante
    }
    else if(strcmp(node->label, "While") == 0) {
        const char* condType = checkExpr(node->children[0]);
        if(strcmp(condType, "Bool") != 0) {
            printf("Erro semântico: condição de WHILE deve ser booleana\n");
        }
        checkExpr(node->children[1]);
        return "Object"; // While não retorna valor
    }
    else if(strcmp(node->label, "Numero") == 0) {
        return "Int";
    }
    else if(strcmp(node->label, "True") == 0 || strcmp(node->label, "False") == 0) {
        return "Bool";
    }
    else {
        return "Object"; // fallback
    }
}
