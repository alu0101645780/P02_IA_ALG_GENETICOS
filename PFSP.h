#ifndef PFSP_H
#define PFSP_H

#include <vector>
#include <string>

// Clase Permutation Flow Shop Scheduling Problem, encargada de almacenar el problema, cargar datos y calcular el makespan
class PFSP {
private:
  int n;  // Número de trabajos
  int m;  // Número de maquinas
  int ub; // Mejor cota superior conocida (Upper Bound de Taillard)
  std::vector<std::vector<int>> p; // Matriz de tiempos de procesamiento: p[maquina][trabajo]

public:
  PFSP();
  
  // Lectura de instancias en formato Taillard (.fsp)
  bool cargarInstancia(const std::string& filename);
  
  int getN() const;
  int getM() const;
  int getUB() const;
  
  // Evaluación del makespan con coste O(nm) mediante la fórmula de recurrencia, tiempo total transcurrido
  // desde que entra el primer trabajo en la primera máquina hasta que sale el último trabajo de la última máquina
  int calcularMakespan(const std::vector<int>& pi) const;
  
  // Comprobacion de factibilidad (verifica que pi sea una permutacion valida)
  bool esPermutacionValida(const std::vector<int>& pi) const;
  
  // Métrica de calidad RPD (Relative Percentage Deviation) respecto a la Upper Bound
  double calcularRPD(int makespan) const;
};

#endif