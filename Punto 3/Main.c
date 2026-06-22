#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
} Fragmento;

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

Fragmento basico(char c) {
    Estado *i = crear_estado(0);
    Estado *f = crear_estado(0);
    conectar(i, c, f);
    return (Fragmento){i, f};
}

Fragmento unir(Fragmento e1, Fragmento e2) {
    Estado *i = crear_estado(0);
    Estado *f = crear_estado(0);
    conectar(i, 'E', e1.inicio);
    conectar(i, 'E', e2.inicio);
    conectar(e1.fin, 'E', f);
    conectar(e2.fin, 'E', f);
    return (Fragmento){i, f};
}

Fragmento concatenar(Fragmento e1, Fragmento e2) {
    conectar(e1.fin, 'E', e2.inicio);
    return (Fragmento){e1.inicio, e2.fin};
}

Fragmento kleene(Fragmento e) {
    Estado *i = crear_estado(0);
    Estado *f = crear_estado(0);
    conectar(i, 'E', e.inicio);
    conectar(i, 'E', f);
    conectar(e.fin, 'E', e.inicio);
    conectar(e.fin, 'E', f);
    return (Fragmento){i, f};
}

Fragmento positiva(Fragmento e) {
    Estado *i = crear_estado(0);
    Estado *f = crear_estado(0);
    conectar(i, 'E', e.inicio);
    conectar(e.fin, 'E', e.inicio);
    conectar(e.fin, 'E', f);
    return (Fragmento){i, f};
}

Fragmento parser_thompson(char *regex) {
    Fragmento pila_frag[100];
    int top_f = -1;
    char pila_op[100];
    int top_o = -1;

    for (int i = 0; i < (int)strlen(regex); i++) {
        char c = regex[i];
        if (isalnum(c)) {
            pila_frag[++top_f] = basico(c);
            if (isalnum(regex[i+1]) || regex[i+1] == '(') pila_op[++top_o] = '.';
        } else if (c == '(') {
            pila_op[++top_o] = '(';
        } else if (c == ')') {
            while (top_o >= 0 && pila_op[top_o] != '(') {
                char op = pila_op[top_o--];
                Fragmento e2 = pila_frag[top_f--];
                Fragmento e1 = pila_frag[top_f--];
                if (op == '.') pila_frag[++top_f] = concatenar(e1, e2);
                else if (op == '|') pila_frag[++top_f] = unir(e1, e2);
            }
            top_o--;
            if (regex[i+1] == '*' || regex[i+1] == '+') continue;
            if (isalnum(regex[i+1]) || regex[i+1] == '(') pila_op[++top_o] = '.';
        } else if (c == '*' || c == '+') {
            if (c == '*') pila_frag[top_f] = kleene(pila_frag[top_f]);
            else pila_frag[top_f] = positiva(pila_frag[top_f]);
            if (isalnum(regex[i+1]) || regex[i+1] == '(') pila_op[++top_o] = '.';
        } else if (c == '|') {
            pila_op[++top_o] = '|';
        }
    }

    while (top_o >= 0) {
        char op = pila_op[top_o--];
        Fragmento e2 = pila_frag[top_f--];
        Fragmento e1 = pila_frag[top_f--];
        if (op == '.') pila_frag[++top_f] = concatenar(e1, e2);
        else if (op == '|') pila_frag[++top_f] = unir(e1, e2);
    }
    return pila_frag[0];
}

int simular(Estado *actual, const char *cadena, int pos) {
    if (cadena[pos] == '\0') {
        if (actual->esFinal) return 1;
        for (Transicion *t = actual->transiciones; t; t = t->siguiente) {
            if (t->simbolo == 'E' && simular(t->destino, cadena, pos)) return 1;
        }
        return 0;
    }
    for (Transicion *t = actual->transiciones; t; t = t->siguiente) {
        if (t->simbolo == cadena[pos] && simular(t->destino, cadena, pos + 1)) return 1;
        if (t->simbolo == 'E' && simular(t->destino, cadena, pos)) return 1;
    }
    return 0;
}

void mostrar_transiciones(Estado *e, int *visitado) {
    if (visitado[e->id]) return;
    visitado[e->id] = 1;
    for (Transicion *t = e->transiciones; t; t = t->siguiente) {
        printf("q%d --%c--> q%d\n", e->id, (t->simbolo == 'E' ? 'e' : t->simbolo), t->destino->id);
        mostrar_transiciones(t->destino, visitado);
    }
}

void dibujar_grafo_consola(Estado *e, int *visitado, int nivel) {
    if (visitado[e->id]) return;
    visitado[e->id] = 1;

    for (Transicion *t = e->transiciones; t; t = t->siguiente) {
        for (int i = 0; i < nivel; i++) printf("    ");
        printf("|--(%c)--> [q%d]", (t->simbolo == 'E' ? 'e' : t->simbolo), t->destino->id);
        if (t->destino->esFinal) printf(" (FINAL)");
        printf("\n");
        dibujar_grafo_consola(t->destino, visitado, nivel + 1);
    }
}

