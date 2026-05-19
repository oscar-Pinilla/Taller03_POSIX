/* **************************************************************************
 * Pontificia Universidad Javeriana
 * Facultad de Ingeniería - Ingeniería de Sistemas
 * Asignatura: Sistemas Operativos
 * Autores: Oscar Pinilla y David Pedraza
 * Descripcion: accede al mismo segmento de memoria y utiliza semáforos para coordinar 
 *su ejecución con el productor, esperando activamente a que existan elementos disponibles 
 *para procesar. Además, gestiona la actualización del índice de salida del buffer circular y se encarga de 
 *la limpieza final de los recursos del sistema, como la desvinculación 
 *de la memoria compartida y el cierre de los semáforos utilizados.
 * **************************************************************************
 */



#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER 5

typedef struct {
    int bus[BUFFER];
    int entrada;
    int salida;
} compartir_datos;




int main() {
    /* Apertura de semáforos existentes (vacio y lleno) */
    sem_t *vacio = sem_open("/vacio", 0);
    sem_t *lleno = sem_open("/lleno", 0);
    
    if (vacio == SEM_FAILED || lleno == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    /* Acceso al objeto de memoria compartida creado por el productor */
    int fd_compartido = shm_open("/memoria_compartida", O_RDWR, 0644);
    if (fd_compartido < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    /* Mapeo de la estructura de datos compartida en el espacio de direcciones */
    compartir_datos *compartir = mmap(NULL, sizeof(compartir_datos), PROT_READ | PROT_WRITE, MAP_SHARED, fd_compartido, 0);
    
    /* Inicialización del índice de lectura */
    compartir->salida = 0;

    /* Ciclo de Consumo: Procesa 10 elementos */
    for (int i = 1; i <= 10; i++) {
        /* Espera a que el productor coloque algo (lleno > 0) */
        sem_wait(lleno);
        
        /* Sección Crítica: Extrae el dato y actualiza el índice circular */
        int item = compartir->bus[compartir->salida];
        printf("Consumidor: Consume %d\n", item);
        compartir->salida = (compartir->salida+1) % BUFFER;
        
        /* Indica que ahora hay un espacio vacío más en el bus */
        sem_post(vacio);
        
        /* Simula tiempo de procesamiento del consumidor */
        sleep(2);  
    }

    /* Limpieza y cierre de recursos */
    munmap(compartir, sizeof(compartir_datos));
    close(fd_compartido);
    
    /* Cierre y desvinculación de objetos del sistema */
    sem_close(lleno);
    sem_unlink("/lleno");
    shm_unlink("/memoria_compartida");
    
    return 0;
}
