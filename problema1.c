#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define VERIFICACION 1
#define PAGO 2

typedef struct {
    int tipo;
    char palabra[100];
    int tarjeta;
} Mensaje;

//Obtiene la longitud de una cadena
int longitudCadena(char cadena[]) {
    int longitud = 0;

    while (cadena[longitud] != '\0') {
        longitud++;
    }

    return longitud;
}

//Invierte la palabra caracter por caracter
void invertirCadena(char origen[], char destino[]) {
    int longitud = longitudCadena(origen);
    int j = 0;

    for (int i = longitud - 1; i >= 0; i--) {
        destino[j] = origen[i];
        j++;
    }

    destino[j] = '\0';
}

int main() {
    int padre_hijo[2];
    int hijo_padre[2];
    pid_t pid;

    //Crear las dos tuberias
    if (pipe(padre_hijo) == -1) {
        perror("Error al crear tuberia padre-hijo");
        return 1;
    }

    if (pipe(hijo_padre) == -1) {
        perror("Error al crear tuberia hijo-padre");
        return 1;
    }

    pid = fork();

    if (pid < 0) {
        perror("Error al crear el proceso hijo");
        return 1;
    }

    if (pid > 0) {
        //Proceso padre
        Mensaje mensaje;
        char opcion;

        close(padre_hijo[0]);
        close(hijo_padre[1]);

        printf("========================================\n");
        printf("       SISTEMA DE PAGO EN LINEA\n");
        printf("========================================\n");

        printf("Desea verificar el estado del canal? (s/n): ");
        scanf(" %c", &opcion);

        if (opcion == 's' || opcion == 'S') {
            mensaje.tipo = VERIFICACION;

            printf("Ingrese una palabra para verificar el canal: ");
            scanf("%99s", mensaje.palabra);

            write(padre_hijo[1], &mensaje, sizeof(Mensaje));

            //Esperar confirmacion del hijo
            char confirmacion;
            read(hijo_padre[0], &confirmacion, sizeof(confirmacion));
        } else {
            printf("\nVerificacion omitida.\n");
        }

        do {
            printf("\nIngrese numero de tarjeta (1000 - 9999): ");
            scanf("%d", &mensaje.tarjeta);

            if (mensaje.tarjeta < 1000 || mensaje.tarjeta > 9999) {
                printf("Numero de tarjeta invalido.\n");
            }

        } while (mensaje.tarjeta < 1000 || mensaje.tarjeta > 9999);

        mensaje.tipo = PAGO;

        //Enviar tarjeta al hijo
        write(padre_hijo[1], &mensaje, sizeof(Mensaje));
        close(padre_hijo[1]);

        char resultado[50];

        //Recibir resultado del pago
        read(hijo_padre[0], resultado, sizeof(resultado));

        printf("\nResultado final: %s\n", resultado);

        close(hijo_padre[0]);
        wait(NULL);

    } else {
        //Proceso hijo
        Mensaje mensaje;

        close(padre_hijo[1]);
        close(hijo_padre[0]);

        while (read(padre_hijo[0], &mensaje, sizeof(Mensaje)) > 0) {

            if (mensaje.tipo == VERIFICACION) {
                char palabraInvertida[100];

                invertirCadena(mensaje.palabra, palabraInvertida);

                printf("\n[HIJO] Canal activo.\n");
                printf("[HIJO] Palabra recibida: %s\n", mensaje.palabra);
                printf("[HIJO] Palabra invertida: %s\n", palabraInvertida);

                fflush(stdout);

                //Avisar al padre que termino la verificacion
                char confirmacion = 'O';
                write(hijo_padre[1], &confirmacion, sizeof(confirmacion));
            }

            else if (mensaje.tipo == PAGO) {
                char aprobado[] = "PAGO_APROBADO";
                char rechazado[] = "PAGO_RECHAZADO";

                printf("\n[HIJO] Procesando tarjeta %d...\n", mensaje.tarjeta);
                fflush(stdout);

                if (mensaje.tarjeta % 2 == 0) {
                    write(hijo_padre[1], aprobado, sizeof(aprobado));
                } else {
                    write(hijo_padre[1], rechazado, sizeof(rechazado));
                }
            }
        }

        close(padre_hijo[0]);
        close(hijo_padre[1]);
        exit(0);
    }

    return 0;
}