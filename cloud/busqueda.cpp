#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;

// Estructura para almacenar información de un documento
struct Documento {
    int id;
    string contenido;
    string url;
    
    Documento(int _id, const string& _contenido, const string& _url) 
        : id(_id), contenido(_contenido), url(_url) {}
};

class BuscadorDocumentos {
private:
    vector<Documento> documentos;
    map<string, set<int>> indiceInvertido;
    set<string> stopWords;
    
    // Función para convertir texto a minúsculas
    string toLower(const string& str) {
        string resultado = str;
        transform(resultado.begin(), resultado.end(), resultado.begin(), ::tolower);
        return resultado;
    }
    
    // Función para limpiar una palabra (eliminar puntuación)
    string limpiarPalabra(const string& palabra) {
        string limpia;
        for (char c : palabra) {
            if (isalnum(c)) {
                limpia += tolower(c);
            }
        }
        return limpia;
    }
    
    // Cargar stop words desde archivo
    void cargarStopWords(const string& archivoStopWords) {
        ifstream archivo(archivoStopWords);
        if (!archivo.is_open()) {
            cout << "Advertencia: No se pudo cargar el archivo de stop words." << endl;
            return;
        }
        
        string palabra;
        while (archivo >> palabra) {
            stopWords.insert(toLower(palabra));
        }
        archivo.close();
        
        cout << "Stop words cargadas: " << stopWords.size() << endl;
    }
    
    // Verificar si una palabra es stop word
    bool esStopWord(const string& palabra) {
        return stopWords.find(toLower(palabra)) != stopWords.end();
    }
    
    // Procesar texto y extraer palabras válidas
    vector<string> extraerPalabras(const string& texto) {
        vector<string> palabras;
        istringstream iss(texto);
        string palabra;
        
        while (iss >> palabra) {
            string palabraLimpia = limpiarPalabra(palabra);
            if (!palabraLimpia.empty() && !esStopWord(palabraLimpia)) {
                palabras.push_back(palabraLimpia);
            }
        }
        return palabras;
    }
    
public:
    // Constructor
    BuscadorDocumentos() {}
    
    // Cargar documentos desde archivo
    void cargarDocumentos(const string& archivoDocumentos, const string& archivoStopWords = "") {
        if (!archivoStopWords.empty()) {
            cargarStopWords(archivoStopWords);
        }
        
        ifstream archivo(archivoDocumentos);
        if (!archivo.is_open()) {
            cout << "Error: No se pudo abrir el archivo " << archivoDocumentos << endl;
            return;
        }
        
        string linea;
        int docId = 1;
        const int MAX_DOCUMENTOS = 100; // Límite de documentos a procesar
        
        while (getline(archivo, linea) && docId <= MAX_DOCUMENTOS) {
            if (linea.empty()) continue;
            
            // Buscar el separador "||" para dividir URL y contenido
            size_t separador = linea.find("||");
            if (separador == string::npos) continue;
            
            string url = linea.substr(0, separador);
            string contenido = linea.substr(separador + 2);
            
            // Crear documento - cada línea es un documento
            documentos.emplace_back(docId, contenido, url);
            
            // Procesar contenido para el índice invertido
            vector<string> palabras = extraerPalabras(contenido);
            for (const string& palabra : palabras) {
                indiceInvertido[palabra].insert(docId);
            }
            
            docId++;
        }
        
        archivo.close();
        cout << "Documentos cargados: " << documentos.size() << endl;
        cout << "Índice construido." << endl;
    }
    
    // Mostrar estadísticas del índice
    void mostrarEstadisticas() {
        cout << "\n=== ESTADÍSTICAS DEL ÍNDICE ===" << endl;
        cout << "Total de documentos: " << documentos.size() << endl;
        cout << "Total de términos únicos: " << indiceInvertido.size() << endl;
        cout << "Stop words cargadas: " << stopWords.size() << endl;
    }
    
    // Buscar documentos que contengan una palabra
    set<int> buscarPalabra(const string& palabra) {
        string palabraLimpia = limpiarPalabra(palabra);
        if (indiceInvertido.find(palabraLimpia) != indiceInvertido.end()) {
            return indiceInvertido[palabraLimpia];
        }
        return set<int>();
    }
    