void escribir_nodos_dot(FILE *archivo, Estado *e, int *visitado) {
    if (visitado[e->id]) return;
    visitado[e->id] = 1;

    if (e->esFinal) {
        fprintf(archivo, "    q%d [shape=doublecircle];\n", e->id);
    } else {
        fprintf(archivo, "    q%d [shape=circle];\n", e->id);
    }

    for (Transicion *t = e->transiciones; t; t = t->siguiente) {
        fprintf(archivo, "    q%d -> q%d [label=\"%c\"];\n", e->id, t->destino->id, t->simbolo);
        escribir_nodos_dot(archivo, t->destino, visitado);
    }
}

void generar_archivo_dot(Estado *inicio, const char *nombre_archivo) {
    FILE *archivo = fopen(nombre_archivo, "w");
    if (!archivo) {
        printf("Error: No se pudo crear el archivo grafico '%s'\n", nombre_archivo);
        return;
    }

    fprintf(archivo, "digraph G {\n");
    fprintf(archivo, "    rankdir=LR;\n"); 
    fprintf(archivo, "    node [fixedsize=true, width=0.6, height=0.6];\n");
    fprintf(archivo, "    inic [shape= point, height=0.05, label=\"\"];\n");
    fprintf(archivo, "    inic -> q%d;\n", inicio->id); 

    int visitados_dot[1000] = {0};
    escribir_nodos_dot(archivo, inicio, visitados_dot);

    fprintf(archivo, "}\n");
    fclose(archivo);
    printf("\n¡Archivo grafico '%s' generado exitosamente!\n", nombre_archivo);
}

// Libera de forma recursiva los estados y la lista enlazada de transiciones
void liberar_automata(Estado *e, int *visitado) {
    if (e == NULL || visitado[e->id]) return;
    visitado[e->id] = 1;

    Transicion *actual = e->transiciones;
    while (actual != NULL) {
        Transicion *siguiente = actual->siguiente;
        liberar_automata(actual->destino, visitado); // Llamada recursiva al destino
        free(actual);                                // Libera la estructura de la transicion
        actual = siguiente;
    }
    free(e); // Libera el estado propiamente dicho
}

int main() {
    char regex[100];
    printf("=== SIMULADOR DE AUTOMATA- ALGORITMO DE THOMPSON ===\n");
    printf("Ingrese expresion regular: ");
    scanf("%s", regex);

    contador_estados = 0;
    Fragmento seleccionado = parser_thompson(regex);
    seleccionado.fin->esFinal = 1;

    printf("Construyendo automata....\n");
    printf("Estados creados: %d\n", contador_estados);
    printf("Estado initial: q%d\n", seleccionado.inicio->id);
    printf("Estado final: q%d\n", seleccionado.fin->id);

    generar_archivo_dot(seleccionado.inicio, "automata_thompson.dot");

    printf("\nTABLA DE TRANSICIONES:\n");
    int visitados_tabla[1000] = {0};
    mostrar_transiciones(seleccionado.inicio, visitados_tabla);

    printf("\nARBOL GRAFICO DE TRANSICIONES:\n");
    printf("[q%d]\n", seleccionado.inicio->id);
    int visitados_grafico[1000] = {0};
    dibujar_grafo_consola(seleccionado.inicio, visitados_grafico, 0);

    int op;
    do {
        printf("\nOpciones:\n1. Ingresar cadenas manualmente\n2. Leer desde archivo\n0. Salir\nSeleccione: ");
        scanf("%d", &op);

        if (op == 1) {
            char cad[256];
            while(1) {
                printf("Ingrese cadena (o 'salir'): ");
                scanf("%s", cad);
                if (strcmp(cad, "salir") == 0) break;
                printf("-> %s\n", simular(seleccionado.inicio, cad, 0) ? "ACEPTADA" : "RECHAZADA");
            }
        } else if (op == 2) {
            FILE *f = fopen("prueba.txt", "r");
            if(!f) { 
                printf("No se pudo abrir el archivo 'prueba.txt'\n"); 
            } else {
                char linea[256];
                while(fgets(linea, sizeof(linea), f)) {
                    linea[strcspn(linea, "\r\n")] = 0;
                    printf("%s -> %s\n", linea, simular(seleccionado.inicio, linea, 0) ? "ACEPTADA" : "RECHAZADA");
                }
                fclose(f);
            }
        } else if (op != 0) {
            printf("Opcon invalida. Intente de nuevo.\n");
        }
    } while (op != 0);

    // LIBERACION DE MEMORIA: Se ejecuta al presionar 0 para limpiar los recursos
    printf("\nLiberando memoria del automata...\n");
    int visitados_liberar[1000] = {0};
    liberar_automata(seleccionado.inicio, visitados_liberar);
    printf("Programa finalizado correctamente!\n");

    return 0;
}