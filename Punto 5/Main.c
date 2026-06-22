#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ESTADOS_AFD 100
#define MAX_ESTADOS_AFN 1000

// --- Estructuras Originales del AFN ---
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

// --- Estructuras para la Construcción del AFD ---
typedef struct {
    int estados_afn[MAX_ESTADOS_AFN];
    int tam;
    int id_afd;
    int es_final_afd;
    int marcado;
} Subconjunto;

typedef struct {
    int origen_afd;
    char simbolo;
    int destino_afd;
} TransicionAFD;

int contador_estados = 0;

// --- Funciones Base de tu Codigo ---
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

// --- Funciones Auxiliares de Ordenamiento para Conjuntos ---
void ordenar_conjunto(int *conjunto, int tam) {
    for (int i = 0; i < tam - 1; i++) {
        for (int j = i + 1; j < tam; j++) {
            if (conjunto[i] > conjunto[j]) {
                int temp = conjunto[i];
                conjunto[i] = conjunto[j];
                conjunto[j] = temp;
            }
        }
    }
}

int agregar_al_conjunto(int *conjunto, int *tam, int id) {
    for (int i = 0; i < *tam; i++) {
        if (conjunto[i] == id) return 0; 
    }
    conjunto[*tam] = id;
    (*tam)++;
    ordenar_conjunto(conjunto, *tam);
    return 1;
}

// Busca punteros a estructuras Estado del AFN por medio de su ID numérico
Estado* buscar_estado_afn(Estado *actual, int id, int *visitado) {
    if (actual == NULL || visitado[actual->id]) return NULL;
    if (actual->id == id) return actual;
    visitado[actual->id] = 1;

    for (Transicion *t = actual->transiciones; t; t = t->siguiente) {
        Estado *res = buscar_estado_afn(t->destino, id, visitado);
        if (res != NULL) return res;
    }
    return NULL;
}

Estado* obtener_estado_por_id(Estado *raiz, int id) {
    int visitados[1000] = {0};
    return buscar_estado_afn(raiz, id, visitados);
}

// --- Operaciones del Algoritmo de Subconjuntos ---

void epsilon_clausura_recursiva(Estado *e, int *conjunto, int *tam) {
    if (e == NULL) return;
    agregar_al_conjunto(conjunto, tam, e->id);

    for (Transicion *t = e->transiciones; t; t = t->siguiente) {
        if (t->simbolo == 'E') {
            int t_antes = *tam;
            if (agregar_al_conjunto(conjunto, tam, t->destino->id)) {
                epsilon_clausura_recursiva(t->destino, conjunto, tam);
            }
        }
    }
}

void epsilon_clausura(Subconjunto *resultado, Estado *raiz) {
    int copia_tam = resultado->tam;
    int copia_estados[MAX_ESTADOS_AFN];
    for(int i=0; i<copia_tam; i++) copia_estados[i] = resultado->estados_afn[i];

    for (int i = 0; i < copia_tam; i++) {
        Estado *e = obtener_estado_por_id(raiz, copia_estados[i]);
        epsilon_clausura_recursiva(e, resultado->estados_afn, &(resultado->tam));
    }
}

void mover(Subconjunto origen, char simbolo, Subconjunto *resultado, Estado *raiz) {
    resultado->tam = 0;
    for (int i = 0; i < origen.tam; i++) {
        Estado *e = obtener_estado_por_id(raiz, origen.estados_afn[i]);
        if (e != NULL) {
            for (Transicion *t = e->transiciones; t; t = t->siguiente) {
                if (t->simbolo == simbolo) {
                    agregar_al_conjunto(resultado->estados_afn, &(resultado->tam), t->destino->id);
                }
            }
        }
    }
}

int comparar_subconjuntos(Subconjunto a, Subconjunto b) {
    if (a.tam != b.tam) return 0;
    for (int i = 0; i < a.tam; i++) {
        if (a.estados_afn[i] != b.estados_afn[i]) return 0;
    }
    return 1;
}

