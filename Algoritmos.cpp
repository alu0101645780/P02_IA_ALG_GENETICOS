#include "Algoritmos.h"
#include <iostream>
#include <numeric>
#include <algorithm>

using namespace std;

Algoritmos::Algoritmos(const PFSP& problema, int semilla) 
  : pfsp(problema), rng(semilla) {} // Se construye en base a la instancia cargada y una semilla para el generador aleatorio

// Heurística NEH
ResultadoEjecucion Algoritmos::heuristica_NEH() {
  int n = pfsp.getN(); // Obtiene el número total de trabajos de la instancia
  vector<pair<int, int>> tiempos_totales(n); // En cada par se guardará {tiempo_total, id_trabajo}
  
  // Ordenar trabajos por tiempo total de proceso decreciente
  // Recorre todos los trabajos, calcula su Makespan y guarda el par en el vector
  for (int j = 0; j < n; ++j) {
    tiempos_totales[j] = {pfsp.calcularMakespan({j}), j};
  }
  // Ordena el vector en orden inverso
  sort(tiempos_totales.rbegin(), tiempos_totales.rend());

  // Construir secuencia evaluando inserciones
  vector<int> seq;
  for (int k = 0; k < n; ++k) {
    int trabajo_actual = tiempos_totales[k].second; // Extrae el identificador del trabajo que toca procesar
    int mejor_pos = 0;
    int mejor_makespan = 1e9; // Número gigante para poder ser mejorado en la primera comparación
    
    // Prueba a insertar trabajo_actual en todas las posiciones posibles de la secuencia parcial seq
    for (int pos = 0; pos <= (int)seq.size(); ++pos) {
      // Copia la secuencia actual en seq_temp e inserta el nuevo trabajo en la posición pos
      vector<int> seq_temp = seq;
      seq_temp.insert(seq_temp.begin() + pos, trabajo_actual);
      
      // Calcula el makespan de la secuencia parcial
      int mk = pfsp.calcularMakespan(seq_temp);
      // Si el makespan es mejor actualiza el registro con ese valor y guarda su posición
      if (mk < mejor_makespan) {
        mejor_makespan = mk;
        mejor_pos = pos;
      }
    }
    // Una vez probadas todas las ubicaciones posibles para el trabajo_actual, lo instera de forma definitiva
    // en mejor pos dentro de la secuencia seq
    seq.insert(seq.begin() + mejor_pos, trabajo_actual);
  }

  // Construye el struct resultado
  ResultadoEjecucion res;
  res.mejor_makespan = pfsp.calcularMakespan(seq); // Almacena el tiempo total que tardará en procesar los trabajos en ese orden
  res.rpd = pfsp.calcularRPD(res.mejor_makespan); // Guarda el porcentaje de desviación
  return res;
}

// Búsqueda Aleatoria
ResultadoEjecucion Algoritmos::busqueda_aleatoria(int eval_max) {
  int n = pfsp.getN(); // Obtiene el número total de trabajos de la instancia
  vector<int> pi(n); // Crea el vector que albergará la secuencia
  iota(pi.begin(), pi.end(), 0); // Rellena el vector con valores consecutivos de 0 hasta n-1
  
  int mejor_makespan = 1e9; // Número gigante para poder ser mejorado en la primera comparación
  ResultadoEjecucion res;
  
  // Bucle principal que se ejecuta eval_max veces
  for (int i = 1; i <= eval_max; ++i) {
    shuffle(pi.begin(), pi.end(), rng); // Mezcla los elementos del vector pi de manera aleatoria
    int mk = pfsp.calcularMakespan(pi); 
    
    if (mk < mejor_makespan) {
      mejor_makespan = mk;
    }
    
    // Guardar puntos clave para la curva de convergencia, tomando el mejor resultado de la 
    // primera evaluación y luego cada 5.000 evaluaciones
    if (i % 5000 == 0 || i == 1) {
      res.curva_convergencia.push_back({i, mejor_makespan});
    }
  }
  
  // Asigna el mejor mp tras las pruebas y devuelve la estructura completa
  res.mejor_makespan = mejor_makespan;
  res.rpd = pfsp.calcularRPD(mejor_makespan);
  return res;
}

