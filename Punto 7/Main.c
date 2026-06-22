#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definición de los estados del AFD predeterminado
#define ESTADO_U0 0
#define ESTADO_U1 1
#define ESTADO_ERROR -1

// Función para simular el AFD predeterminado paso a paso
int simular_afd_predeterminado(const char *cadena) {
    int estado_actual = ESTADO_U0;
    int i = 0;

    printf("\nProcesando la cadena...\n");
    printf("Estado inicial: U%d\n", estado_actual);

    while (cadena[i] != '\0') {
        char simbolo = cadena[i];
        int estado_anterior = estado_actual;

        // Matriz de transiciones lógica
        if (estado_actual == ESTADO_U0) {
            if (simbolo == 'a' || simbolo == 'b') {
                estado_actual = ESTADO_U0;
            } else if (simbolo == 'c') {
                estado_actual = ESTADO_U1;
            } else {
                estado_actual = ESTADO_ERROR;
            }
        } else if (estado_actual == ESTADO_U1) {
            // Desde el estado final U1, cualquier símbolo adicional es un error
            estado_actual = ESTADO_ERROR;
        }

        // Si entramos en estado de error, quebramos el bucle inmediatamente
        if (estado_actual == ESTADO_ERROR) {
            printf("  Caracter '%c' invalido o transicion inexistente. -> Estado ERROR\n", simbolo);
            break;
        }

        printf("  Lee: '%c' | Transicion: U%d --(%c)--> U%d\n", simbolo, estado_anterior, simbolo, estado_actual);
        i++;
    }

    // El autómata acepta si termina exactamente en el estado final U1
    if (estado_actual == ESTADO_U1) {
        return 1; // ACEPTADA
    } else {
        return 0; // RECHAZADA
    }
}

int main() {
    char cadena[256];
    int opcion = 1;

    printf("=========================================================\n");
    printf("  SIMULADOR AFD PREDETERMINADO PARA LA ER: (a|b)*c       \n");
    printf("=========================================================\n");
    printf("Alfabeto valido: {a, b, c}\n\n");

    // El bucle continuará mientras el usuario elija la opción 1
    while (opcion == 1) {
        printf("Ingrese la cadena a evaluar: ");
        scanf("%255s", cadena);

        // Evaluar la cadena en el AFD
        int resultado = simular_afd_predeterminado(cadena);

        // Formato visual de los resultados
        printf("---------------------------------------------------------\n");
        if (resultado) {
            printf("Resultado: CADENA ACEPTADA\n");
        } else {
            printf("Resultado: CADENA RECHAZADA\n");
        }
        printf("=========================================================\n");

        // Menú interactivo de opciones
        printf("\nQue desea hacer?\n");
        printf("1. Probar otra cadena\n");
        printf("2. Salir\n");
        printf("Seleccione una opcion: ");
        scanf("%d", &opcion);
        printf("\n");
    }

    printf("Programa finalizado correctamente.\n");
    return 0;
}