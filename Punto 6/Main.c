#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ESTADOS_AFD 100
#define MAX_ESTADOS_AFN 1000

// ============================================================================
// 1. ESTRUCTURAS DE DATOS
// ============================================================================

// --- Estructura para el Arbol de Analisis Sintactico ---
typedef struct NodoArbol {
    char info[16]; // Almacena el nombre del no-terminal o token (ej: "E", "T'", "a", "|")
    struct NodoArbol *hijos[4]; // Soporta hasta 4 hijos por regla gramatical
    int num_hijos;
} NodoArbol;

// --- Estructuras para los Automatas (AFN / AFD) ---
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

// Variables Globales
int contador_estados = 0;
int contador_nodos_arbol = 0; // Para identificar nodos unicos en el archivo DOT
const char *cursor;

// Prototipos del Parser LL(1) que ahora construyen el Arbol
NodoArbol* E();
NodoArbol* Edash();
NodoArbol* T();
NodoArbol* Tdash();
NodoArbol* F();
NodoArbol* Fdash();
NodoArbol* P();

// ============================================================================
// 2. FUNCIONES PARA CREACION DEL ARBOL SINTACTICO
// ============================================================================

NodoArbol* crear_nodo_arbol(const char *info) {
    NodoArbol *nodo = (NodoArbol*)malloc(sizeof(NodoArbol));
    strcpy(nodo->info, info);
    nodo->num_hijos = 0;
    for (int i = 0; i < 4; i++) nodo->hijos[i] = NULL;
    return nodo;
}

void agregar_hijo(NodoArbol *padre, NodoArbol *hijo) {
    if (padre != NULL && hijo != NULL && padre->num_hijos < 4) {
        padre->hijos[padre->num_hijos] = hijo;
        padre->num_hijos++;
    }
}

// Imprime el arbol de forma jerarquica en la consola
void mostrar_arbol_consola(NodoArbol *nodo, int nivel) {
    if (nodo == NULL) return;
    for (int i = 0; i < nivel; i++) printf("    ");
    printf("|-- %s\n", nodo->info);
    for (int i = 0; i < nodo->num_hijos; i++) {
        mostrar_arbol_consola(nodo->hijos[i], nivel + 1);
    }
}

// Escribe recursivamente las conexiones del arbol para el archivo DOT
void escribir_nodos_arbol_dot(FILE *archivo, NodoArbol *nodo, int *id_actual) {
    if (nodo == NULL) return;
    int mi_id = (*id_actual)++;
    
    // Definir la etiqueta del nodo
    fprintf(archivo, "    n%d [label=\"%s\"];\n", mi_id, nodo->info);
    
    int id_padre = mi_id;
    for (int i = 0; i < nodo->num_hijos; i++) {
        int id_hijo = *id_actual;
        fprintf(archivo, "    n%d -> n%d;\n", id_padre, id_hijo);
        escribir_nodos_arbol_dot(archivo, nodo->hijos[i], id_actual);
    }
}

void generar_archivo_arbol_dot(NodoArbol *raiz, const char *nombre_archivo) {
    FILE *archivo = fopen(nombre_archivo, "w");
    if (!archivo) {
        printf("Error al crear el archivo del arbol '%s'\n", nombre_archivo);
        return;
    }
    fprintf(archivo, "digraph ArbolSintactico {\n");
    fprintf(archivo, "    node [fontname=\"Arial\"];\n");
    int id_acumulado = 0;
    escribir_nodos_arbol_dot(archivo, raiz, &id_acumulado);
    fprintf(archivo, "}\n");
    fclose(archivo);
    printf("\n¡Archivo grafico del arbol '%s' generado exitosamente!\n", nombre_archivo);
}

void liberar_memoria_arbol(NodoArbol *nodo) {
    if (nodo == NULL) return;
    for (int i = 0; i < nodo->num_hijos; i++) {
        liberar_memoria_arbol(nodo->hijos[i]);
    }
    free(nodo);
}

// ============================================================================
// 3. IMPLEMENTACION DEL PARSER LL(1) (CONSTRUCCION DEL ARBOL)
// ============================================================================

// E -> T E'
NodoArbol* E() {
    NodoArbol *nodo = crear_nodo_arbol("E");
    NodoArbol *h1 = T();
    if (h1 == NULL) { free(nodo); return NULL; }
    agregar_hijo(nodo, h1);
    
    NodoArbol *h2 = Edash();
    if (h2 == NULL) { free(nodo); return NULL; }
    agregar_hijo(nodo, h2);
    return nodo;
}

