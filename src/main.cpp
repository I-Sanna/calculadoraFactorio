#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

using namespace std;

int main(int argc , char* argv[]){
	inicializarDB();

	cout << "Ingrese el nombre de la receta a calcular: ";

	string nombreReceta;
	getline(cin, nombreReceta);

	cout << "Ingrese la cantidad que quiere fabricar por hora: ";

	int cantidadPorHora;
	cin >> cantidadPorHora;

	if(validarExistenciaReceta(nombreReceta)){
		calcularMaquinasNecesarias(nombreReceta, cantidadPorHora);
	}
	else{
		cout << "La receta ingresada no existe ¿Desea crearla? (Si/No): ";

		string validacion;
		cin >> validacion;

		if (validacion == "Si"){
			crearReceta(nombreReceta);
		}
	}

	imprimirMaquinasNecesarias();

	return 0;
}

