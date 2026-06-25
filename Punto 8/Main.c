#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ESTADO_U0 0
#define ESTADO_U1 1
#define ESTADO_U2 2
#define ESTADO_U3 3
#define ESTADO_U4 4
#define ESTADO_ERROR -1

typedef struct NodoTransicion {
    int estado_origen;
    char simbolo;
    int estado_destino;
    struct NodoTransicion *siguiente;
} NodoTransicion;

NodoTransicion *cabeza_automata = NULL;

void agregar_transicion(int origen, char simbolo, int destino) {
    NodoTransicion *nuevo = (NodoTransicion*)malloc(sizeof(NodoTransicion));
    nuevo->estado_origen = origen;
    nuevo->simbolo = simbolo;
    nuevo->estado_destino = destino;
    nuevo->siguiente = cabeza_automata;
    cabeza_automata = nuevo;
}

void inicializar_afd_subcadena() {
    agregar_transicion(ESTADO_U0, 'h', ESTADO_U1);
    agregar_transicion(ESTADO_U0, 'o', ESTADO_U2);
    agregar_transicion(ESTADO_U0, 'l', ESTADO_U0);
    agregar_transicion(ESTADO_U0, 'a', ESTADO_U0);

    agregar_transicion(ESTADO_U1, 'h', ESTADO_U1);
    agregar_transicion(ESTADO_U1, 'o', ESTADO_U2);
    agregar_transicion(ESTADO_U1, 'l', ESTADO_U0);
    agregar_transicion(ESTADO_U1, 'a', ESTADO_U0);

    agregar_transicion(ESTADO_U2, 'l', ESTADO_U3);
    agregar_transicion(ESTADO_U2, 'h', ESTADO_U1);
    agregar_transicion(ESTADO_U2, 'o', ESTADO_U0);
    agregar_transicion(ESTADO_U2, 'a', ESTADO_U0);

    agregar_transicion(ESTADO_U3, 'a', ESTADO_U4);
    agregar_transicion(ESTADO_U3, 'h', ESTADO_U1);
    agregar_transicion(ESTADO_U3, 'o', ESTADO_U2);
    agregar_transicion(ESTADO_U3, 'l', ESTADO_U0);

    agregar_transicion(ESTADO_U4, 'h', ESTADO_U4);
    agregar_transicion(ESTADO_U4, 'o', ESTADO_U4);
    agregar_transicion(ESTADO_U4, 'l', ESTADO_U4);
    agregar_transicion(ESTADO_U4, 'a', ESTADO_U4);
}

int buscar_siguiente_estado(int estado_actual, char simbolo) {
    NodoTransicion *actual = cabeza_automata;
    while (actual != NULL) {
        if (actual->estado_origen == estado_actual && actual->simbolo == simbolo) {
            return actual->estado_destino;
        }
        actual = actual->siguiente;
    }

    if (estado_actual == ESTADO_U4) {
        return ESTADO_U4;
    }

    if (simbolo == 'o') {
        return ESTADO_U2;
    }

    return ESTADO_U0;
}

int simular_afd(const char *cadena) {
    int estado_actual = ESTADO_U0;
    int i = 0;

    printf("  Estado inicial: U%d\n", estado_actual);

    while (cadena[i] != '\0') {
        char simbolo = cadena[i];
        int estado_anterior = estado_actual;

        estado_actual = buscar_siguiente_estado(estado_anterior, simbolo);

        printf("    Lee: '%c' | Transicion: U%d --(%c)--> U%d\n", simbolo, estado_anterior, simbolo, estado_actual);
        i++;
    }

    if (estado_actual == ESTADO_U4) {
        return 1;
    } else {
        return 0;
    }
}

void liberar_nodos_automata() {
    NodoTransicion *actual = cabeza_automata;
    while (actual != NULL) {
        NodoTransicion *siguiente = actual->siguiente;
        free(actual);
        actual = siguiente;
    }
    cabeza_automata = NULL;
}

int main() {
    char nombre_archivo[] = "prueba.txt";
    char linea[256];
    FILE *archivo;

    inicializar_afd_subcadena();

    printf("=========================================================\n");
    printf("  SIMULADOR AFD PARA EVALUAR SUBCADENA: h*.o.l.a        \n");
    printf("=========================================================\n");
    
    archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        printf("Error: No se pudo abrir el archivo '%s'.\n", nombre_archivo);
        liberar_nodos_automata();
        return 1;
    }

    printf("Leyendo cadenas desde '%s'...\n", nombre_archivo);
    int num_cadena = 1;

    while (fgets(linea, sizeof(linea), archivo) != NULL) {
        linea[strcspn(linea, "\r\n")] = 0;
        if (strlen(linea) == 0) continue;

        printf("\n---------------------------------------------------------\n");
        printf("Cadena #%d: \"%s\"\n", num_cadena++, linea);
        printf("---------------------------------------------------------\n");
        
        int resultado = simular_afd(linea);

        if (resultado) {
            printf("Resultado: CADENA ACEPTADA (Contiene el patron h*.o.l.a)\n");
        } else {
            printf("Resultado: CADENA RECHAZADA\n");
        }
    }

    fclose(archivo);
    liberar_nodos_automata();

    printf("\n=========================================================\n");
    printf("Proceso finalizado. Memoria liberada correctamente.\n");
    printf("=========================================================\n");

    return 0;
}