// E' -> '|' T E' | $
NodoArbol* Edash() {
    NodoArbol *nodo = crear_nodo_arbol("E'");
    if (*cursor == '|') {
        char temp[2] = {*cursor, '\0'};
        cursor++; // Consumir '|'
        
        agregar_hijo(nodo, crear_nodo_arbol(temp));
        
        NodoArbol *h2 = T();
        if (h2 == NULL) { free(nodo); return NULL; }
        agregar_hijo(nodo, h2);
        
        NodoArbol *h3 = Edash();
        if (h3 == NULL) { free(nodo); return NULL; }
        agregar_hijo(nodo, h3);
        return nodo;
    }
    // Caso lambda ($)
    agregar_hijo(nodo, crear_nodo_arbol("$"));
    return nodo;
}

// T -> F T'
NodoArbol* T() {
    NodoArbol *nodo = crear_nodo_arbol("T");
    NodoArbol *h1 = F();
    if (h1 == NULL) { free(nodo); return NULL; }
    agregar_hijo(nodo, h1);
    
    NodoArbol *h2 = Tdash();
    if (h2 == NULL) { free(nodo); return NULL; }
    agregar_hijo(nodo, h2);
    return nodo;
}

// T' -> '.' F T' | $
NodoArbol* Tdash() {
    NodoArbol *nodo = crear_nodo_arbol("T'");
    if (*cursor == '.') {
        char temp[2] = {*cursor, '\0'};
        cursor++; // Consumir '.'
        
        agregar_hijo(nodo, crear_nodo_arbol(temp));
        
        NodoArbol *h2 = F();
        if (h2 == NULL) { free(nodo); return NULL; }
        agregar_hijo(nodo, h2);
        
        NodoArbol *h3 = Tdash();
        if (h3 == NULL) { free(nodo); return NULL; }
        agregar_hijo(nodo, h3);
        return nodo;
    }
    // Caso lambda ($)
    agregar_hijo(nodo, crear_nodo_arbol("$"));
    return nodo;
}

// F -> P F'
NodoArbol* F() {
    NodoArbol *nodo = crear_nodo_arbol("F");
    NodoArbol *h1 = P();
    if (h1 == NULL) { free(nodo); return NULL; }
    agregar_hijo(nodo, h1);
    
    NodoArbol *h2 = Fdash();
    if (h2 == NULL) { free(nodo); return NULL; }
    agregar_hijo(nodo, h2);
    return nodo;
}

// F' -> '*' F' | $
NodoArbol* Fdash() {
    NodoArbol *nodo = crear_nodo_arbol("F'");
    if (*cursor == '*') {
        char temp[2] = {*cursor, '\0'};
        cursor++; // Consumir '*'
        
        agregar_hijo(nodo, crear_nodo_arbol(temp));
        
        NodoArbol *h2 = Fdash();
        if (h2 == NULL) { free(nodo); return NULL; }
        agregar_hijo(nodo, h2);
        return nodo;
    }
    // Caso lambda ($)
    agregar_hijo(nodo, crear_nodo_arbol("$"));
    return nodo;
}

// P -> '(' E ')' | 'a' | 'b' | 'c'
NodoArbol* P() {
    NodoArbol *nodo = crear_nodo_arbol("P");
    if (*cursor == '(') {
        cursor++; // Consumir '('
        agregar_hijo(nodo, crear_nodo_arbol("("));
        
        NodoArbol *h2 = E();
        if (h2 == NULL) { free(nodo); return NULL; }
        agregar_hijo(nodo, h2);
        
        if (*cursor == ')') {
            cursor++; // Consumir ')'
            agregar_hijo(nodo, crear_nodo_arbol(")"));
            return nodo;
        } else {
            free(nodo);
            return NULL; // Error parentesis de cierre
        }
    } else if (*cursor == 'a' || *cursor == 'b' || *cursor == 'c') {
        char temp[2] = {*cursor, '\0'};
        cursor++; // Consumir token
        agregar_hijo(nodo, crear_nodo_arbol(temp));
        return nodo;
    }
    free(nodo);
    return NULL;
}

// ============================================================================
// 4. GENERADOR POST-PARSER DEL AUTOMATA DE THOMPSON DESDE EL ARBOL SINTACTICO
// ============================================================================

Estado* crear_estado_afn(int esFinal) {
    Estado *e = (Estado*)malloc(sizeof(Estado));
    e->id = contador_estados++;
    e->esFinal = esFinal;
    e->transiciones = NULL;
    return e;
}

void conectar_afn(Estado *origen, char simbolo, Estado *destino) {
    Transicion *t = (Transicion*)malloc(sizeof(Transicion));
    t->simbolo = simbolo;
    t->destino = destino;
    t->siguiente = origen->transiciones;
    origen->transiciones = t;
}

