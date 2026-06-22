#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Definicion de los tipos de nodos para mayor claridad
typedef enum {
    OPERAND,   // Hojas: a-z, 0-9, o 'E'
    OPERATOR   // Nodos internos: '|', '.', '*'
} NodeType;

// Estructura del nodo del arbol binario
typedef struct RegexNode {
    NodeType type;
    char value;                 // Guarda el caracter del operador o del operando
    struct RegexNode* left;     // Hijo izquierdo
    struct RegexNode* right;    // Hijo derecho (NULL si es unario o si es una hoja)
} RegexNode;

// Funcion para crear un nodo de tipo operando (hoja)
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

// Funcion para crear un nodo de tipo operador
RegexNode* createOperatorNode(char operator, RegexNode* left, RegexNode* right) {
    RegexNode* newNode = (RegexNode*)malloc(sizeof(RegexNode));
    if (!newNode) {
        printf("Error: No se pudo asignar memoria.\n");
        exit(1);
    }
    newNode->type = OPERATOR;
    newNode->value = operator;
    newNode->left = left;
    
    // Si es la clausura de Kleene (*), es un operador unario, el hijo derecho debe ser NULL
    if (operator == '*') {
        newNode->right = NULL;
    } else {
        newNode->right = right;
    }
    
    return newNode;
}

// Funcion auxiliar para imprimir el arbol en formato infijo (para verificar la estructura)
void printExpression(RegexNode* root) {
    if (root == NULL) return;

    if (root->type == OPERATOR) {
        // Si no es unario (*), abrimos parentesis para mantener el orden visual
        if (root->value != '*') printf("(");
        
        printExpression(root->left);
        printf("%c", root->value);
        printExpression(root->right);
        
        if (root->value != '*') printf(")");
    } else {
        // Es un operando
        printf("%c", root->value);
    }
}

// Funcion para liberar la memoria del arbol
void freeTree(RegexNode* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

int main() {
    printf("--- Arbol Binario para Expresiones Regulares ---\n\n");

    // Ejemplo: Representar la expresion regular: (a|b).c*
    // Construccion de abajo hacia arriba (Bottom-up):
    
    // 1. Subarbol para (a|b)
    RegexNode* nodeA = createOperandNode('a');
    RegexNode* nodeB = createOperandNode('b');
    RegexNode* unionNode = createOperatorNode('|', nodeA, nodeB);

    // 2. Subarbol para c*
    RegexNode* nodeC = createOperandNode('c');
    RegexNode* starNode = createOperatorNode('*', nodeC, NULL);

    // 3. Nodo raiz: concatenacion de ambos subarboles
    RegexNode* root = createOperatorNode('.', unionNode, starNode);

    // Mostrar la expresion resultante recorriendo el arbol
    printf("Expresion generada recorriendo el arbol: ");
    printExpression(root);
    printf("\n");

    // Ejemplo con lambda (E) -> E.a
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