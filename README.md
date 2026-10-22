# Práctica 2: Algoritmo Genético para el Permutation Flow Shop Scheduling Problem (PFSP)

**Asignatura:** Inteligencia Artificial  
**Titulación:** Grado en Ingeniería Informática  

---

## 1. Estructura del Proyecto

- `PFSP.h` / `PFSP.cpp`: Clase encargada del modelado del problema, carga de instancias en formato Taillard (.fsp), cálculo de la función de evaluación (*makespan*) con coste $O(nm)$, verificación de factibilidad de soluciones y cálculo del RPD.
- `Algoritmos.h` / `Algoritmos.cpp`: Implementación de los algoritmos requeridos:
  * Heurística constructiva NEH.
  * Búsqueda Aleatoria.
  * Algoritmo Genético Generacional con Elitismo, selección por torneo binario, cruce de orden (OX) y mutación por inserción (*shift*).
- `main.cpp`: Gestión de parámetros de entrada, invocación de los algoritmos e impresión formateada de las tablas de resultados y curvas de convergencia.
- `Makefile`: Fichero de compilación del proyecto.

---

## 2. Instrucciones de Compilación y Ejecución

### Compilación
Para compilar el programa basta con ejecutar el siguiente comando en la terminal:

```bash
make
```

### Ejecución
Ejecutar el programa con la instancia por defecto (instances/tai20_5_5.fsp):

```bash
./pfsp_ga
```

Para ejecutar especificando una instancia concreta:

```bash
./pfsp_ga instances/ejemplo_pdf.fsp
```

Para ejecutar modificando todos los parámetros (Instancia, Población, Prob. Cruce, Prob. Mutación, Tamaño Torneo, Semilla, Evaluaciones):

```bash
./pfsp_ga instances/ejemplo.fsp 100 0.8 0.05 2 1234 50000
```

Para obtener la salida en una sola línea con formato CSV (o formato tabla):

```bash
./pfsp_ga instances/ejemplo.fsp 50 0.8 0.1 2 1234 50000 --tabla
```
### Visualización
Para la visualización de las gráficas copiar y pegar las tablas resultado (convergencia.txt) en un excel u hoja de cálculo e instar una gráfica.