// Cruce OX (Order Crossover), asegura q el hijo sea una permutación válida
vector<int> Algoritmos::cruce_OX(const vector<int>& p1, const vector<int>& p2) {
  int n = pfsp.getN(); // Obtiene el número total de trabajos de la instancia
  uniform_int_distribution<int> dist(0, n - 1); // Distribución uniforme para elegir índices aleatorios
  // Genera dos puntos de corte aleatorios
  int a = dist(rng); 
  int b = dist(rng);

  if (a > b) swap(a, b); // Asegura que a sea el límite inferior y b el superior
  
  vector<int> h(n, -1); // Crea el vector hijo inicializado con -1 en todas las posiciones
  vector<bool> en_h(n, false); // Comprueba qué trabajos han sido incluidos en el vector h, evita duplicados
  
  // Copiar segmento p1[a..b] en las mismas posiciones
  for (int i = a; i <= b; ++i) {
    h[i] = p1[i];
    en_h[p1[i]] = true;
  }
  
  // Rellenar de izquierda a derecha según el orden de p2
  int idx_p2 = 0;
  for (int i = 0; i < n; ++i) {
    if (h[i] == -1) {
      while (en_h[p2[idx_p2]]) idx_p2++; // Avanza en p2 buscando el primer trabajo que aún no haya sido metido en el hijo
      h[i] = p2[idx_p2];
      en_h[p2[idx_p2]] = true;
    }
  }
  return h;
}

// Mutación por Inserción (Shift)
void Algoritmos::mutacion_shift(vector<int>& pi) {
  uniform_int_distribution<int> dist(0, pfsp.getN() - 1); // Prepara la distribución para elegir un orden válido
  int from = dist(rng); // Índice de la posición de origen
  int to = dist(rng); // Índice de la posición destino
  if (from == to) return;
  
  int trabajo = pi[from];
  pi.erase(pi.begin() + from); // Al eliminar el elemnto, todos los elementos a su derecha se despazan a la izquierda
  pi.insert(pi.begin() + to, trabajo); // Al insertar otro, todos los elemntos se desplazan a la derecha
}

// Mutación por Intercambio (Swap)
void Algoritmos::mutacion_swap(vector<int>& pi) {
  uniform_int_distribution<int> dist(0, pfsp.getN() - 1);
  int p1 = dist(rng);
  int p2 = dist(rng);
  swap(pi[p1], pi[p2]);
}