    // Buscar documentos que contengan todas las palabras (intersección)
    set<int> buscarMultiplesPalabras(const vector<string>& palabras) {
        if (palabras.empty()) return set<int>();
        
        set<int> resultado = buscarPalabra(palabras[0]);
        
        for (size_t i = 1; i < palabras.size(); i++) {
            set<int> documentosActuales = buscarPalabra(palabras[i]);
            set<int> interseccion;
            
            set_intersection(resultado.begin(), resultado.end(),
                           documentosActuales.begin(), documentosActuales.end(),
                           inserter(interseccion, interseccion.begin()));
            
            resultado = interseccion;
        }
        
        return resultado;
    }
    
    // Mostrar resultados de búsqueda
    void mostrarResultados(const set<int>& documentosEncontrados, const string& consulta) {
        vector<string> palabras = extraerPalabras(consulta);
        
        if (documentosEncontrados.empty()) {
            cout << "No se encontraron documentos." << endl;
            return;
        }
        
        if (palabras.size() == 1) {
            // Búsqueda de una sola palabra
            cout << "La palabra \"" << palabras[0] << "\" aparece en los documentos: ";
            bool primero = true;
            for (int docId : documentosEncontrados) {
                if (!primero) cout << ", ";
                cout << "doc" << docId;
                primero = false;
            }
            cout << "." << endl;
        } else {
            // Búsqueda de múltiples palabras
            cout << "Los documentos que contienen todas las palabras son: ";
            bool primero = true;
            for (int docId : documentosEncontrados) {
                if (!primero) cout << ", ";
                cout << "doc" << docId;
                primero = false;
            }
            cout << "." << endl;
        }
    }
    
    // Procesar consulta del modo Frecuencia
    void procesarConsulta(const string& consulta) {
        if (consulta == "salir") return;
        
        vector<string> palabras = extraerPalabras(consulta);
        if (palabras.empty()) {
            cout << "Consulta vacia." << endl;
            return;
        }
        
        set<int> resultados = buscarMultiplesPalabras(palabras);
        
        cout << "Consulta: " << consulta << endl;
        mostrarResultados(resultados, consulta);
    }
    
    // Modo Frecuencia
    void modoFrecuencia() {
        cout << "\nModo Frecuencia. Escriba su consulta o 'salir' para terminar." << endl;
        
        string consulta;
        while (true) {
            cout << "\n> ";
            getline(cin, consulta);
            
            if (consulta == "salir") {
                cout << "\nSaliendo del programa." << endl;
                break;
            }
            
            procesarConsulta(consulta);
        }
    }
    
    // Modo automático con archivo de consultas
    void modoID(const string& archivoConsultas) {
        ifstream archivo(archivoConsultas);
        if (!archivo.is_open()) {
            cout << "Error: No se pudo abrir el archivo de consultas " << archivoConsultas << endl;
            return;
        }
        
        string consulta;
        while (getline(archivo, consulta)) {
            if (!consulta.empty()) {
                vector<string> palabras = extraerPalabras(consulta);
                if (!palabras.empty()) {
                    set<int> resultados = buscarMultiplesPalabras(palabras);
                    cout << "Consulta: " << consulta << endl;
                    mostrarResultados(resultados, consulta);
                    cout << endl;
                }
            }
        }
        
        archivo.close();
    }
};

int main(int argc, char* argv[]) {
    BuscadorDocumentos buscador;
    
    if (argc < 2) {
        cout << "Cual desea usar?: " << endl;
        cout << "Para empezar a ocupar cualquer modo ingrese ./busqueda.exe y los documentos que desee" << endl;
        cout << "  Modo Frecuencia: ./busqueda.exe <gov2_pages.dat>" << endl;
        cout << "  Modo ID:  ./busqueda.exe <gov2_pages.dat> <Log-Queries.dat>" << endl;
        return 1;
    }
    
    string archivoDocumentos = argv[1];
    string archivoStopWords = "stopwords_english.dat"; // Archivo de stop words
    
    // Cargar documentos y construir índice
    buscador.cargarDocumentos(archivoDocumentos, archivoStopWords);
    
    if (argc == 2) {
        // Modo interactivo
        buscador.modoFrecuencia();
    } else if (argc == 3) {
        // Modo automático
        string archivoConsultas = argv[2];
        buscador.modoID(archivoConsultas);
    }
    
    return 0;
}