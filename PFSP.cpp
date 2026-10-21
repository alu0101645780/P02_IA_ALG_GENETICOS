#include "PFSP.h"
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace std;

PFSP::PFSP() : n(0), m(0), ub(0) {}

bool PFSP::cargarInstancia(const string& filename) {
  ifstream file(filename);
  if (!file.is_open()) return false;
  
  string token;
  // Saltar cabecera hasta el primer ":"
  while (file >> token) {
    if (token == ":") break;
  }
  
  // Leer dimensiones y cotas
  int semilla, lb;
  if (!(file >> n >> m >> semilla >> ub >> lb)) {
    return false;
  }
  
  // Saltar encabezado de la matriz hasta el segundo ":"
  while (file >> token) {
    if (token == ":") break;
  }
  
  // Creamos un vector p de m filas con el contenido de cada fila (vacío inicialmente)
  p.assign(m, vector<int>(n));
  
  // Cargar tiempos de procesamiento de cada máquina para cada trabajo
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      file >> p[i][j];
    }
  }
  
  file.close();
  return (n > 0 && m > 0);
}

int PFSP::getN() const { return n; }
int PFSP::getM() const { return m; }
int PFSP::getUB() const { return ub; }

// Cálculo del makespan
int PFSP::calcularMakespan(const vector<int>& pi) const {
  if (pi.empty()) return 0;
  
  int num_trabajos = pi.size();

  // Crea y reserva espacio para una matriz bidimensional dinámica llamada C 
  // de tamaño num_trabajos x m, e inicializa todos sus valores a 0
  vector<vector<int>> C(num_trabajos, vector<int>(m, 0));
  
  // Calcula cómo pasan todos los trabajos por la primera máquina
  C[0][0] = p[0][pi[0]];
  
  for (int k = 1; k < num_trabajos; ++k) {
    C[k][0] = C[k-1][0] + p[0][pi[k]];
  }
  
  // Calcula el primer trabajo para todas las máquinas
  for (int i = 1; i < m; ++i) {
    C[0][i] = C[0][i-1] + p[i][pi[0]];
  }
    
  // Recurrencia del Makespan: C(k,i) = max{ C(k-1,i), C(k,i-1) } + p_i,pi_k
  for (int k = 1; k < num_trabajos; ++k) {
    for (int i = 1; i < m; ++i) {
      // Momento en el que el trabajo k termina de procesarse por completo en la máquina i, tiene que cumplir dos condiciones:
      // Que la máquina esté libre: C[k-1][i]
      // Que el trabajo haya llegado: C[k][i-1]
      // Se queda con el mayor de los tiempos y suma su propia duración
      C[k][i] = max(C[k-1][i], C[k][i-1]) + p[i][pi[k]];
    }
  }

  // Devuelve la última casilla de la matriz (resultado final acumulado) 
  return C[num_trabajos-1][m-1];
}

// Verificación de factibilidad
bool PFSP::esPermutacionValida(const vector<int>& pi) const {
  if ((int)pi.size() != n) return false;
  vector<bool> visto(n, false);
  for (int trabajo : pi) {
    // Compruba si el número de trabajos es negativo, si supera el número de trabajos total
    // o si el trabajo ya ha sido comprobado (que hayan repetidos)
    if (trabajo < 0 || trabajo >= n || visto[trabajo]) return false;
    visto[trabajo] = true;
  }
  return true;
}

// Fórmula del RPD: 100 * (Makespan - UB) / UB
double PFSP::calcularRPD(int makespan) const {
  if (ub <= 0) return 0.0;
  return 100.0 * (makespan - ub) / static_cast<double>(ub);
}