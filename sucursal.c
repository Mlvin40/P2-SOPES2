#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define FIFO_PATH "/tmp/fifo_pagos"

typedef struct {
    char sucursal[50];
    int totalPagos;
    int cerrar;
} Reporte;

int main() {
    int fd;
    Reporte reporte;

    printf("========================================\n");
    printf("           SUCURSAL\n");
    printf("========================================\n");

    printf("Ingrese nombre de la sucursal o 'cerrar': ");
    scanf("%49s", reporte.sucursal);

    //Comprobar si se desea cerrar el dia
    if (strcmp(reporte.sucursal, "cerrar") == 0) {
        reporte.cerrar = 1;
        reporte.totalPagos = 0;
    } else {
        reporte.cerrar = 0;

        do {
            printf("Ingrese total de pagos procesados: ");
            scanf("%d", &reporte.totalPagos);

            if (reporte.totalPagos < 0) {
                printf("El total no puede ser negativo.\n");
            }

        } while (reporte.totalPagos < 0);
    }

    //Esperar hasta que el centro haya creado la FIFO
    while ((fd = open(FIFO_PATH, O_WRONLY)) == -1) {
        if (errno == ENOENT) {
            printf("Centro no disponible. Esperando...\n");
            sleep(1);
        } else {
            perror("Error al abrir la FIFO");
            return 1;
        }
    }

    //Enviar reporte al centro
    if (write(fd, &reporte, sizeof(Reporte)) == -1) {
        perror("Error al enviar reporte");
        close(fd);
        return 1;
    }

    if (reporte.cerrar == 1) {
        printf("Mensaje de cierre enviado al centro.\n");
    } else {
        printf("Reporte enviado correctamente.\n");
    }

    close(fd);

    return 0;
}