#include "Algoritmos.h"
#include <iostream>
#include <numeric>
#include <algorithm>

using namespace std;

Algoritmos::Algoritmos(const PFSP& problema, int semilla) 
  : pfsp(problema), rng(semilla) {}

// T2: Heurística NEH
ResultadoEjecucion Algoritmos::heuristica_NEH() {
  int n = pfsp.getN();
  vector<pair<int, int>> tiempos_totales(n);
  
  // 1. Ordenar trabajos por tiempo total de proceso decreciente
  for (int j = 0; j < n; ++j) {
    tiempos_totales[j] = {pfsp.calcularMakespan({j}), j};
  }
  sort(tiempos_totales.rbegin(), tiempos_totales.rend());

  // 2. Construir secuencia evaluando inserciones
  vector<int> seq;
  for (int k = 0; k < n; ++k) {
    int trabajo_actual = tiempos_totales[k].second;
    int mejor_pos = 0;
    int mejor_makespan = 1e9;
    
    for (int pos = 0; pos <= (int)seq.size(); ++pos) {
      vector<int> seq_temp = seq;
      seq_temp.insert(seq_temp.begin() + pos, trabajo_actual);
      
      int mk = pfsp.calcularMakespan(seq_temp);
      if (mk < mejor_makespan) {
        mejor_makespan = mk;
        mejor_pos = pos;
      }
    }
    seq.insert(seq.begin() + mejor_pos, trabajo_actual);
  }

  ResultadoEjecucion res;
  res.mejor_makespan = pfsp.calcularMakespan(seq);
  res.rpd = pfsp.calcularRPD(res.mejor_makespan);
  return res;
}

// T2: Búsqueda Aleatoria
ResultadoEjecucion Algoritmos::busqueda_aleatoria(int eval_max) {
  int n = pfsp.getN();
  vector<int> pi(n);
  iota(pi.begin(), pi.end(), 0);
  
  int mejor_makespan = 1e9;
  ResultadoEjecucion res;
  
  for (int i = 1; i <= eval_max; ++i) {
    shuffle(pi.begin(), pi.end(), rng);
    int mk = pfsp.calcularMakespan(pi);
    
    if (mk < mejor_makespan) {
      mejor_makespan = mk;
    }
    
    // Guardar puntos clave para la curva de convergencia
    if (i % 5000 == 0 || i == 1) {
      res.curva_convergencia.push_back({i, mejor_makespan});
    }
  }
  
  res.mejor_makespan = mejor_makespan;
  res.rpd = pfsp.calcularRPD(mejor_makespan);
  return res;
}

// T3: Cruce OX (Order Crossover)
vector<int> Algoritmos::cruce_OX(const vector<int>& p1, const vector<int>& p2) {
  int n = pfsp.getN();
  uniform_int_distribution<int> dist(0, n - 1);
  int a = dist(rng);
  int b = dist(rng);
  if (a > b) swap(a, b);
  
  vector<int> h(n, -1);
  vector<bool> en_h(n, false);
  
  // Copiar segmento p1[a..b] en las mismas posiciones
  for (int i = a; i <= b; ++i) {
    h[i] = p1[i];
    en_h[p1[i]] = true;
  }
  
  // Rellenar de izquierda a derecha según el orden de p2
  int idx_p2 = 0;
  for (int i = 0; i < n; ++i) {
    if (h[i] == -1) {
      while (en_h[p2[idx_p2]]) idx_p2++;
      h[i] = p2[idx_p2];
      en_h[p2[idx_p2]] = true;
    }
  }
  return h;
}

// T3: Mutación por Inserción (Shift)
void Algoritmos::mutacion_shift(vector<int>& pi) {
  uniform_int_distribution<int> dist(0, pfsp.getN() - 1);
  int from = dist(rng);
  int to = dist(rng);
  if (from == to) return;
  
  int trabajo = pi[from];
  pi.erase(pi.begin() + from);
  pi.insert(pi.begin() + to, trabajo);
}

// T3: Mutación por Intercambio (Swap)
void Algoritmos::mutacion_swap(vector<int>& pi) {
  uniform_int_distribution<int> dist(0, pfsp.getN() - 1);
  int p1 = dist(rng);
  int p2 = dist(rng);
  swap(pi[p1], pi[p2]);
}

// T3: Algoritmo Genético completo
ResultadoEjecucion Algoritmos::algoritmo_genetico(int N, float pc, float pm, int k_torneo, int E) {
  int n = pfsp.getN();
  vector<vector<int>> poblacion(N, vector<int>(n));
  vector<int> fitness(N);
  int evaluaciones = 0;
  
  ResultadoEjecucion res;

  // 1. Población inicial: 1 individuo generado con NEH + (N-1) aleatorios
  ResultadoEjecucion res_neh_init = heuristica_NEH();
  
  // Generar la secuencia NEH para el primer individuo
  vector<pair<int, int>> t_tot(n);
  for (int j = 0; j < n; ++j) {
    t_tot[j] = {pfsp.calcularMakespan({j}), j};
  }
  sort(t_tot.rbegin(), t_tot.rend());
  
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
  
  poblacion[0] = seq_neh;
  fitness[0] = pfsp.calcularMakespan(poblacion[0]);
  evaluaciones++;
  
  // Resto de la población aleatoria
  for (int i = 1; i < N; ++i) {
    iota(poblacion[i].begin(), poblacion[i].end(), 0);
    shuffle(poblacion[i].begin(), poblacion[i].end(), rng);
    fitness[i] = pfsp.calcularMakespan(poblacion[i]);
    evaluaciones++;
  }
  
  int mejor_global = fitness[0];
  int idx_mejor = 0;
  
  uniform_real_distribution<float> prob(0.0, 1.0);
  uniform_int_distribution<int> dist_pop(0, N - 1);
  
  // 2. Ciclo Evolutivo
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