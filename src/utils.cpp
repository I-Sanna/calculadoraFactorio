#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include <sqlite3.h>
#include <map>

using namespace std;

sqlite3* db;

struct detalle
{
    int id;
    string nombreReceta;
    string nombreComponente;
    int cantidad;
};

struct receta
{
    string nombreReceta;
    float tiempoDeFabricacion;
};

static int callbackReceta(void *veryUsed, int argc, char **argv, char **azColName){
  receta *info = (receta *)veryUsed;
  for(int i=0; i<argc; i++){
    if (strcmp(azColName[i], "nombre") == 0)
       info->nombreReceta = argv[0];
    else if (strcmp(azColName[i], "tiempo_de_fabricacion") == 0)
        info->tiempoDeFabricacion = stof(argv[1]);
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
    } else {
        std::cout << "Comando ejecutado exitosamente" << std::endl;
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
                      "tiempo_de_fabricacion REAL NOT NULL);";
    
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

void crearReceta(string nombreReceta){
    cout << "Ingrese el tiempo necesario en seg para fabricar 1 unidad: ";

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

    string comando = "INSERT INTO recetas (nombre, tiempo_de_fabricacion) VALUES ('" + nombreReceta + "'," + to_string(tiempoDeFabricacion) + ");";

    const char* sql = comando.c_str();
    
    ejecutarComandoSQL(sql);

    for (int i = 0; i < cantidadDependencias; i++){
        comando = "INSERT INTO detalle_receta (nombre_receta, nombre_componente, cantidad) VALUES ('" + nombreReceta + "','" + listaNombres[i] + "'," + to_string(listaCantidades[i]) + ");";
        const char* sql = comando.c_str();
        ejecutarComandoSQL(sql);
    }
}

void calcularMaquinasNecesarias(string nombreReceta, int tiempo){
    int rc;
    char* errorMessage = nullptr;
    receta info;
    string sqlStr = "SELECT * FROM recetas WHERE nombre = '" + nombreReceta + "'";
    const char* sql = sqlStr.c_str();

    rc = sqlite3_exec(db, sql, callbackReceta, &info, &errorMessage);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    } else {
        std::cout << "Comando ejecutado exitosamente" << std::endl;
    }

    cout << info.nombreReceta << "\n" << info.tiempoDeFabricacion << endl;
}