#ifndef GRAFO_H
#define GRAFO_H

#include <vector>
#include <string>
#include <set>

using namespace std;

// Declaração da função para limpar espaços de uma string.
string limparEspacosGlobal(const string& s);

// Representa um serviço que precisa ser atendido, podendo ser um nó, aresta ou arco.
struct Servico {
    string id_original;
    int id_numerico_sequencial;
    enum Tipo { NOH, ARESTA, ARCO } tipo;
    int u, v;
    int demanda;
    int custo_percurso;
    int custo_servico;
    bool atendido = false;
};

// Representa uma parada individual dentro da rota de um veículo.
struct ParadaRota {
    char tipo_parada;
    string id_servico;
    int u;
    int v;
};

// Representa a rota completa de um veículo, incluindo todos os seus custos e paradas.
struct Rota {
    int id_rota;
    int demanda_total = 0;
    int custo_total = 0;
    vector<ParadaRota> paradas;
};

// Classe principal que encapsula todos os dados e a lógica do problema do carteiro rural.
class Grafo {
private:
    int numVertices;
    vector<vector<int>> matrizAdj;
    set<int> verticesRequeridos;
    set<pair<int, int>> arestasRequeridasOriginal;
    set<pair<int, int>> arcosRequeridosOriginal;

    vector<vector<int>> dist;
    vector<vector<int>> pred;

    int capacidadeVeiculo;
    int noDeposito;
    vector<vector<int>> custosDiretos;
    vector<Servico> servicosRequeridos;

    // Aplica a heurística de otimização 2-opt para tentar melhorar as rotas existentes.
    void aplicar2Opt(vector<Rota>& todasAsRotas);

    // Encontra o próximo serviço mais próximo e viável a partir da localização atual.
    int encontrarServicoMaisProximo(int localizacaoAtual, int capacidadeAtual);
    
    // Lê o valor de clock de referência de um arquivo CSV para comparação de performance.
    long long lerClockRefDoCSV(const string& nomeInstanciaBase, int indiceColuna);

    // Realiza uma busca em profundidade (DFS) para auxiliar na contagem de componentes conexos.
    void dfs(int v, vector<bool>& visitado);

public:
    // Construtor da classe, responsável por ler e interpretar o arquivo da instância.
    Grafo(const string& nomeArquivo);

    // Salva as estatísticas básicas do grafo em um arquivo CSV.
    void salvarEstatisticas();
    // Conta o número total de arestas (não direcionadas) no grafo.
    int contarArestas();
    // Conta o número total de arcos (direcionados) no grafo.
    int contarArcos();
    // Calcula a densidade do grafo.
    double calcularDensidade();
    // Conta o número de componentes conexos do grafo.
    int contarComponentesConexos();
    // Executa o algoritmo de Floyd-Warshall para encontrar todos os caminhos mínimos.
    void calcularCaminhosMinimosComCustos();
    // Calcula o comprimento médio dos caminhos mínimos entre todos os pares de vértices.
    void calcularCaminhoMedio();
    // Calcula o diâmetro do grafo, que é o maior dos caminhos mínimos.
    void calcularDiametro();
    // Calcula a intermediação (betweenness centrality) de cada vértice.
    void calcularIntermediacao();
    // Calcula o grau mínimo entre todos os vértices do grafo.
    int calcularGrauMinimo();
    // Calcula o grau máximo entre todos os vértices do grafo.
    int calcularGrauMaximo();

    // Orquestra a criação da solução, desde a heurística construtiva até a otimização e salvamento.
    void construirESalvarSolucaoVM(const string& nomeInstancia, const string& pastaDeSaida);
};

#endif // GRAFO_H