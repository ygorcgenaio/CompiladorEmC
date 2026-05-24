typedef struct ASTNode {
    char *label;
    struct ASTNode **children;
    int childCount;
} ASTNode;

ASTNode* program();
void printAST(ASTNode *node, int depth);
extern FILE *arquivo;
extern Token currentToken;
extern char* errors[];
extern int errorCount;