// Algoritmo Genético completo
ResultadoEjecucion Algoritmos::algoritmo_genetico(int N, float pc, float pm, int k_torneo, int E) {
  int n = pfsp.getN();
  vector<vector<int>> poblacion(N, vector<int>(n)); // Matriz que guarda los N individuos
  vector<int> fitness(N);
  int evaluaciones = 0;
  
  ResultadoEjecucion res;

  // Población inicial: 1 individuo generado con NEH + (N-1) aleatorios
  ResultadoEjecucion res_neh_init = heuristica_NEH();
  
  // Generar la secuencia NEH para el primer individuo
  // Suma los tiempos de cada trabajo en las máquinas y los ordena de mayor a menor (los trabajos más pesados primero)
  vector<pair<int, int>> t_tot(n);
  for (int j = 0; j < n; ++j) {
    t_tot[j] = {pfsp.calcularMakespan({j}), j};
  }
  sort(t_tot.rbegin(), t_tot.rend()); // Ordena los trabajos de mayor a menor carga
  
  // Aplica NEH, toma cada trabajo del ranking anterior y prueba a insertarlo en todas las posiciones posibles 
  // de la secuencia parcial, eligiendo siempre la posición que minimice el makespan parcial
  vector<int> seq_neh;
  for (int k = 0; k < n; ++k) {
    int trabajo = t_tot[k].second, m_pos = 0, m_mk = 1e9;
    for (int pos = 0; pos <= (int)seq_neh.size(); ++pos) {
      vector<int> tmp = seq_neh;
      tmp.insert(tmp.begin() + pos, trabajo);
      int mk = pfsp.calcularMakespan(tmp);
      if (mk < m_mk) { 
        m_mk = mk; 
        m_pos = pos; 
      }
    }
    seq_neh.insert(seq_neh.begin() + m_pos, trabajo);
  }
  
  poblacion[0] = seq_neh; // Asigna la secuencia ordenada por NEH al primer miembro de la población
  fitness[0] = pfsp.calcularMakespan(poblacion[0]); // Calcula el makespan de esa secuencia NEH y lo guarda en fitness[0]
  evaluaciones++;
  
  // Crea el resto de la población aleatoria y las valida antes de cruzarlas entre sí
  for (int i = 1; i < N; ++i) {
    iota(poblacion[i].begin(), poblacion[i].end(), 0);
    shuffle(poblacion[i].begin(), poblacion[i].end(), rng);
    fitness[i] = pfsp.calcularMakespan(poblacion[i]);
    evaluaciones++;
  }
  
  int mejor_global = fitness[0]; // Guarda el récord
  int idx_mejor = 0; // El mejor individuo está en la posición 0
  
  uniform_real_distribution<float> prob(0.0, 1.0);
  uniform_int_distribution<int> dist_pop(0, N - 1);
  
  // Ciclo Evolutivo
  while (evaluaciones < E) {
    vector<vector<int>> nueva_poblacion;
    
    while ((int)nueva_poblacion.size() < N && evaluaciones < E) {
      // Selección por Torneo
      int p1_idx = dist_pop(rng), p2_idx = dist_pop(rng);
      for (int j = 1; j < k_torneo; ++j) {
        int c = dist_pop(rng);
        if (fitness[c] < fitness[p1_idx]) p1_idx = c;
      }
      for (int j = 1; j < k_torneo; ++j) {
        int c = dist_pop(rng);
        if (fitness[c] < fitness[p2_idx]) p2_idx = c;
      }
      
      vector<int> h1 = poblacion[p1_idx];
      vector<int> h2 = poblacion[p2_idx];
      
      // Cruce OX
      if (prob(rng) < pc) {
        h1 = cruce_OX(poblacion[p1_idx], poblacion[p2_idx]);
        h2 = cruce_OX(poblacion[p2_idx], poblacion[p1_idx]);
      }
      
      // Mutación Shift
      if (prob(rng) < pm) mutacion_shift(h1);
      if (prob(rng) < pm) mutacion_shift(h2);
      
      nueva_poblacion.push_back(h1);
      if ((int)nueva_poblacion.size() < N) nueva_poblacion.push_back(h2);
    }
    
    // Reemplazo generacional con elitismo
    int peor_hijo_idx = 0;
    int peor_hijo_fit = 0;
    vector<int> fitness_nuevos(N);
    
    for (int i = 0; i < N && evaluaciones < E; ++i) {
      fitness_nuevos[i] = pfsp.calcularMakespan(nueva_poblacion[i]);
      evaluaciones++;
      
      if (fitness_nuevos[i] > peor_hijo_fit) {
        peor_hijo_fit = fitness_nuevos[i];
        peor_hijo_idx = i;
      }
    }
    
    // Elitismo: El mejor padre sustituye al peor hijo si lo supera
    if (mejor_global < peor_hijo_fit) {
      nueva_poblacion[peor_hijo_idx] = poblacion[idx_mejor];
      fitness_nuevos[peor_hijo_idx] = mejor_global;
    }
    
    poblacion = nueva_poblacion;
    fitness = fitness_nuevos;
    
    for (int i = 0; i < N; ++i) {
      if (fitness[i] < mejor_global) {
        mejor_global = fitness[i];
        idx_mejor = i;
      }
    }
    
    // Registrar datos de la curva de convergencia
    if (evaluaciones % 5000 == 0 || evaluaciones == E) {
      res.curva_convergencia.push_back({evaluaciones, mejor_global});
    }
  }

  res.mejor_makespan = mejor_global;
  res.rpd = pfsp.calcularRPD(mejor_global);
  return res;
}