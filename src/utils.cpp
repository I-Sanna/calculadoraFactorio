#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include <sqlite3.h>
#include <map>
#include <vector>

using namespace std;

sqlite3* db;
map<string, int> maquinas;
map<string, int> recursosNecesariosPorHora;
map<string, float> cantidadProducidaPorHora;

struct detalle
{
    vector<string> componentes;
    vector<int> cantidades;
};

struct receta
{
    string nombreReceta;
    float tiempoDeFabricacion;
    int cantidadPorReceta;
};

static int callbackReceta(void *veryUsed, int argc, char **argv, char **azColName){
  receta *info = (receta *)veryUsed;
  for(int i=0; i<argc; i++){
    if (strcmp(azColName[i], "nombre") == 0)
       info->nombreReceta = argv[0];
    else if (strcmp(azColName[i], "tiempo_de_fabricacion") == 0)
        info->tiempoDeFabricacion = stof(argv[1]);
    else if (strcmp(azColName[i], "cantidad_por_receta") == 0)
        info->cantidadPorReceta = stof(argv[2]);
  }
  return 0;
}

static int callbackDetalleReceta(void *veryUsed, int argc, char **argv, char **azColName){
  detalle *info = (detalle *)veryUsed;
  for(int i=0; i < argc; i++){
    if (strcmp(azColName[i], "nombre_componente") == 0)
       info->componentes.push_back(argv[2]);
    else if (strcmp(azColName[i], "cantidad") == 0)
        info->cantidades.push_back(stof(argv[3]));
  }
  return 0;
}


void ejecutarComandoSQL(const char* sql){
    int rc;
    char* errorMessage = nullptr;

    rc = sqlite3_exec(db, sql, nullptr, 0, &errorMessage);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    }
}

void inicializarDB(){
    int rc;

    rc = sqlite3_open("../DB/recetas.db", &db);
    if (rc) {
        cerr << "Error al abrir la base de datos: " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "Base de datos abierta exitosamente." << endl;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS recetas("
                      "nombre TEXT PRIMARY KEY,"
                      "tiempo_de_fabricacion REAL NOT NULL,"
                      "cantidad_por_receta INT NOT NULL);";
    
    ejecutarComandoSQL(sql);

    sql = "CREATE TABLE IF NOT EXISTS detalle_receta("
          "id INT PRIMARY KEY,"
          "nombre_receta TEXT NOT NULL,"
          "nombre_componente TEXT NOT NULL,"
          "cantidad INTEGER NOT NULL,"
          "FOREIGN KEY (nombre_receta)"
          "  REFERENCES recetas (nombre)"
          ");";

    ejecutarComandoSQL(sql);
}

bool validarExistenciaReceta(string nombreReceta){
    const char* consulta = "SELECT * FROM recetas WHERE nombre = ?";
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_prepare_v2(db, consulta, -1, &stmt, nullptr);
    if(rc != SQLITE_OK)
    {
      cout << "Error al preparar la consulta: " << sqlite3_errmsg(db) << endl;
      sqlite3_close(db);
    }

    sqlite3_bind_text(stmt, 1, nombreReceta.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (rc == SQLITE_ROW) {
        return true;
    } else {
        return false;
    }
}

void crearReceta(string nombreReceta, int cantidadPorReceta){
    cout << "Ingrese el tiempo necesario en seg para fabricar la receta: ";

    float tiempoDeFabricacion;
    cin >> tiempoDeFabricacion;

    cout << "Ingrese la cantidad de recetas necesarias para fabricarla (no las materias primas): ";

    int cantidadDependencias;
    cin >> cantidadDependencias;

    cin.ignore();

    string listaNombres[cantidadDependencias];
    int listaCantidades[cantidadDependencias];

    string nombre;
    int cantidad;

    for (int i = 0; i < cantidadDependencias; i++){
        cout << "Ingrese el nombre del componente numero " << i << ": ";
        getline(cin, nombre);

        if (!validarExistenciaReceta(nombre)){
            cout << "La receta: " << nombre << " no se encuentra cargada";
            exit(0);
        }

        cout << "Ingrese la cantidad del componente numero " << i << ": ";
        cin >> cantidad;

        listaNombres[i] = nombre;
        listaCantidades[i] = cantidad;

        cin.ignore();
    }

    string comando = "INSERT INTO recetas (nombre, tiempo_de_fabricacion, cantidad_por_receta) VALUES ('" + nombreReceta + "'," + to_string(tiempoDeFabricacion) + "," + to_string(cantidadPorReceta) + ");";

    const char* sql = comando.c_str();
    
    ejecutarComandoSQL(sql);

    for (int i = 0; i < cantidadDependencias; i++){
        comando = "INSERT INTO detalle_receta (nombre_receta, nombre_componente, cantidad) VALUES ('" + nombreReceta + "','" + listaNombres[i] + "'," + to_string(listaCantidades[i]) + ");";
        const char* sql = comando.c_str();
        ejecutarComandoSQL(sql);
    }
}

void imprimirMaquinasNecesarias(){
    map<string, int>::iterator itr;

    for(itr=recursosNecesariosPorHora.begin();itr!=recursosNecesariosPorHora.end();itr++)
    {
        string nombreReceta = itr->first;
        int cantidadNecesaria = recursosNecesariosPorHora[nombreReceta];
        float cantidadPorHora = cantidadProducidaPorHora[nombreReceta];

        int maquinasNecesarias = cantidadNecesaria / cantidadPorHora;

        if(cantidadNecesaria / cantidadPorHora - maquinasNecesarias != 0){
            maquinasNecesarias++;
        }

        maquinas[nombreReceta] = maquinasNecesarias;
    }

    for(itr=maquinas.begin();itr!=maquinas.end();itr++)
    {
        cout<<"Receta: \""<< itr->first << "\" - Maquinas: "<< itr->second << endl;
    }
}

void calcularMaquinasNecesarias(string nombreReceta, int cantidad){
    int rc;
    char* errorMessage = nullptr;
    receta info;
    string sqlStr = "SELECT * FROM recetas WHERE nombre = '" + nombreReceta + "'";
    const char* sql = sqlStr.c_str();

    rc = sqlite3_exec(db, sql, callbackReceta, &info, &errorMessage);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    }

    float cantidadPorMaquina = 3600 / (info.tiempoDeFabricacion / info.cantidadPorReceta);
    int maquinasNecesarias = cantidad / cantidadPorMaquina;

    if(cantidad / cantidadPorMaquina - maquinasNecesarias != 0){
        maquinasNecesarias++;
    }

    recursosNecesariosPorHora[nombreReceta] += cantidad;
    cantidadProducidaPorHora[nombreReceta] = cantidadPorMaquina;

    sqlStr = "SELECT * FROM detalle_receta WHERE nombre_receta = '" + nombreReceta + "'";
    sql = sqlStr.c_str();

    detalle detalleReceta;

    rc = sqlite3_exec(db, sql, callbackDetalleReceta, &detalleReceta, &errorMessage);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    }

    for(int i = 0; i < detalleReceta.cantidades.size(); i++){
        calcularMaquinasNecesarias(detalleReceta.componentes[i], detalleReceta.cantidades[i] / info.tiempoDeFabricacion * 3600 * maquinasNecesarias);
    }
}