// --- Obtencion del Alfabeto ---
void extraer_alfabeto(Estado *e, int *visitado, char *alfabeto, int *tam_alfa) {
    if (e == NULL || visitado[e->id]) return;
    visitado[e->id] = 1;

    for (Transicion *t = e->transiciones; t; t = t->siguiente) {
        if (t->simbolo != 'E') {
            int existe = 0;
            for (int i = 0; i < *tam_alfa; i++) {
                if (alfabeto[i] == t->simbolo) { existe = 1; break; }
            }
            if (!existe) {
                alfabeto[*tam_alfa] = t->simbolo;
                (*tam_alfa)++;
            }
        }
        extraer_alfabeto(t->destino, visitado, alfabeto, tam_alfa);
    }
}

// --- Exportador DOT para el AFD ---
void generar_dot_afd(Subconjunto *estados_afd, int cont_afd, TransicionAFD *trans_afd, int cont_trans, const char *nombre_archivo) {
    FILE *archivo = fopen(nombre_archivo, "w");
    if (!archivo) return;

    fprintf(archivo, "digraph AFD {\n");
    fprintf(archivo, "    rankdir=LR;\n");
    fprintf(archivo, "    node [fixedsize=true, width=0.6, height=0.6];\n");
    fprintf(archivo, "    inic [shape=point, height=0.05, label=\"\"];\n");
    fprintf(archivo, "    inic -> U0;\n"); // Estado inicial siempre es U0

    for (int i = 0; i < cont_afd; i++) {
        if (estados_afd[i].es_final_afd) {
            fprintf(archivo, "    U%d [shape=doublecircle];\n", estados_afd[i].id_afd);
        } else {
            fprintf(archivo, "    U%d [shape=circle];\n", estados_afd[i].id_afd);
        }
    }

    for (int i = 0; i < cont_trans; i++) {
        fprintf(archivo, "    U%d -> U%d [label=\"%c\"];\n", trans_afd[i].origen_afd, trans_afd[i].destino_afd, trans_afd[i].simbolo);
    }

    fprintf(archivo, "}\n");
    fclose(archivo);
    printf("\n[AFD] Archivo grafico '%s' generado exitosamente.\n", nombre_archivo);
}

void liberar_automata(Estado *e, int *visitado) {
    if (e == NULL || visitado[e->id]) return;
    visitado[e->id] = 1;
    Transicion *actual = e->transiciones;
    while (actual != NULL) {
        Transicion *siguiente = actual->siguiente;
        liberar_automata(actual->destino, visitado);
        free(actual);
        actual = siguiente;
    }
    free(e);
}

