#include "parser.h"
#include "semantic.h"

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
        checkExpr(root); // chamada da análise semântica
    }

    fclose(arquivo);
    return 0;
}
