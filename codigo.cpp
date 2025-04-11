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
    vector<vector<int>> dist;
    vector<vector<int>> pred;

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
                stringstream ss(linha.substr(8));
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
                        matrizAdj[from][to] = 2;
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
                        matrizAdj[from][to] = 1;
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
                        matrizAdj[from][to] = 2;
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
                        matrizAdj[from][to] = 1;
                    } else {
                        cerr << "Arco opcional ignorado por ter vértices inválidos: " << from << " -> " << to << endl;
                    }
                }
            }
        }
        arquivo.close();
    }

    int contarArestas() {
        int total = 0;
        for (int i = 1; i <= numVertices; ++i) {
            for (int j = i + 1; j <= numVertices; ++j) {
                if (matrizAdj[i][j] == 2) total++;
            }
        }
        return total;
    }

    int contarArcos() {
        int total = 0;
        for (int i = 1; i <= numVertices; ++i) {
            for (int j = 1; j <= numVertices; ++j) {
                if (matrizAdj[i][j] == 1) total++;
            }
        }
        return total;
    }

    double calcularDensidade() {
        int totalConexoes = contarArestas() * 2 + contarArcos();
        double maxConexoes = numVertices * (numVertices - 1);
        return totalConexoes / maxConexoes;
    }

    void dfs(int v, vector<bool>& visitado) {
        visitado[v] = true;
        for (int i = 1; i <= numVertices; ++i) {
            if (!visitado[i] && (matrizAdj[v][i] > 0 || matrizAdj[i][v] > 0)) {
                dfs(i, visitado);
            }
        }
    }

    int contarComponentesConexos() {
        vector<bool> visitado(numVertices + 1, false);
        int componentes = 0;
        for (int i = 1; i <= numVertices; ++i) {
            if (!visitado[i]) {
                dfs(i, visitado);
                componentes++;
            }
        }
        return componentes;
    }

    void salvarEstatisticas() {
        ofstream resultados("resultados.csv");
        resultados << "Metrica,Valor" << endl;
       
        resultados << "Numero total de vertices," << numVertices << endl;
        resultados << "Numero total de arestas," << contarArestas() << endl;
        resultados << "Numero total de arcos," << contarArcos() << endl;
        resultados << "Numero de vertices requeridos," << verticesRequeridos.size() << endl;
        resultados << "Numero de arestas requeridas," << arestasRequeridas.size() / 2 << endl;
        resultados << "Numero de arcos requeridos," << arcosRequeridos.size() << endl;
        resultados << "Densidade do grafo," << calcularDensidade() << endl;
        resultados << "Componentes conexos," << contarComponentesConexos() << endl;
        resultados << "Grau minimo," << calcularGrauMinimo() << endl;
        resultados << "Grau maximo," << calcularGrauMaximo() << endl;

        resultados.close();
    }

    int calcularGrauMinimo() {
        int min_grau = INF;
        for(int i = 1; i <= numVertices; i++) {
            int grau = 0;
            for(int j = 1; j <= numVertices; j++) {
                if(matrizAdj[i][j] != 0) grau++;
            }
            min_grau = min(min_grau, grau);
        }
        return min_grau;
    }
   
    int calcularGrauMaximo() {
        int max_grau = 0;
        for(int i = 1; i <= numVertices; i++) {
            int grau = 0;
            for(int j = 1; j <= numVertices; j++) {
                if(matrizAdj[i][j] != 0) grau++;
            }
            max_grau = max(max_grau, grau);
        }
        return max_grau;
    }

    void calcularCaminhosMinimos() {
        dist.assign(numVertices + 1, vector<int>(numVertices + 1, INF));
        pred.assign(numVertices + 1, vector<int>(numVertices + 1, -1));
        //Inicializa distancias
        for (int i = 1; i <= numVertices; ++i) {
            for (int j = 1; j <= numVertices; ++j) {
                if (i == j) {
                    dist[i][j] = 0;
                    pred[i][j] = i;
                } else if (matrizAdj[i][j] > 0) {
                    dist[i][j] = 1;
                    pred[i][j] = i;
                }
            }
        }
        // Floyd-Warshall
        for (int k = 1; k <= numVertices; ++k) {
            for (int i = 1; i <= numVertices; ++i) {
                for (int j = 1; j <= numVertices; ++j) {
                    if (dist[i][k] + dist[k][j] < dist[i][j]) {
                        dist[i][j] = dist[i][k] + dist[k][j];
                        pred[i][j] = pred[k][j];
                    }
                }
            }
        }
    }

    void calcularCaminhoMedio() {
        ofstream resultados("resultados.csv", ios::app);
        int soma = 0;
        int contagem = 0;
        for (int i = 1; i <= numVertices; ++i) {
            for (int j = 1; j <= numVertices; ++j) {
                if (i != j && dist[i][j] != INF) {
                    soma += dist[i][j];
                    contagem++;
                }
            }
        }
        if (contagem == 0) {
            resultados << "Caminho medio,Nao ha caminhos entre pares de vertices." << endl;
        } else {
            double caminhoMedio = static_cast<double>(soma) / contagem;
            resultados << "Caminho medio," << fixed << setprecision(2) << caminhoMedio << endl;
        }
        resultados.close();
    }

    void calcularDiametro() {
        ofstream resultados("resultados.csv", ios::app);
        int diametro = 0;
        for (int i = 1; i <= numVertices; ++i) {
            for (int j = 1; j <= numVertices; ++j) {
                if (i != j && dist[i][j] != INF) {
                    diametro = max(diametro, dist[i][j]);
                }
            }
        }
        resultados << "Diametro do grafo," << diametro << endl;
        resultados.close();
    }
};

int main() {
    Grafo g("Caminho ate a instancia");
    g.salvarEstatisticas();
    g.calcularCaminhosMinimos();
    g.calcularCaminhoMedio();
    g.calcularDiametro();
    return 0;
}