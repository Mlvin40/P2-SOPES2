#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define TOTAL_PEDIDOS 20

int main() {
    int tuberia[2];
    pid_t hijo1, hijo2;

    if (pipe(tuberia) == -1) {
        perror("Error al crear la tuberia");
        return 1;
    }

    hijo1 = fork();

    if (hijo1 < 0) {
        perror("Error al crear hijo 1");
        return 1;
    }

    if (hijo1 == 0) {
        //Estacion 1
        close(tuberia[1]);

        int pedido;
        int cantidadPedidos = 0;
        int totalCamisas = 0;

        while (read(tuberia[0], &pedido, sizeof(int)) > 0) {
            cantidadPedidos++;
            totalCamisas += pedido;

            printf("[ESTACION 1] Pedido recibido: %d camisas\n", pedido);

            //Simula tiempo de procesamiento
            usleep(100000);
        }

        printf("\n===== REPORTE ESTACION 1 =====\n");
        printf("Pedidos procesados: %d\n", cantidadPedidos);
        printf("Total de camisas: %d\n", totalCamisas);

        close(tuberia[0]);
        exit(0);
    }

    hijo2 = fork();

    if (hijo2 < 0) {
        perror("Error al crear hijo 2");
        return 1;
    }

    if (hijo2 == 0) {
        //Estacion 2
        close(tuberia[1]);

        int pedido;
        int cantidadPedidos = 0;
        int totalCamisas = 0;

        while (read(tuberia[0], &pedido, sizeof(int)) > 0) {
            cantidadPedidos++;
            totalCamisas += pedido;

            printf("[ESTACION 2] Pedido recibido: %d camisas\n", pedido);

            //Simula tiempo de procesamiento
            usleep(100000);
        }

        printf("\n===== REPORTE ESTACION 2 =====\n");
        printf("Pedidos procesados: %d\n", cantidadPedidos);
        printf("Total de camisas: %d\n", totalCamisas);

        close(tuberia[0]);
        exit(0);
    }

    //Proceso padre
    close(tuberia[0]);

    int pedidos[TOTAL_PEDIDOS];

    printf("========================================\n");
    printf("       REGISTRO DE PEDIDOS\n");
    printf("========================================\n");

    for (int i = 0; i < TOTAL_PEDIDOS; i++) {
        do {
            printf("Ingrese unidades del pedido %d (1 - 100): ", i + 1);
            scanf("%d", &pedidos[i]);

            if (pedidos[i] < 1 || pedidos[i] > 100) {
                printf("Cantidad invalida. Intente nuevamente.\n");
            }

        } while (pedidos[i] < 1 || pedidos[i] > 100);
    }

    printf("\nTodos los pedidos fueron registrados.\n");
    printf("Enviando pedidos a las estaciones...\n\n");

    //Enviar todos los pedidos
    for (int i = 0; i < TOTAL_PEDIDOS; i++) {
        write(tuberia[1], &pedidos[i], sizeof(int));
    }

    //Cerrar escritura para indicar que no hay mas pedidos
    close(tuberia[1]);

    waitpid(hijo1, NULL, 0);
    waitpid(hijo2, NULL, 0);

    printf("\nTodos los pedidos fueron procesados.\n");

    return 0;
}