// --- Algoritmo de conversion AFN a AFD ---
void convertir_afn_a_afd(Estado *inicio_afn, int id_final_afn) {
    Subconjunto estados_afd[MAX_ESTADOS_AFD];
    int contador_afd = 0;

    TransicionAFD transiciones_afd[MAX_ESTADOS_AFD * 10];
    int contador_transiciones = 0;

    char alfabeto[100];
    int tam_alfabeto = 0;
    int visitados_alfa[1000] = {0};
    extraer_alfabeto(inicio_afn, visitados_alfa, alfabeto, &tam_alfabeto);

    // 1. Crear el primer subconjunto: E-clausura(inicio)
    Subconjunto inicial;
    inicial.tam = 0;
    agregar_al_conjunto(inicial.estados_afn, &inicial.tam, inicio_afn->id);
    epsilon_clausura(&inicial, inicio_afn);
    
    inicial.id_afd = contador_afd++;
    inicial.marcado = 0;
    inicial.es_final_afd = 0;
    for(int i=0; i<inicial.tam; i++) {
        if(inicial.estados_afn[i] == id_final_afn) inicial.es_final_afd = 1;
    }
    estados_afd[0] = inicial;

    // 2. Ciclo principal del Algoritmo de Subconjuntos
    int pendientes = 1;
    while (pendientes) {
        int index_marcar = -1;
        for (int i = 0; i < contador_afd; i++) {
            if (!estados_afd[i].marcado) {
                index_marcar = i;
                break;
            }
        }

        if (index_marcar == -1) {
            pendientes = 0;
            break;
        }

        estados_afd[index_marcar].marcado = 1;

        // Evaluar para cada simbolo del alfabeto conocido
        for (int a = 0; a < tam_alfabeto; a++) {
            Subconjunto aux_mover;
            mover(estados_afd[index_marcar], alfabeto[a], &aux_mover, inicio_afn);

            if (aux_mover.tam > 0) {
                epsilon_clausura(&aux_mover, inicio_afn);

                // Verificar si este subconjunto ya existe en el AFD
                int existe_idx = -1;
                for (int e = 0; e < contador_afd; e++) {
                    if (comparar_subconjuntos(estados_afd[e], aux_mover)) {
                        existe_idx = e;
                        break;
                    }
                }

                int destino_id;
                if (existe_idx == -1) {
                    // Es un nuevo estado mapeado en el AFD
                    aux_mover.id_afd = contador_afd;
                    aux_mover.marcado = 0;
                    aux_mover.es_final_afd = 0;
                    for (int f = 0; f < aux_mover.tam; f++) {
                        if (aux_mover.estados_afn[f] == id_final_afn) aux_mover.es_final_afd = 1;
                    }
                    estados_afd[contador_afd] = aux_mover;
                    destino_id = contador_afd;
                    contador_afd++;
                } else {
                    destino_id = existe_idx;
                }

                // Guardar la transicion de transicion determinista
                transiciones_afd[contador_transiciones].origen_afd = estados_afd[index_marcar].id_afd;
                transiciones_afd[contador_transiciones].simbolo = alfabeto[a];
                transiciones_afd[contador_transiciones].destino_afd = destino_id;
                contador_transiciones++;
            }
        }
    }

    // 3. Imprimir resultados por Consola (Sin tildes)
    printf("\n===================================================\n");
    printf("   RESULTADO DE LA CONVERSION DE AFN A AFD\n");
    printf("===================================================\n");
    printf("Estados del AFD creados: %d\n", contador_afd);
    
    printf("\nEquivalencia de subconjuntos:\n");
    for (int i = 0; i < contador_afd; i++) {
        printf("  U%d = {", estados_afd[i].id_afd);
        for (int j = 0; j < estados_afd[i].tam; j++) {
            printf("q%d%s", estados_afd[i].estados_afn[j], (j == estados_afd[i].tam - 1) ? "" : ", ");
        }
        printf("}%s\n", estados_afd[i].es_final_afd ? " [FINAL]" : "");
    }

    printf("\nTabla de Transiciones del AFD:\n");
    for (int i = 0; i < contador_transiciones; i++) {
        printf("  U%d --(%c)--> U%d\n", transiciones_afd[i].origen_afd, transiciones_afd[i].simbolo, transiciones_afd[i].destino_afd);
    }

    // Guardar archivo .dot para graficar el AFD
    generar_dot_afd(estados_afd, contador_afd, transiciones_afd, contador_transiciones, "afd_resultado.dot");
}

int main() {
    char regex[100];
    printf("=== SIMULADOR DE AUTOMATA- ALGORITMO DE THOMPSON ===\n");
    printf("Ingrese expresion regular: ");
    scanf("%s", regex);

    contador_estados = 0;
    Fragmento seleccionado = parser_thompson(regex);
    seleccionado.fin->esFinal = 1;

    printf("Construyendo AFN (Thompson)....\n");
    printf("Estados AFN creados: %d\n", contador_estados);
    int id_final = seleccionado.fin->id;

    // Procesar conversion de AFN a AFD
    convertir_afn_a_afd(seleccionado.inicio, id_final);

    // Liberacion de recursos
    int visitados_liberar[1000] = {0};
    liberar_automata(seleccionado.inicio, visitados_liberar);
    printf("\nPrograma finalizado correctamente!\n");

    return 0;
}