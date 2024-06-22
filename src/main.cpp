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

	if(validarExistenciaReceta(nombreReceta)){
		cout << "Ingrese la cantidad que quiere fabricar por hora: ";

		int cantidadPorHora;
		cin >> cantidadPorHora;
			
		calcularMaquinasNecesarias(nombreReceta, cantidadPorHora);
	}
	else{
		cout << "La receta ingresada no existe ¿Desea crearla? (Si/No): ";

		string validacion;
		cin >> validacion;

		if (validacion == "Si"){
			cout << "Ingrese la cantidad que fabrica la receta: ";

			int cantidad;
			cin >> cantidad;

			crearReceta(nombreReceta, cantidad);
		}
	}

	imprimirMaquinasNecesarias();

	return 0;
}

