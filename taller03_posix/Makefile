################################################################################
#		Pontificia Universidad Javeriana
#	Autor: Oscar Pinillla y david Pedraza
#	Fecha: 24/03/2026
#	Taller: Taller 03 Posix Semaforos
#	Actividad: Sincronizacion Semaforos-Posix
#	Descripcion: Este Makefile automatiza la compilación de múltiples programas en C
# 	relacionados con la sincronización POSIX. Gestiona las banderas de enlace para
# 	la biblioteca pthreads y facilita la limpieza del entorno de trabajo.
################################################################################

# Definición de variables de entorno y compilador
GCC=gcc

BANDERAS= -lpthread
PROGRAMAS= producer consumer posixSincro concurrenciaPOsix

all = ${PROGRAMAS}

# Compilación del módulo Productor: utiliza pthreads para manejo de hilos
producer:
	$(GCC) producer.c -o producer $(BANDERAS)

# Compilación del módulo Consumidor: sincronizado con el productor mediante semáforos
consumer:
	$(GCC) consumer.c -o consumer $(BANDERAS)

# Módulo de Sincronización POSIX: implementación de buffer circular y condiciones
posixSincro:
	$(GCC) posixSincro.c -o posixSincro $(BANDERAS)

# Módulo de Concurrencia: búsqueda de máximo valor utilizando división de carga por hilos
concurrenciaPOsix:
	$(GCC) concurrenciaPOsix.c -o concurrenciaPOsix $(BANDERAS)

clean:
	$(RM) ${PROGRAMAS}
