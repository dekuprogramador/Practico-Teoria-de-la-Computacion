#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef enum {
    OPERAND,
    OPERATOR
} NodeType;

typedef struct RegexNode {
    NodeType type;
    char value;
    struct RegexNode* left;
    struct RegexNode* right;
} RegexNode;

RegexNode* createOperandNode(char value) {
    RegexNode* newNode = (RegexNode*)malloc(sizeof(RegexNode));
    if (!newNode) {
        printf("Error: No se pudo asignar memoria.\n");
        exit(1);
    }
    newNode->type = OPERAND;
    newNode->value = value;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}

RegexNode* createOperatorNode(char operator, RegexNode* left, RegexNode* right) {
    RegexNode* newNode = (RegexNode*)malloc(sizeof(RegexNode));
    if (!newNode) {
        printf("Error: No se pudo asignar memoria.\n");
        exit(1);
    }
    newNode->type = OPERATOR;
    newNode->value = operator;
    newNode->left = left;
    
    if (operator == '*') {
        newNode->right = NULL;
    } else {
        newNode->right = right;
    }
    
    return newNode;
}

void writeDOT(RegexNode* root) {
    if (root == NULL) return;

    unsigned long long id = (unsigned long long)root;

    printf("    N%llu[label=\"%c\"];\n", id, root->value);

    if (root->left != NULL) {
        unsigned long long leftId = (unsigned long long)root->left;
        printf("    N%llu -> N%llu;\n", id, leftId);
        writeDOT(root->left);
    }

    if (root->right != NULL) {
        unsigned long long rightId = (unsigned long long)root->right;
        printf("    N%llu -> N%llu;\n", id, rightId);
        writeDOT(root->right);
    }
}

void exportToDOT(RegexNode* root) {
    if (root == NULL) return;
    
    printf("digraph G {\n");
    writeDOT(root);
    printf("}\n");
}

void freeTree(RegexNode* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

int main() {

    RegexNode* a1 = createOperandNode('a');
    RegexNode* b1 = createOperandNode('b');
    RegexNode* dot1 = createOperatorNode('.', a1, b1);

    RegexNode* a2 = createOperandNode('a');
    RegexNode* b2 = createOperandNode('b');
    RegexNode* star = createOperatorNode('*', b2, NULL);
    RegexNode* dot2 = createOperatorNode('.', a2, star);

    RegexNode* root = createOperatorNode('|', dot1, dot2);

    exportToDOT(root);

    freeTree(root);

    return 0;
}