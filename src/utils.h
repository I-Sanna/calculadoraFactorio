#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>

using namespace std;

void inicializarDB();
bool validarExistenciaReceta(string nombreReceta);
void crearReceta(string nombreReceta, int cantidadPorReceta);
void calcularMaquinasNecesarias(string nombreReceta, int cantidad);
void imprimirMaquinasNecesarias();

#endif