// Reconstruye de forma limpia la logica de Thompson recorriendo recursivamente nuestro arbol sintactico estructurado
Fragmento construir_thompson_desde_arbol(NodoArbol *nodo) {
    if (nodo == NULL) return (Fragmento){NULL, NULL};

    if (strcmp(nodo->info, "P") == 0) {
        if (nodo->num_hijos == 1) { // Terminal 'a', 'b' o 'c'
            char sim = nodo->hijos[0]->info[0];
            Estado *i = crear_estado_afn(0);
            Estado *f = crear_estado_afn(0);
            conectar_afn(i, sim, f);
            return (Fragmento){i, f};
        } else if (nodo->num_hijos == 3) { // Expresion entre parentesis: '(' E ')'
            return construir_thompson_desde_arbol(nodo->hijos[1]);
        }
    }
    
    if (strcmp(nodo->info, "F") == 0) {
        Fragmento p_frag = construir_thompson_desde_arbol(nodo->hijos[0]);
        NodoArbol *fdash = nodo->hijos[1];
        // Mientras existan cerraduras de Kleene consecutivas en F' -> '*' F'
        while (fdash != NULL && fdash->num_hijos > 0 && fdash->hijos[0]->info[0] == '*') {
            Estado *i = crear_estado_afn(0);
            Estado *f = crear_estado_afn(0);
            conectar_afn(i, 'E', p_frag.inicio);
            conectar_afn(i, 'E', f);
            conectar_afn(p_frag.fin, 'E', p_frag.inicio);
            conectar_afn(p_frag.fin, 'E', f);
            p_frag = (Fragmento){i, f};
            fdash = fdash->hijos[1];
        }
        return p_frag;
    }
    
    if (strcmp(nodo->info, "T") == 0) {
        Fragmento f_frag = construir_thompson_desde_arbol(nodo->hijos[0]);
        NodoArbol *tdash = nodo->hijos[1];
        // Concatenaciones sucesivas T' -> '.' F T'
        while (tdash != NULL && tdash->num_hijos > 0 && tdash->hijos[0]->info[0] == '.') {
            Fragmento f2_frag = construir_thompson_desde_arbol(tdash->hijos[1]);
            conectar_afn(f_frag.fin, 'E', f2_frag.inicio);
            f_frag = (Fragmento){f_frag.inicio, f2_frag.fin};
            tdash = tdash->hijos[2];
        }
        return f_frag;
    }
    
    if (strcmp(nodo->info, "E") == 0) {
        Fragmento t_frag = construir_thompson_desde_arbol(nodo->hijos[0]);
        NodoArbol *edash = nodo->hijos[1];
        // Uniones sucesivas E' -> '|' T E'
        while (edash != NULL && edash->num_hijos > 0 && edash->hijos[0]->info[0] == '|') {
            Fragmento t2_frag = construir_thompson_desde_arbol(edash->hijos[1]);
            Estado *i = crear_estado_afn(0);
            Estado *f = crear_estado_afn(0);
            conectar_afn(i, 'E', t_frag.inicio);
            conectar_afn(i, 'E', t2_frag.inicio);
            conectar_afn(t_frag.fin, 'E', f);
            conectar_afn(t2_frag.fin, 'E', f);
            t_frag = (Fragmento){i, f};
            edash = edash->hijos[2];
        }
        return t_frag;
    }

    return (Fragmento){NULL, NULL};
}

// ============================================================================
// 5. SECCION ADICIONAL DEL ALGORITMO DE CONVERSION AFN A AFD
// ============================================================================

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
    int visitados[MAX_ESTADOS_AFN] = {0};
    return buscar_estado_afn(raiz, id, visitados);
}

void epsilon_clausura_recursiva(Estado *e, int *conjunto, int *tam) {
    if (e == NULL) return;
    agregar_al_conjunto(conjunto, tam, e->id);

    for (Transicion *t = e->transiciones; t; t = t->siguiente) {
        if (t->simbolo == 'E') {
            if (agregar_al_conjunto(conjunto, tam, t->destino->id)) {
                epsilon_clausura_recursiva(t->destino, conjunto, tam);
            }
        }
    }
}

