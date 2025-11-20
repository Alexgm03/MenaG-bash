# Intérprete Interactivo de Comandos — Mini Shell en C

Este proyecto es un intérprete interactivo de comandos similar a **bash**, desarrollado para Linux utilizando exclusivamente **llamadas al sistema (system calls)** y funciones básicas como `fork()`, `exec()`, `waitpid()` y manejo de señales.

El objetivo es comprender el funcionamiento interno de un shell, la gestión de procesos, la redirección de E/S y la ejecución de comandos tanto internos como externos.

---

## Funcionalidades Implementadas

### Comandos internos:
- `cd` — Cambia el directorio actual
- `ls` — Lista archivos en el directorio
- `pwd` — Muestra la ruta actual
- `mkdir` — Crea un directorio
- `rm` — Elimina archivos o carpetas
- `cp` — Copia archivos
- `mv` — Mueve o renombra archivos o directorios
- `cat` — Muestra el contenido de un archivo

### Ejecución de programas externos
Permite ejecutar comandos como: gcc, vim, nano, etc.

usando `fork()` + `execvp()`.

### Ejecución en segundo plano
Permite ejecutar comandos con `&` al final, sin bloquear el shell.

### Redirección de entrada y salida
- `>`
- `>>`
- `<`

### Manejo de procesos zombi
Uso de señales (`SIGCHLD`) para limpiar procesos hijos finalizados.

---

## Compilación

Este proyecto incluye un **Makefile**, por lo que compilar es tan simple como:

```bash
make

