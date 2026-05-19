/* **************************************************************************
 * Pontificia Universidad Javeriana
 * Facultad de Ingeniería - Ingeniería de Sistemas
 * Asignatura: Sistemas Operativos
 * Sincronización de Hilos 
 * Autores: Oscar Samuel Pinilla y David Pedraza
 * Descripcion: Este programa implementa el modelo Productor-Consumidor utilizando 
 * la biblioteca Pthreads. Se gestiona un buffer circular compartido donde 10 hilos 
 * productores insertan mensajes, mientras un hilo spooler los extrae y muestra 
 * por pantalla, garantizando la sincronización mediante mutexes y variables de condiciónn.
 * **************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

/* Capacidad máxima del buffer circular compartido */
#define MAX_BUFFERS 10

/* Estructura de datos del buffer y punteros de gestión circular */
char buf [MAX_BUFFERS] [100];
int buffer_index;        /* Rastrea la posición de inserción para productores */
int buffer_print_index;  /* Rastrea la posición de lectura para el spooler */

/* * Primitivas de sincronización POSIX:
 * El mutex garantiza la exclusión mutua en la sección crítica.
 * Las variables de condición permiten la espera pasiva (sin consumo de CPU).
 */
pthread_mutex_t buf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buf_cond = PTHREAD_COND_INITIALIZER;   /* Bloquea productores si el buffer está lleno */
pthread_cond_t spool_cond = PTHREAD_COND_INITIALIZER;  /* Bloquea al spooler si el buffer está vacío */

/* Variables de control para el estado de ocupación del buffer */
int buffers_available = MAX_BUFFERS; /* Contador de espacios vacíos */
int lines_to_print = 0;              /* Contador de mensajes listos para imprimir */

void *producer (void *arg);
void *spooler (void *arg);

int main (int argc, char **argv){
    pthread_t tid_producer [10], tid_spooler;
    int i, r;

    /* Inicialización de índices antes de lanzar los hilos */
    buffer_index = buffer_print_index = 0;

    /* * Creación del hilo Spooler (Consumidor):
     * Su función es monitorizar el buffer y realizar la salida estándar de datos.
     */
    if ((r = pthread_create (&tid_spooler, NULL, spooler, NULL)) != 0) {
        fprintf (stderr, "Error = %d (%s)\n", r, strerror (r)); exit (1);
    }

    /* * Creación de la flota de 10 hilos productores:
     * Cada hilo recibe su ID único mediante un puntero a un entero.
     */
    int thread_no [10];
    for (i = 0; i < 10; i++) {
        thread_no [i] = i;
        if ((r = pthread_create (&tid_producer [i], NULL, producer, (void *) &thread_no [i])) != 0) {
            fprintf (stderr, "Error = %d (%s)\n", r, strerror (r)); exit (1);
        }
    }

    /* * pthread_join: El hilo principal se bloquea hasta que todos los 
     * productores terminen sus 10 ciclos de inserción.
     */
    for (i = 0; i < 10; i++)
        if ((r = pthread_join (tid_producer [i], NULL)) != 0) {
            fprintf (stderr, "Error = %d (%s)\n", r, strerror (r)); exit (1);
        }
    
    /* * Bucle de espera activa controlada: Asegura que el spooler termine 
     * de imprimir los últimos mensajes depositados antes del cierre.
     */
    while (lines_to_print) sleep (1);

    /* * pthread_cancel: Terminación forzosa del hilo spooler, ya que 
     * este posee un bucle infinito de ejecución.
     */
    if ((r = pthread_cancel (tid_spooler)) != 0) {
        fprintf (stderr, "Error = %d (%s)\n", r, strerror (r)); exit (1);
    }

    exit (0);
}

void *producer (void *arg){
    int i, r;
    int my_id = *((int *) arg); /* Casting del argumento para recuperar el ID del hilo */
    int count = 0;

    for (i = 0; i < 10; i++) {
        /* Intento de bloqueo del Mutex para entrar a la sección crítica */
        if ((r = pthread_mutex_lock (&buf_mutex)) != 0) {
            fprintf (stderr, "Error = %d (%s)\n", r, strerror (r)); exit (1);
        }
            /* * Protocolo de seguridad: Se usa 'while' para evitar el problema de 
             * despertares espurios. Si no hay espacio, el hilo se duerme.
             */
            while (!buffers_available) 
                pthread_cond_wait (&buf_cond, &buf_mutex);

            /* SECCIÓN CRÍTICA: Inserción de datos en el buffer circular */
            int j = buffer_index;
            buffer_index++;
            if (buffer_index == MAX_BUFFERS)
                buffer_index = 0; /* Retorno al inicio al alcanzar el límite */
            
            buffers_available--; /* Reducción de la disponibilidad */

            /* Formateo y almacenamiento del mensaje en la memoria compartida */
            sprintf (buf [j], "Thread %d: %d\n", my_id, ++count);
            lines_to_print++;

            /* pthread_cond_signal: Despierta al spooler si estaba esperando datos */
            pthread_cond_signal (&spool_cond);

        /* Salida de la sección crítica: Liberación obligatoria del Mutex */
        if ((r = pthread_mutex_unlock (&buf_mutex)) != 0) {
            fprintf (stderr, "Error = %d (%s)\n", r, strerror (r)); exit (1);
        }
    
        /* Suspensión de 1s para permitir el entrelazado de otros hilos productores */
        sleep (1);
    }
    return NULL;
}

void *spooler (void *arg){
    int r;

    while (1) {  
        /* Bloqueo del mutex para acceso exclusivo a los contadores de estado */
        if ((r = pthread_mutex_lock (&buf_mutex)) != 0) {
            fprintf (stderr, "Error = %d (%s)\n", r, strerror (r)); exit (1);
        }
            /* * Si el contador de líneas está en cero, el hilo se bloquea hasta que 
             * un productor emita una señal a través de spool_cond.
             */
            while (!lines_to_print) 
                pthread_cond_wait (&spool_cond, &buf_mutex);

            /* SECCIÓN CRÍTICA: Lectura del buffer y salida por consola */
            printf ("%s", buf [buffer_print_index]);
            lines_to_print--;

            /* Actualización del puntero de lectura de forma circular */
            buffer_print_index++;
            if (buffer_print_index == MAX_BUFFERS)
               buffer_print_index = 0;

            /* * Notificación: Se avisa a los productores que un slot se ha liberado 
             * y pueden intentar insertar nuevos mensajes.
             */
            buffers_available++;
            pthread_cond_signal (&buf_cond);

        /* Liberación del mutex tras la operación de E/S */
        if ((r = pthread_mutex_unlock (&buf_mutex)) != 0) {
            fprintf (stderr, "Error = %d (%s)\n", r, strerror (r)); exit (1);
        }
    }
    return NULL;
}
