#include <stdio.h>
#include <stdlib.h>

typedef struct NodoArbol { 
    char valor; 
    struct NodoArbol *izq; 
    struct NodoArbol *der; 
} NodoArbol;

NodoArbol* crearNodoArbol(char valor, NodoArbol* izq, NodoArbol* der) {
    NodoArbol* n = (NodoArbol*)malloc(sizeof(NodoArbol));
    n->valor = valor; 
    n->izq = izq; 
    n->der = der; 
    return n;
}

typedef struct Estado Estado;
typedef struct Transicion { 
    char simbolo; 
    Estado *destino; 
    struct Transicion *siguiente; 
} Transicion;

struct Estado { 
    int id; 
    int esFinal; 
    Transicion *transiciones; 
};

typedef struct { 
    Estado *inicio; 
    Estado *fin; 
} AFN;

int contador_estados = 0;

Estado* crear_estado(int esFinal) {
    Estado *e = (Estado*)malloc(sizeof(Estado));
    e->id = contador_estados++; 
    e->esFinal = esFinal; 
    e->transiciones = NULL; 
    return e;
}

void conectar(Estado *origen, char simbolo, Estado *destino) {
    Transicion *t = (Transicion*)malloc(sizeof(Transicion));
    t->simbolo = simbolo; 
    t->destino = destino; 
    t->siguiente = origen->transiciones; 
    origen->transiciones = t;
}

AFN basico(char c) {
    Estado *i = crear_estado(0);
    Estado *f = crear_estado(0);
    conectar(i, c, f);
    return (AFN){i, f};
}

AFN union_afn(AFN A, AFN B) {
    Estado *i = crear_estado(0);
    Estado *f = crear_estado(0);
    conectar(i, 'E', A.inicio);
    conectar(i, 'E', B.inicio);
    conectar(A.fin, 'E', f);
    conectar(B.fin, 'E', f);
    return (AFN){i, f};
}

AFN concatenacion_afn(AFN A, AFN B) {
    conectar(A.fin, 'E', B.inicio);
    return (AFN){A.inicio, B.fin};
}

AFN clausura_afn(AFN A) {
    Estado *i = crear_estado(0);
    Estado *f = crear_estado(0);
    conectar(i, 'E', A.inicio);
    conectar(i, 'E', f);
    conectar(A.fin, 'E', A.inicio);
    conectar(A.fin, 'E', f);
    return (AFN){i, f};
}

AFN arbol_a_afn(NodoArbol* nodo) {
    if (nodo == NULL) return basico('E');
    
    if (nodo->izq == NULL && nodo->der == NULL) {
        return basico(nodo->valor);
    }
    
    if (nodo->valor == '*') {
        AFN izqAFN = arbol_a_afn(nodo->izq);
        return clausura_afn(izqAFN);
    }
    
    if (nodo->valor == '.') {
        AFN izqAFN = arbol_a_afn(nodo->izq);
        AFN derAFN = arbol_a_afn(nodo->der);
        return concatenacion_afn(izqAFN, derAFN);
    }
    
    if (nodo->valor == '|') {
        AFN izqAFN = arbol_a_afn(nodo->izq);
        AFN derAFN = arbol_a_afn(nodo->der);
        return union_afn(izqAFN, derAFN);
    }
    
    return basico('E');
}

void mostrar_transiciones(Estado *e, int *visitado) {
    if (e == NULL || visitado[e->id]) return;
    visitado[e->id] = 1;
    
    for (Transicion *t = e->transiciones; t; t = t->siguiente) {
        printf("  [Estado q%d] --(%c)--> [Estado q%d]\n", e->id, t->simbolo, t->destino->id);
        mostrar_transiciones(t->destino, visitado);
    }
}

void liberar_arbol(NodoArbol* nodo) {
    if (nodo == NULL) return;
    liberar_arbol(nodo->izq);
    liberar_arbol(nodo->der);
    free(nodo);
}

void liberar_afn(Estado *e, int *visitado) {
    if (e == NULL || visitado[e->id]) return;
    visitado[e->id] = 1;
    Transicion *t = e->transiciones;
    while (t != NULL) {
        Transicion *sig = t->siguiente;
        liberar_afn(t->destino, visitado);
        free(t);
        t = sig;
    }
    free(e);
}

int main() {
    printf("=== ARBOL DE EXPRESION -> AUTOMATA AFN (THOMPSON) ===\n\n");
    
    printf("[1] Construyendo el Arbol Sintactico para: (a . b)\n");
    NodoArbol* hojaA = crearNodoArbol('a', NULL, NULL);
    NodoArbol* hojaB = crearNodoArbol('b', NULL, NULL);
    NodoArbol* raizArbol = crearNodoArbol('.', hojaA, hojaB);
    
    printf("[2] Aplicando Algoritmo de Thompson recursivamente...\n");
    contador_estados = 0; 
    AFN resultadoAFN = arbol_a_afn(raizArbol);
    
    resultadoAFN.fin->esFinal = 1;
    
    printf("\n[3] TRANSICIONES DEL AUTOMATA RESULTANTE ('E' = Lambda):\n");
    printf("----------------------------------------------------\n");
    int *visitados = (int*)calloc(contador_estados, sizeof(int));
    mostrar_transiciones(resultadoAFN.inicio, visitados);
    printf("----------------------------------------------------\n");
    printf("Estado Inicial: q%d\n", resultadoAFN.inicio->id);
    printf("Estado Final de Aceptacion: q%d\n\n", resultadoAFN.fin->id);
    
    free(visitados);
    liberar_arbol(raizArbol);
    
    int *visitados_liberacion = (int*)calloc(contador_estados, sizeof(int));
    liberar_afn(resultadoAFN.inicio, visitados_liberacion);
    free(visitados_liberacion);
    
    printf("[Sistema] Memoria liberada correctamente.\n");
    return 0;
}