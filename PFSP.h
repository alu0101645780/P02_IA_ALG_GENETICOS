#ifndef PFSP_H
#define PFSP_H

#include <vector>
#include <string>

// Clase encargada de almacenar el problema, cargar datos y calcular el makespan
class PFSP {
private:
  int n;  // Numero de trabajos
  int m;  // Numero de maquinas
  int ub; // Mejor cota superior conocida (Upper Bound de Taillard)
  std::vector<std::vector<int>> p; // Matriz de tiempos de procesamiento: p[maquina][trabajo]

public:
  PFSP();
  
  // T1: Lectura de instancias en formato Taillard (.fsp)
  bool cargarInstancia(const std::string& filename);
  
  int getN() const;
  int getM() const;
  int getUB() const;
  
  // T1: Evaluacion del makespan con coste O(nm) mediante la formula de recurrencia
  int calcularMakespan(const std::vector<int>& pi) const;
  
  // T4: Comprobacion de factibilidad (verifica que pi sea una permutacion valida)
  bool esPermutacionValida(const std::vector<int>& pi) const;
  
  // Métrica de calidad RPD (Relative Percentage Deviation) respecto a la Upper Bound
  double calcularRPD(int makespan) const;
};

#endif