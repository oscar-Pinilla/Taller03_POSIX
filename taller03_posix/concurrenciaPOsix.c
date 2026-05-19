 /*********************************************************************************************
 * Pontificia Universidad Javeriana
 *
 * Materia Sistemas Operativos
 * Docente: J. Corredor, PhD
 * Fecha: 27/10/2025
 * Tema: Posix para la creación de hilos concurrentes:
 * Autores : David Pedraza y Oscar Pinilla
 * Descripción:
 * Este programa realiza la búsqueda del valor máximo en un vector cargado desde un fichero.
 * La implementación utiliza hilos POSIX para dividir la carga de trabajo de forma 
 * concurrente. El programa lee el tamaño del vector y sus elementos de un archivo 
 * de entrada y emplea una estructura para pasar argumentos a los hilos, incluyendo el rango 
 * de búsqueda y el almacenamiento del máximo parcial
 *
 * 
 *********************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Estructura para pasar múltiples argumentos a cada hilo */
struct argHilos {
    int inicio;     /* Índice de inicio en el vector para el hilo */
    int fin;        /* Índice de fin en el vector para el hilo */
    int *vector;    /* Puntero al vector de datos original */
    int maxparcial; /* Resultado local encontrado por este hilo */
};

typedef struct argHilos param_H;

/* Prototipo de la función que gestiona los hilos */
int maximoValor(int *vec, int n, int nhilos);

/* Función que ejecuta cada hilo para buscar el máximo en su rango asignado */
void *buscarMax(void *parametro) {
    param_H *argumentos = (param_H *)parametro;
    
    /* Inicializamos el máximo parcial con el primer elemento del rango */
    argumentos->maxparcial = argumentos->vector[argumentos->inicio];
    
    /* Recorremos el subconjunto del vector asignado al hilo */
    for(int i = argumentos->inicio; i < argumentos->fin; i++) {
        if(argumentos->vector[i] > argumentos->maxparcial) {
            argumentos->maxparcial = argumentos->vector[i];
        }
    }
    
    pthread_exit(0);
    return NULL;
}

int main(int argc, char *argv[]) {
    FILE *fichero;
    int n, nhilos, i;
    int *vec;
    int ret, maximo;

    /* Validación de argumentos de línea de comandos */
    if (argc != 3) {
        fprintf(stderr, "Error en número de argumentos \n");
        exit(-1);
    }

    /* Apertura del archivo de datos en modo lectura */
    fichero = fopen(argv[1], "r");
    if (fichero == NULL) {
        perror("No se puede abrir fichero");
        exit(-2);
    }

    /* Lectura del tamaño total del vector (primer dato del archivo) */
    ret = fscanf(fichero, "%d", &n);
    if (ret != 1) {
        fprintf(stderr, "No se puede leer tamaño\n");
        exit(-3);
    }

    /* Número de hilos a crear según el usuario */
    nhilos = atoi(argv[2]);
    
    /* Reserva de memoria dinámica para el vector */
    vec = malloc(sizeof(int) * n);
    
    /* Lectura de los n elementos del archivo */
    for (i = 0; i < n; ++i) {
        ret = fscanf(fichero, "%d", &vec[i]);
        if (ret != 1) {
            fprintf(stderr, "No se puede leer elemento nro %d\n", i);
            fclose(fichero);
            free(vec);
            exit(-1);
        }
    }

    /* Llamada a la función lógica que orquestará los hilos */
    maximo = maximoValor(vec, n, nhilos);
    
    printf("Máximo: %d\n", maximo);

    /* Limpieza de recursos del proceso principal */
    fclose(fichero);
    free(vec);
    
    return 0;
}

/* Función que crea los hilos y consolida los resultados parciales */
int maximoValor(int *vec, int n, int nhilos) {
    pthread_t hilos[nhilos];
    param_H args[nhilos];
    int tamano = n / nhilos; /* División de la carga de trabajo */

    for (int i = 0; i < nhilos; i++) {
        args[i].inicio = i * tamano;
        /* El último hilo toma los elementos sobrantes para cubrir el vector completo */
        args[i].fin = (i == nhilos - 1) ? n : (i + 1) * tamano;
        args[i].vector = vec;
        args[i].maxparcial = 0;

        /* Creación del hilo y paso de su estructura de parámetros */
        pthread_create(&hilos[i], NULL, buscarMax, &args[i]);
    }

    /* Inicializamos el máximo global con el primer resultado parcial disponible */
    int maximo = args[0].maxparcial;

    for (int i = 0; i < nhilos; i++) {
        /* Espera a que el hilo termine su ejecución */
        pthread_join(hilos[i], NULL);
        
        /* Comparamos los máximos parciales de cada hilo para hallar el máximo total */
        if (args[i].maxparcial > maximo) {
            maximo = args[i].maxparcial;
        }
    }

    return maximo;
}
