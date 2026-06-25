#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    newNode->right = right;
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

void writeDOT(FILE* file, RegexNode* root) {
    if (root == NULL || file == NULL) return;

    unsigned long long id = (unsigned long long)root;

    if (root->type == OPERATOR) {
        fprintf(file, "    N%llu [label=\"%c\", shape=\"ellipse\"];\n", id, root->value);
    } else {
        fprintf(file, "    N%llu [label=\"%c\", shape=\"ellipse\"];\n", id, root->value);
    }

    if (root->left != NULL) {
        unsigned long long leftId = (unsigned long long)root->left;
        fprintf(file, "    N%llu -> N%llu;\n", id, leftId);
        writeDOT(file, root->left);
    }

    if (root->right != NULL) {
        unsigned long long rightId = (unsigned long long)root->right;
        fprintf(file, "    N%llu -> N%llu;\n", id, rightId);
        writeDOT(file, root->right);
    }
}

void exportToDOT(RegexNode* root, const char* filename) {
    if (root == NULL) {
        printf("El arbol esta vacio. No se puede exportar.\n");
        return;
    }
    
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error: No se pudo crear el archivo '%s'.\n", filename);
        return;
    }
    
    fprintf(file, "digraph G {\n");
    fprintf(file, "    label=\"Arbol de Expresion Regular (a.b|a.b*)\";\n");
    fprintf(file, "    labelloc=\"t\";\n");
    fprintf(file, "    node [fontname=\"Arial\", fontsize=12];\n");
    
    writeDOT(file, root);
    
    fprintf(file, "}\n");
    fclose(file);
    
    printf("[Sistema] Archivo '.dot' generado exitosamente: '%s'\n", filename);
}

void freeTree(RegexNode* root) {
    if (root == NULL) return;
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

int main() {
    printf("--- GENERADOR DE ARBOL DE EXPRESION COMPLETO ---\n\n");
    
    RegexNode* nodeA = createOperandNode('a');
    RegexNode* nodeB = createOperandNode('b');
    RegexNode* unionNode = createOperatorNode('|', nodeA, nodeB);

    RegexNode* nodeC = createOperandNode('c');
    RegexNode* starNode = createOperatorNode('*', nodeC, NULL);

    RegexNode* root = createOperatorNode('.', unionNode, starNode);

    printf("Expresion generada recorriendo el arbol: ");
    printExpression(root);
    printf("\n");

    printf("1. Expresion regular resultante:\n   ");
    printExpression(root);
    printf("\n\n");

    printf("2. Generando estructura jerarquica DOT...\n");
    const char* nombreArchivo = "arbol_expresion.dot";
    exportToDOT(root, nombreArchivo);
    printf("\n");

    printf("3. Liberando memoria asignada...\n");
    freeTree(root);
    printf("[Sistema] Memoria dinamica liberada correctamente.\n\n");

    printf("Programa finalizado con exito.\n");
    return 0;
}