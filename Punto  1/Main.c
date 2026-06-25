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

void printExpression(RegexNode* root) {
    if (root == NULL) return;

    if (root->type == OPERATOR) {
        if (root->value != '*') printf("(");
        
        printExpression(root->left);
        printf("%c", root->value);
        printExpression(root->right);
        
        if (root->value != '*') printf(")");
    } else {
        printf("%c", root->value);
    }
}

void freeTree(RegexNode* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

int main() {
    printf("--- Arbol Binario para Expresiones Regulares ---\n\n");

    RegexNode* nodeA = createOperandNode('a');
    RegexNode* nodeB = createOperandNode('b');
    RegexNode* unionNode = createOperatorNode('|', nodeA, nodeB);

    RegexNode* nodeC = createOperandNode('c');
    RegexNode* starNode = createOperatorNode('*', nodeC, NULL);

    RegexNode* root = createOperatorNode('.', unionNode, starNode);

    printf("Expresion generada recorriendo el arbol: ");
    printExpression(root);
    printf("\n");

    RegexNode* lambdaNode = createOperandNode('E');
    RegexNode* nodeA2 = createOperandNode('a');
    RegexNode* root2 = createOperatorNode('.', lambdaNode, nodeA2);

    printf("Expresion con lambda (E): ");
    printExpression(root2);
    printf("\n");

    // Liberar memoria
    freeTree(root);
    freeTree(root2);

    return 0;
}