#ifndef ALGORITMOS_H
#define ALGORITMOS_H

#include "PFSP.h"
#include <vector>
#include <random>

// Estructura para registrar los datos de convergencia durante la ejecución
struct PuntoConvergencia {
  int evaluacion;
  int mejor_makespan;
};

// Estructura para agrupar los resultados finales de un algoritmo
struct ResultadoEjecucion {
  int mejor_makespan;
  double rpd;
  std::vector<PuntoConvergencia> curva_convergencia;
};

class Algoritmos {
private:
  const PFSP& pfsp;
  std::mt19937 rng;

  // Operadores de Cruce y Mutacion cerrados sobre permutaciones
  std::vector<int> cruce_OX(const std::vector<int>& p1, const std::vector<int>& p2);
  void mutacion_shift(std::vector<int>& pi);
  void mutacion_swap(std::vector<int>& pi);

public:
  Algoritmos(const PFSP& problema, int semilla);

  // T2: Algoritmos de referencia
  ResultadoEjecucion heuristica_NEH();
  ResultadoEjecucion busqueda_aleatoria(int eval_max);

  // T3: Algoritmo Genético Generacional con Elitismo
  ResultadoEjecucion algoritmo_genetico(int N, float pc, float pm, int k_torneo, int eval_max);
};

#endif