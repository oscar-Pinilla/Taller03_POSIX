/* **************************************************************************
 * Pontificia Universidad Javeriana
 * Facultad de Ingeniería - Ingeniería de Sistemas
 * Asignatura: Sistemas Operativos
 * Proyecto: Implementación del Productor (IPC con Memoria Compartida y Semáforos)
 * Autor: Oscar Pinilla y David Pedraza
 * Descripcion: crea y mapea un objeto de memoria compartida donde se aloja un buffer circular para 
 *el intercambio de datos. La sincronización se gestiona mediante semáforos con 
 *nombre que controlan la disponibilidad de espacios vacíos y elementos llenos, asegurando que 
 *el productor se bloquee cuando el buffer alcanza su capacidad máxima y evitando condiciones de carrera.
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
    /* Abre semáforos: vacio inicia en 5, lleno inicia en 0 */
    sem_t *vacio = sem_open("/vacio", O_CREAT, 0644, BUFFER);
    sem_t *lleno = sem_open("/lleno", O_CREAT, 0644, 0);

    if (vacio == SEM_FAILED || lleno == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    /* Abre el objeto de memoria compartida */
    int shm_fd = shm_open("/memoria_compartida", O_CREAT | O_RDWR, 0644);
    if (shm_fd < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    /* Define el tamaño de la memoria compartida */
    ftruncate(shm_fd, sizeof(compartir_datos));

    /* Mapea la memoria al espacio del proceso */
    compartir_datos *compartir = mmap(NULL, sizeof(compartir_datos), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    compartir->entrada = 0;

    for (int i = 1; i <= 10; i++) {
        /* Espera a que haya espacio disponible */
        sem_wait(vacio);
        
        /* Inserta el dato en el bus y actualiza índice */
        compartir->bus[compartir->entrada] = i;
        printf("Productor: Produce%d\n", i);
        compartir->entrada = (compartir->entrada+1) % BUFFER;
        
        /* Indica que hay un nuevo dato listo */
        sem_post(lleno);
        sleep(1);
    }

    /* Libera y cierra la memoria mapeada */
    munmap(compartir, sizeof(compartir_datos));
    close(shm_fd);
    
    /* Cierra y elimina los semáforos del sistema */
    sem_close(vacio);
    sem_unlink("/vacio");
    sem_close(lleno);
    sem_unlink("/lleno");
    
    /* Elimina el objeto de memoria compartida */
    shm_unlink("/memoria_compartida");

    return 0;
}
