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
    
    if (operator == '*') {
        newNode->right = NULL;
    } else {
        newNode->right = right;
    }
    
    return newNode;
}

// Funcion recursiva auxiliar que escribe las definiciones y conexiones en formato DOT
void writeDOT(RegexNode* root) {
    if (root == NULL) return;

    // Usamos el puntero del nodo convertido a entero de largo completo (unsigned long long)
    // para generar un identificador totalmente unico en la ejecucion (ej: N140737)
    unsigned long long id = (unsigned long long)root;

    // Imprimir la definicion del nodo actual y su etiqueta (label)
    printf("    N%llu[label=\"%c\"];\n", id, root->value);

    // Si tiene hijo izquierdo, imprimir la relacion y continuar el recorrido
    if (root->left != NULL) {
        unsigned long long leftId = (unsigned long long)root->left;
        printf("    N%llu -> N%llu;\n", id, leftId);
        writeDOT(root->left);
    }

    // Si tiene hijo derecho, imprimir la relacion y continuar el recorrido
    if (root->right != NULL) {
        unsigned long long rightId = (unsigned long long)root->right;
        printf("    N%llu -> N%llu;\n", id, rightId);
        writeDOT(root->right);
    }
}

// Funcion principal solicitada para realizar la salida en formato DOT
void exportToDOT(RegexNode* root) {
    if (root == NULL) return;
    
    printf("digraph G {\n");
    writeDOT(root);
    printf("}\n");
}

// Funcion para liberar la memoria del arbol
void freeTree(RegexNode* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

int main() {
    // Recreamos exactamente el arbol de la imagen de ejemplo para la expresion: a.b|a.b*
    
    // Subarbol izquierdo: a.b
    RegexNode* a1 = createOperandNode('a');
    RegexNode* b1 = createOperandNode('b');
    RegexNode* dot1 = createOperatorNode('.', a1, b1);

    // Subarbol derecho: a.b*
    RegexNode* a2 = createOperandNode('a');
    RegexNode* b2 = createOperandNode('b');
    RegexNode* star = createOperatorNode('*', b2, NULL);
    RegexNode* dot2 = createOperatorNode('.', a2, star);

    // Raiz de la expresion: union (|) de ambos lados
    RegexNode* root = createOperatorNode('|', dot1, dot2);

    // Ejecucion y salida del formato DOT requerido
    exportToDOT(root);

    // Liberar la memoria utilizada
    freeTree(root);

    return 0;
}