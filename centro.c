#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
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
    int totalGeneral = 0;
    int cantidadReportes = 0;

    //Crear FIFO si todavia no existe
    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("Error al crear la FIFO");
        return 1;
    }

    printf("========================================\n");
    printf("       CENTRO DE OPERACIONES\n");
    printf("========================================\n");
    printf("Esperando reportes de las sucursales...\n\n");

    //Abrir FIFO para mantener el centro escuchando
    fd = open(FIFO_PATH, O_RDWR);

    if (fd == -1) {
        perror("Error al abrir la FIFO");
        return 1;
    }

    while (1) {
        if (read(fd, &reporte, sizeof(Reporte)) <= 0) {
            continue;
        }

        //Finalizar el dia
        if (reporte.cerrar == 1) {
            printf("\nMensaje 'cerrar' recibido.\n");
            break;
        }

        //Obtener hora exacta de llegada
        time_t ahora = time(NULL);
        struct tm *horaLocal = localtime(&ahora);
        char hora[20];

        strftime(hora, sizeof(hora), "%H:%M:%S", horaLocal);

        printf("Reporte recibido\n");
        printf("Sucursal: %s\n", reporte.sucursal);
        printf("Pagos procesados: %d\n", reporte.totalPagos);
        printf("Hora de llegada: %s\n\n", hora);

        totalGeneral += reporte.totalPagos;
        cantidadReportes++;
    }

    printf("\n========================================\n");
    printf("          RESUMEN DEL DIA\n");
    printf("========================================\n");
    printf("Reportes recibidos: %d\n", cantidadReportes);
    printf("Total de pagos procesados: %d\n", totalGeneral);

    close(fd);

    //Eliminar FIFO al terminar
    unlink(FIFO_PATH);

    return 0;
}