#include "PFSP.h"
#include "Algoritmos.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <chrono>
#include <cmath>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
  // Parámetros por defecto
  string file_path = "instances/tai20_5_5.fsp";
  int N = 50;
  float pc = 0.8f;
  float pm = 0.1f;
  int k_torneo = 2;
  int semilla_base = 42;
  int eval_max = 50000;

  // Configuración por consola
  if (argc > 1) file_path = argv[1];
  if (argc > 2) N = stoi(argv[2]);
  if (argc > 3) pc = stof(argv[3]);
  if (argc > 4) pm = stof(argv[4]);
  if (argc > 5) k_torneo = stoi(argv[5]);
  if (argc > 6) semilla_base = stoi(argv[6]);
  if (argc > 7) eval_max = stoi(argv[7]);

  PFSP problema;
  if (!problema.cargarInstancia(file_path)) {
    cerr << "Error al abrir el fichero de instancia: " << file_path << "\n";
    return 1;
  }

  Algoritmos solver_base(problema, semilla_base);

  // 1. Ejecutar NEH y Búsqueda Aleatoria
  ResultadoEjecucion res_neh = solver_base.heuristica_NEH();
  ResultadoEjecucion res_ba = solver_base.busqueda_aleatoria(eval_max);

  // 2. Realizar las 10 repeticiones del AG
  int repeticiones = 10;
  int mejor_cmax_ag = 1e9;
  double suma_cmax = 0.0;
  double suma_tiempos = 0.0;
  vector<int> historico_cmax(repeticiones);
  vector<PuntoConvergencia> mejor_curva;

  for (int i = 0; i < repeticiones; ++i) {
    Algoritmos solver_ag(problema, semilla_base + i);
    
    auto start_time = chrono::high_resolution_clock::now();
    ResultadoEjecucion res_ag = solver_ag.algoritmo_genetico(N, pc, pm, k_torneo, eval_max);
    auto end_time = chrono::high_resolution_clock::now();
    
    chrono::duration<double> diff = end_time - start_time;
    
    historico_cmax[i] = res_ag.mejor_makespan;
    suma_cmax += res_ag.mejor_makespan;
    suma_tiempos += diff.count();
    
    if (res_ag.mejor_makespan < mejor_cmax_ag) {
      mejor_cmax_ag = res_ag.mejor_makespan;
      mejor_curva = res_ag.curva_convergencia;
    }
  }

  // 3. Cálculos estadísticos
  double promedio_cmax = suma_cmax / repeticiones;
  double promedio_tiempo = suma_tiempos / repeticiones;
  
  double suma_varianza = 0.0;
  for (int val : historico_cmax) {
    suma_varianza += pow(val - promedio_cmax, 2);
  }
  double std_dev = sqrt(suma_varianza / repeticiones);

  double rpd_ub = problema.calcularRPD(mejor_cmax_ag); 
  double rpd_neh = 100.0 * (mejor_cmax_ag - res_neh.mejor_makespan) / static_cast<double>(res_neh.mejor_makespan);

  // ---------------------------------------------------------
  // 4. IMPRIMIR TABLA ORIGINAL (La que tenías bien)
  // ---------------------------------------------------------
  cout << fixed << setprecision(2);
  cout << "======================================================================\n";
  cout << "Instancia: " << file_path << " (" << problema.getN() << " trabajos x " << problema.getM() << " maquinas)\n";
  cout << "Upper Bound (UB): " << problema.getUB() << "\n";
  cout << "======================================================================\n";
  cout << left << setw(22) << "Algoritmo" << "| Makespan | RPD (%)\n";
  cout << "----------------------------------------------------------------------\n";
  cout << left << setw(22) << "NEH Heuristica" << "| " << right << setw(8) << res_neh.mejor_makespan << " | " << problema.calcularRPD(res_neh.mejor_makespan) << "%\n";
  cout << left << setw(22) << "Busqueda Aleatoria" << "| " << right << setw(8) << res_ba.mejor_makespan << " | " << problema.calcularRPD(res_ba.mejor_makespan) << "%\n";
  cout << left << setw(22) << "Algoritmo Genetico" << "| " << right << setw(8) << mejor_cmax_ag << " | " << rpd_ub << "%\n";
  cout << "======================================================================\n";

  // ---------------------------------------------------------
  // 5. GUARDAR EN ARCHIVOS .TXT
  // ---------------------------------------------------------
  string inst_name = file_path;
  size_t pos = inst_name.find_last_of("/\\");
  if(pos != string::npos) inst_name = inst_name.substr(pos+1);

  ofstream out_tabla("tabla_resultados.txt", ios::app);
  out_tabla.seekp(0, ios::end);
  if (out_tabla.tellp() == 0) { 
    out_tabla << "Inst\tn\tm\tMejor Cmax\tPromedio Cmax(10rep)\tStd Dev\tNEH\tPromedio Tiempo(s)\tRPD UB(%)\tRPD NEH(%)\n";
  }
  out_tabla << inst_name << "\t" << problema.getN() << "\t" << problema.getM() << "\t"
            << mejor_cmax_ag << "\t" << promedio_cmax << "\t" << std_dev << "\t"
            << res_neh.mejor_makespan << "\t" << promedio_tiempo << "\t"
            << rpd_ub << "\t" << rpd_neh << "\n";
  out_tabla.close();

  ofstream out_conv("convergencia.txt");
  out_conv << "Evaluaciones\tMejor Cmax\n";
  for (const auto& punto : mejor_curva) {
    out_conv << punto.evaluacion << "\t" << punto.mejor_makespan << "\n";
  }
  out_conv.close();

  // ---------------------------------------------------------
  // 6. IMPRIMIR RESUMEN DE LAS 10 REPETICIONES
  // ---------------------------------------------------------
  cout << "\n=================================== RESUMEN DE RESULTADOS (10 REP) =======================\n";
  cout << "Inst   |  n |  m | Mejor Cmax | Promedio(10) | Std Dev |  NEH | Prom Tiempo(s) | RPD UB | RPD NEH\n";
  cout << "------------------------------------------------------------------------------------------\n";
  cout << left << setw(7) << inst_name.substr(0,7) << "| " 
       << right << setw(2) << problema.getN() << " | " 
       << setw(2) << problema.getM() << " | " 
       << setw(10) << mejor_cmax_ag << " | " 
       << setw(12) << promedio_cmax << " | " 
       << setw(7) << std_dev << " | " 
       << setw(4) << res_neh.mejor_makespan << " | " 
       << setw(14) << promedio_tiempo << " | " 
       << setw(5) << rpd_ub << "% | " 
       << setw(6) << rpd_neh << "%\n";
  cout << "==========================================================================================\n";
  cout << ">> Los datos se han guardado en 'tabla_resultados.txt'.\n";
  cout << ">> Los datos de la grafica estan en 'convergencia.txt'.\n\n";

  return 0;
}