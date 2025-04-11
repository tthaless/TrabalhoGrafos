#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

class Grafo {
private:
    int numVertices = 0;
    vector<vector<int>> matrizAdj;
    
public:
    Grafo(const string& nomeArquivo) {
        ifstream arquivo(nomeArquivo);
        if (!arquivo.is_open()) {
            cerr << "Erro ao abrir o arquivo" << endl;
            exit(1);
        }
        arquivo.close();
    }
};