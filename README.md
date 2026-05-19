# Taller03_POSIX
# Taller 03 — Sincronización POSIX

**Asignatura:** Sistemas Operativos  
**Institución:** Pontificia Universidad Javeriana — Facultad de Ingeniería de Sistemas  
**Docente:** J. Corredor, PhD  
**Autor:** Oscar Samuel Pinilla Alvira
**Fecha:** 2026

---

## Descripción general

Este taller explora los mecanismos de sincronización que ofrece la interfaz POSIX para programación concurrente en C. Se implementan dos grandes problemas clásicos de los sistemas operativos:

1. **Búsqueda concurrente del máximo** en un vector mediante hilos Pthreads.
2. **Modelo Productor–Consumidor** en dos variantes: con memoria compartida entre procesos (IPC) y con hilos sincronizados dentro de un mismo proceso.

---

## Estructura del repositorio

```
taller03_posix/
├── concurrenciaPOsix.c   # Búsqueda del máximo con Pthreads (versión base)
├── concurrenciaPOsix.txt # Versión alternativa / notas del ejercicio
├── posixSincro.c         # Productor-Consumidor con Pthreads (buffer circular + mutex + cond vars)
├── producer.c            # Proceso Productor (IPC: shared memory + semáforos POSIX)
├── consumer.c            # Proceso Consumidor (IPC: shared memory + semáforos POSIX)
├── numero.txt            # Archivo de entrada para el ejercicio de concurrencia
└── Makefile              # Automatiza la compilación de todos los módulos
```

---

## Módulos

### 1. `concurrenciaPOsix.c` — Búsqueda del máximo con hilos

Divide un vector de enteros leído desde archivo entre `N` hilos POSIX. Cada hilo busca el máximo en su rango asignado (*máximo parcial*) y el proceso principal consolida los resultados al hacer `pthread_join`.

**Conceptos aplicados:**
- Creación y unión de hilos con `pthread_create` / `pthread_join`
- Paso de argumentos mediante estructura `param_H`
- División de carga de trabajo y manejo del último segmento (elementos sobrantes)

**Uso:**
```bash
./concurrenciaPOsix <archivo_datos> <número_de_hilos>
# Ejemplo:
./concurrenciaPOsix numero.txt 4
```

---

### 2. `posixSincro.c` — Productor-Consumidor con Pthreads

Implementa el modelo Productor–Consumidor usando un **buffer circular compartido** dentro de un mismo proceso. 10 hilos productores insertan mensajes y un hilo *spooler* (consumidor) los extrae e imprime.

**Conceptos aplicados:**
- Mutex (`pthread_mutex_t`) para exclusión mutua en la sección crítica
- Variables de condición (`pthread_cond_t`) para sincronización pasiva sin espera activa
- Manejo de despertares espurios con `while` en lugar de `if`
- Buffer circular con contadores `buffers_available` y `lines_to_print`
- Terminación controlada con `pthread_cancel`

**Uso:**
```bash
./posixSincro
```

---

### 3. `producer.c` / `consumer.c` — IPC con Memoria Compartida y Semáforos

Implementación del Productor–Consumidor entre **dos procesos independientes** usando mecanismos IPC de POSIX:

- **`shm_open` / `mmap`:** objeto de memoria compartida donde reside el buffer circular.
- **`sem_open`:** dos semáforos con nombre (`/vacio` y `/lleno`) que controlan el flujo y evitan condiciones de carrera.

El Productor crea los recursos, genera 10 ítems y los deposita. El Consumidor accede al mismo segmento, extrae y procesa los ítems, y realiza la limpieza final de los recursos del sistema.

**Uso (en terminales separadas):**
```bash
# Terminal 1 — iniciar el productor primero
./producer

# Terminal 2 — iniciar el consumidor
./consumer
```

---

## Compilación

El proyecto incluye un `Makefile` que compila todos los ejecutables con soporte para `pthreads`:

```bash
# Compilar todos los módulos
make all

# Compilar un módulo específico
make producer
make consumer
make posixSincro
make concurrenciaPOsix

# Limpiar ejecutables generados
make clean
```

**Requisitos:** `gcc` con soporte POSIX (Linux). La flag `-lpthread` es añadida automáticamente por el Makefile.

---

## Conceptos clave

| Concepto | Usado en |
|---|---|
| `pthread_create` / `pthread_join` | `concurrenciaPOsix.c`, `posixSincro.c` |
| `pthread_mutex_t` | `posixSincro.c` |
| `pthread_cond_t` | `posixSincro.c` |
| `sem_open` / `sem_wait` / `sem_post` | `producer.c`, `consumer.c` |
| `shm_open` / `mmap` | `producer.c`, `consumer.c` |
| Buffer circular | `posixSincro.c`, `producer.c` / `consumer.c` |

---

## Autores

| Nombre | Rol |
|---|---|
| Oscar Samuel Pinilla Alvira| Desarrollo e implementación |
