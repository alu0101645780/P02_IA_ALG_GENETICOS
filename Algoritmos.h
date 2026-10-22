#ifndef ALGORITMOS_H
#define ALGORITMOS_H

#include "PFSP.h"
#include <vector>
#include <random>

// Estructura para registrar los datos de convergencia durante la ejecución
struct PuntoConvergencia {
  int evaluacion; // Número de evaluación en la que nos encontramos 
  int mejor_makespan; // El mejor Makespan encontrado hasta ese momento
};

// Estructura para agrupar los resultados finales de un algoritmo, mejor makespam, secuencia y la curva de convergencia
struct ResultadoEjecucion {
  int mejor_makespan;
  double rpd;
  std::vector<PuntoConvergencia> curva_convergencia;
};

// La clase Algoritmos contine toda la lógica de resolución del problema
class Algoritmos {
private:
  const PFSP& pfsp;
  std::mt19937 rng; // Generador de números aleatorios

  // Operadores de Cruce y Mutacion cerrados sobre permutaciones
  std::vector<int> cruce_OX(const std::vector<int>& p1, const std::vector<int>& p2); // Recibe dos padres y devuelve un hijo válido
  void mutacion_shift(std::vector<int>& pi); // Mutación por inserción, mueve un trabajo de una posición a otra en pi
  void mutacion_swap(std::vector<int>& pi); // Mutación por intercambio, intercambia las posiciones de dos trabajos en pi

public:
  Algoritmos(const PFSP& problema, int semilla);

  // Algoritmos de referencia
  ResultadoEjecucion heuristica_NEH();
  ResultadoEjecucion busqueda_aleatoria(int eval_max); // eval_max es el límite de evaluaciones

  // Algoritmo Genético Generacional con Elitismo, recibe el tamaño de población, probabilidad de cruce,
  // probabilidad de mutación, tamaño del torneo y límite de evaluaciones
  ResultadoEjecucion algoritmo_genetico(int N, float pc, float pm, int k_torneo, int eval_max);
};

#endif