void epsilon_clausura(Subconjunto *resultado, Estado *raiz) {
    int copia_tam = resultado->tam;
    int copia_estados[MAX_ESTADOS_AFN];
    for (int i = 0; i < copia_tam; i++) copia_estados[i] = resultado->estados_afn[i];

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

void generar_dot_afd(Subconjunto *estados_afd, int cont_afd, TransicionAFD *trans_afd, int cont_trans, const char *nombre_archivo) {
    FILE *archivo = fopen(nombre_archivo, "w");
    if (!archivo) return;

    fprintf(archivo, "digraph AFD {\n");
    fprintf(archivo, "    rankdir=LR;\n");
    fprintf(archivo, "    node [fixedsize=true, width=0.6, height=0.6];\n");
    fprintf(archivo, "    inic [shape=point, height=0.05, label=\"\"];\n");
    fprintf(archivo, "    inic -> U0;\n");

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
    printf("[AFD] Archivo grafico '%s' generado exitosamente.\n", nombre_archivo);
}

void convertir_afn_a_afd(Estado *inicio_afn, int id_final_afn) {
    Subconjunto estados_afd[MAX_ESTADOS_AFD];
    int contador_afd = 0;

    TransicionAFD transiciones_afd[MAX_ESTADOS_AFD * 10];
    int contador_transiciones = 0;

    char alfabeto[100];
    int tam_alfabeto = 0;
    int visitados_alfa[MAX_ESTADOS_AFN] = {0};
    extraer_alfabeto(inicio_afn, visitados_alfa, alfabeto, &tam_alfabeto);

    Subconjunto inicial;
    inicial.tam = 0;
    agregar_al_conjunto(inicial.estados_afn, &inicial.tam, inicio_afn->id);
    epsilon_clausura(&inicial, inicio_afn);
    
    inicial.id_afd = contador_afd++;
    inicial.marcado = 0;
    inicial.es_final_afd = 0;
    for(int i = 0; i < inicial.tam; i++) {
        if(inicial.estados_afn[i] == id_final_afn) inicial.es_final_afd = 1;
    }
    estados_afd[0] = inicial;

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

        for (int a = 0; a < tam_alfabeto; a++) {
            Subconjunto aux_mover;
            mover(estados_afd[index_marcar], alfabeto[a], &aux_mover, inicio_afn);

            if (aux_mover.tam > 0) {
                epsilon_clausura(&aux_mover, inicio_afn);

                int existe_idx = -1;
                for (int e = 0; e < contador_afd; e++) {
                    if (comparar_subconjuntos(estados_afd[e], aux_mover)) {
                        existe_idx = e;
                        break;
                    }
                }

                int destino_id;
                if (existe_idx == -1) {
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

                transiciones_afd[contador_transiciones].origen_afd = estados_afd[index_marcar].id_afd;
                transiciones_afd[contador_transiciones].simbolo = alfabeto[a];
                transiciones_afd[contador_transiciones].destino_afd = destino_id;
                contador_transiciones++;
            }
        }
    }

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
    printf("\n");

    generar_dot_afd(estados_afd, contador_afd, transiciones_afd, contador_transiciones, "afd_resultado.dot");
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

// ============================================================================
// 6. MAIN DEL PROGRAMA
// ============================================================================

int main() {
    char regex[100];
    printf("=== PARSER LL(1) CON VISUALIZACION DE ARBOL SINTACTICO ===\n");
    printf("Alfabeto aceptado: {a, b, c}\n");
    printf("Operadores admitidos: '.' (Concatenacion), '|' (Union), '*' (Kleene)\n");
    printf("Ingrese expresion regular (ej: a.b|c*): ");
    scanf("%s", regex);

    cursor = regex;
    contador_estados = 0;

    // 1. Ejecutar el Analizador Sintactico LL(1) construyendo la estructura del Arbol
    NodoArbol *raiz_arbol = E();

    // 2. Comprobar si toda la expresion fue procesada sin errores
    if (raiz_arbol != NULL && *cursor == '\0') {
        printf("\n¡Cadena sintacticamente CORRECTA!\n");
        
        // --- Mostrar Arbol en Consola ---
        printf("\nARBOL DE ANALISIS SINTACTICO EN CONSOLA:\n");
        mostrar_arbol_consola(raiz_arbol, 0);
        
        // --- Generar Arbol en Formato .DOT ---
        generar_archivo_arbol_dot(raiz_arbol, "arbol_sintactico.dot");

        // 3. Crear el AFN (Thompson) navegando por la estructura del arbol
        Fragmento seleccionado = construir_thompson_desde_arbol(raiz_arbol);
        
        if (seleccionado.inicio != NULL) {
            seleccionado.fin->esFinal = 1;
            int id_final = seleccionado.fin->id;

            // 4. Transformar a AFD
            convertir_afn_a_afd(seleccionado.inicio, id_final);

            // Liberar memoria del automata
            int visitados_liberar[MAX_ESTADOS_AFN] = {0};
            liberar_automata(seleccionado.inicio, visitados_liberar);
        }

        // Liberar memoria del arbol
        liberar_memoria_arbol(raiz_arbol);
        
    } else {
        printf("\n¡Error Sintactico! La expresion no pertenece a la gramatica LL(1).\n");
        if (raiz_arbol != NULL) liberar_memoria_arbol(raiz_arbol);
    }

    printf("Programa finalizado correctamente!\n");
    return 0;
}