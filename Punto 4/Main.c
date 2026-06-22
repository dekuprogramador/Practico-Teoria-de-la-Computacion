#include <stdio.h>
#include <stdlib.h>

/* --- ESTRUCTURAS �RBOL --- */
typedef struct NodoArbol { char valor; struct NodoArbol *izq; struct NodoArbol *der; } NodoArbol;
NodoArbol* crearNodoArbol(char valor, NodoArbol* izq, NodoArbol* der) {
    NodoArbol* n = (NodoArbol*)malloc(sizeof(NodoArbol));
    n->valor = valor; n->izq = izq; n->der = der; return n;
}

/* --- ESTRUCTURAS AFN --- */
typedef struct Estado Estado;
typedef struct Transicion { char simbolo; Estado *destino; struct Transicion *siguiente; } Transicion;
struct Estado { int id; int esFinal; Transicion *transiciones; };
typedef struct { Estado *inicio; Estado *fin; } AFN;

int contador_estados = 0;
Estado* crear_estado(int esFinal) {
    Estado *e = (Estado*)malloc(sizeof(Estado));
    e->id = contador_estados++; e->esFinal = esFinal; e->transiciones = NULL; return e;
}
void conectar(Estado *origen, char simbolo, Estado *destino) {
    Transicion *t = (Transicion*)malloc(sizeof(Transicion));
    t->simbolo = simbolo; t->destino = destino; t->siguiente = origen->transiciones; origen->transiciones = t;
}

/* --- OPERACIONES AFN --- */
AFN basico(char c) {
    Estado *i = crear_estado(0); Estado *f = crear_estado(0); conectar(i, c, f); return (AFN){i, f};
}
AFN clausura_afn(AFN A) {
    Estado *i = crear_estado(0); Estado *f = crear_estado(0);
    conectar(i, 'E', A.inicio); conectar(i, 'E', f);
    conectar(A.fin, 'E', A.inicio); conectar(A.fin, 'E', f); return (AFN){i, f};
}

/* --- EJERCICIO 4 --- */
AFN arbol_a_afn(NodoArbol* nodo) {
    if (nodo == NULL) return basico('E');
    if (nodo->izq == NULL && nodo->der == NULL) return basico(nodo->valor);
    if (nodo->valor == '*') return clausura_afn(arbol_a_afn(nodo->izq));
    return basico('E');
}

/* --- VISUALIZADOR --- */
void mostrar_transiciones(Estado *e, int *visitado) {
    if (visitado[e->id]) return;
    visitado[e->id] = 1;
    for (Transicion *t = e->transiciones; t; t = t->siguiente) {
        printf("  [Estado q%d] --(%c)--> [Estado q%d]\n", e->id, t->simbolo, t->destino->id);
        mostrar_transiciones(t->destino, visitado);
    }
}

int main() {
    printf("=== TEST EJERCICIO 4: Arbol -> AFN ===\n");
    printf("Armando arbol para la expresion: a*\n");

    NodoArbol* hojaA = crearNodoArbol('a', NULL, NULL);
    NodoArbol* raiz = crearNodoArbol('*', hojaA, NULL);

    AFN automata = arbol_a_afn(raiz);
    automata.fin->esFinal = 1;

    printf("\nExito: AFN generado en memoria.\n");
    printf("Estado Inicial: q%d | Estado Final: q%d\n", automata.inicio->id, automata.fin->id);

    printf("\nTabla de transiciones real en memoria:\n");
    int visitados[1000] = {0};
    mostrar_transiciones(automata.inicio, visitados);

    return 0;
}