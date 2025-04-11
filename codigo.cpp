#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <string>
#include <iomanip>
using namespace std;

const int INF = 1e9;

class Grafo {
private:
    int numVertices = 0;
    vector<vector<int>> matrizAdj;
    set<int> verticesRequeridos;
    vector<pair<int, int>> arestas;
    vector<pair<int, int>> arcos;
    set<pair<int, int>> arestasRequeridas;
    set<pair<int, int>> arcosRequeridos;

    string limparEspacos(const string& s) {
        size_t inicio = s.find_first_not_of(" \t\r");
        size_t fim = s.find_last_not_of(" \t\r");
        return (inicio == string::npos || fim == string::npos) ? "" : s.substr(inicio, fim - inicio + 1);
    }

public:
    Grafo(const string& nomeArquivo) {
        ifstream arquivo(nomeArquivo);
        if (!arquivo.is_open()) {
            cerr << "Erro ao abrir o arquivo: " << nomeArquivo << endl;
            exit(1);
        }
        string linha;
        while (getline(arquivo, linha)) {
            linha = limparEspacos(linha);
            if (linha.empty()) continue;
            if (linha.find("#Nodes:") != string::npos) {
                stringstream ss(linha.substr(8)); // Pula "#Nodes:"
                ss >> numVertices;
                matrizAdj.assign(numVertices + 1, vector<int>(numVertices + 1, 0));
            } else if (linha.find("ReN.") != string::npos) {
                while (getline(arquivo, linha)) {
                    linha = limparEspacos(linha);
                    if (linha.empty() || linha[0] != 'N') break;
                    int v;
                    sscanf(linha.c_str(), "N%d", &v);
                    verticesRequeridos.insert(v);
                }
            } else if (linha.find("ReE.") != string::npos) {
                while (getline(arquivo, linha)) {
                    linha = limparEspacos(linha);
                    if (linha.empty() || linha[0] != 'E') break;
                    int id, from, to;
                    sscanf(linha.c_str(), "E%d %d %d", &id, &from, &to);
                    if (from >= 1 && from <= numVertices && to >= 1 && to <= numVertices) {
                        arestas.push_back({from, to});
                        arestasRequeridas.insert({from, to});
                        arestasRequeridas.insert({to, from});
                        matrizAdj[from][to] = 2; // Aresta requerida
                        matrizAdj[to][from] = 2;
                    } else {
                        cerr << "Aresta requerida ignorada por ter vértices inválidos: " << from << " -> " << to << endl;
                    }
                }
            } else if (linha.find("ReA.") != string::npos) {
                while (getline(arquivo, linha)) {
                    linha = limparEspacos(linha);
                    if (linha.empty() || linha[0] != 'A') break;
                    int id, from, to;
                    sscanf(linha.c_str(), "A%d %d %d", &id, &from, &to);
                    if (from >= 1 && from <= numVertices && to >= 1 && to <= numVertices) {
                        arcos.push_back({from, to});
                        arcosRequeridos.insert({from, to});
                        matrizAdj[from][to] = 1; // Arco requerido
                    } else {
                        cerr << "Arco requerido ignorado por ter vértices inválidos: " << from << " -> " << to << endl;
                    }
                }
            } else if (linha.find("EDGE") != string::npos) {
                while (getline(arquivo, linha)) {
                    linha = limparEspacos(linha);
                    if (linha.empty() || linha.substr(0, 3) != "NrE") break;
                    int id, from, to, custoTotal;
                    sscanf(linha.c_str(), "NrE%d %d %d %d", &id, &from, &to, &custoTotal);
                    if (from >= 1 && from <= numVertices && to >= 1 && to <= numVertices) {
                        matrizAdj[from][to] = 2; // Aresta opcional
                        matrizAdj[to][from] = 2;
                    } else {
                        cerr << "Aresta opcional ignorada por ter vértices inválidos: " << from << " -> " << to << endl;
                    }
                }
            } else if (linha.find("ARC") != string::npos) {
                while (getline(arquivo, linha)) {
                    linha = limparEspacos(linha);
                    if (linha.empty() || linha.substr(0, 3) != "NrA") break;
                    int id, from, to, custoTotal;
                    sscanf(linha.c_str(), "NrA%d %d %d %d", &id, &from, &to, &custoTotal);
                    if (from >= 1 && from <= numVertices && to >= 1 && to <= numVertices) {
                        matrizAdj[from][to] = 1; // Arco opcional
                    } else {
                        cerr << "Arco opcional ignorado por ter vértices inválidos: " << from << " -> " << to << endl;
                    }
                }
            }
        }
        arquivo.close();
    }

int main() {
    Grafo g("Caminho ate a instancia");
    return 0;
}