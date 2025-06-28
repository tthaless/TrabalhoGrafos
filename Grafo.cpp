#include "Grafo.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <x86intrin.h> // Para obter os ciclos de clock do processador e medir o tempo de execução.

// Inclusões específicas do sistema operativo para a criação de diretórios.
#ifdef _WIN32
#include <direct.h> // Para a função _mkdir no Windows.
#else
#include <sys/stat.h> // Para a função mkdir em sistemas Unix/Linux.
#endif

using namespace std;

const int INF = 1e9; // Representação de um valor muito elevado para simular o infinito em custos.
const int MAX_2OPT_PASSES = 2000; // Limite de iterações para a heurística 2-opt numa única rota, para evitar loops longos.

// Função auxiliar para remover espaços em branco, tabulações e quebras de linha do início e do fim de uma string.
string limparEspacosGlobal(const string& s) {
    size_t inicio = s.find_first_not_of(" \t\r\n");
    size_t fim = s.find_last_not_of(" \t\r\n");
    return (inicio == string::npos || fim == string::npos) ? "" : s.substr(inicio, fim - inicio + 1);
}

// O construtor é responsável pela leitura e interpretação do ficheiro da instância, inicializando o objeto Grafo.
Grafo::Grafo(const string& nomeArquivo) {
    // Inicialização dos membros da classe para garantir um estado inicial consistente.
    this->numVertices = 0;
    this->capacidadeVeiculo = 0;
    this->noDeposito = 0;
    
    int contador_id_servico = 1; // Atribui um ID numérico sequencial a cada serviço para facilitar a referência interna.
    ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open()) {
        throw runtime_error("Falha ao abrir o ficheiro da instância: " + nomeArquivo);
    }
    string linha;
    string secaoAtual = ""; // Variável de estado para controlar qual secção do ficheiro está a ser processada.
    
    // A leitura do ficheiro inteiro para um buffer em memória otimiza o acesso, evitando múltiplas leituras de disco.
    vector<string> bufferLinhas;
    while (getline(arquivo, linha)) {
        bufferLinhas.push_back(linha);
    }
    arquivo.close();

    // Itera sobre cada linha do buffer para extrair os dados.
    for(const string& linhaOriginal : bufferLinhas){
        // A limpeza da linha é um pré-processamento crucial para a correta interpretação dos dados.
        string linhaProcessada = limparEspacosGlobal(linhaOriginal);
        // Linhas vazias ou de comentário (iniciadas por '%') são ignoradas.
        if (linhaProcessada.empty() || linhaProcessada[0] == '%') continue;

        // Bloco de identificação de secções e leitura dos parâmetros do problema.
        if (linhaProcessada.find("Capacity:") != string::npos) {
            sscanf(linhaProcessada.c_str(), "Capacity: %d", &capacidadeVeiculo);
        } else if (linhaProcessada.find("Depot Node:") != string::npos) {
            sscanf(linhaProcessada.c_str(), "Depot Node: %d", &noDeposito);
        } else if (linhaProcessada.find("#Nodes:") != string::npos) {
            sscanf(linhaProcessada.c_str(), "#Nodes: %d", &numVertices);
            // Com o número de vértices definido, alocamos dinamicamente as matrizes necessárias.
            if (numVertices > 0) {
                custosDiretos.assign(numVertices + 1, vector<int>(numVertices + 1, INF));
                for(int i = 0; i <= numVertices; ++i) custosDiretos[i][i] = 0;
                matrizAdj.assign(numVertices + 1, vector<int>(numVertices + 1, 0));
            } else {
                cerr << "Erro: Número de vértices inválido (" << numVertices << ") no ficheiro " << nomeArquivo << endl;
                throw runtime_error("Numero de vertices invalido.");
            }
        }
        else if (linhaProcessada.find("ReN.") != string::npos) { secaoAtual = "ReN"; continue; }
        else if (linhaProcessada.find("ReE.") != string::npos) { secaoAtual = "ReE"; continue; }
        else if (linhaProcessada.find("ReA.") != string::npos) { secaoAtual = "ReA"; continue; }
        else if (linhaProcessada.find("EDGE") != string::npos) { secaoAtual = "EDGE"; continue; }
        else if (linhaProcessada.find("ARC") != string::npos) { secaoAtual = "ARC"; continue; }
        else if (linhaProcessada.find("END") != string::npos) { break; }

        if (numVertices == 0 && (secaoAtual != "")) {
            cerr << "Erro: Dados de secção encontrados antes da definição de #Nodes." << endl;
            throw runtime_error("Dados de secao encontrados antes de #Nodes.");
        }

        // Leitura de dados com base na secção atual
        if (secaoAtual == "ReN" && linhaProcessada.rfind("N", 0) == 0) { // Serviços em Nós Requeridos
            char id_str[20];
            int demanda_val, custo_s_val;
            if (sscanf(linhaProcessada.c_str(), "%s %d %d", id_str, &demanda_val, &custo_s_val) == 3) {
                int no_num = atoi(id_str + 1);
                Servico s;
                s.id_original = id_str;
                s.id_numerico_sequencial = contador_id_servico++;
                s.tipo = Servico::NOH;
                s.u = no_num; s.v = no_num;
                s.demanda = demanda_val;
                s.custo_percurso = 0;
                s.custo_servico = custo_s_val;
                servicosRequeridos.push_back(s);
                if(no_num > 0 && no_num <= numVertices) {
                   verticesRequeridos.insert(no_num);
                   matrizAdj[no_num][no_num] = 3;
                }
            }
        }
        else if (secaoAtual == "ReE" && linhaProcessada.rfind("E", 0) == 0) { // Serviços em Arestas Requeridas
            char id_str[20];
            int u_val, v_val, custo_t_val, demanda_val, custo_s_val;
            if (sscanf(linhaProcessada.c_str(), "%s %d %d %d %d %d", id_str, &u_val, &v_val, &custo_t_val, &demanda_val, &custo_s_val) == 6) {
                Servico s;
                s.id_original = id_str;
                s.id_numerico_sequencial = contador_id_servico++;
                s.tipo = Servico::ARESTA;
                s.u = u_val; s.v = v_val;
                s.demanda = demanda_val;
                s.custo_percurso = custo_t_val;
                s.custo_servico = custo_s_val;
                servicosRequeridos.push_back(s);
                if (u_val > 0 && u_val <= numVertices && v_val > 0 && v_val <= numVertices) {
                    custosDiretos[u_val][v_val] = min(custosDiretos[u_val][v_val], custo_t_val);
                    custosDiretos[v_val][u_val] = min(custosDiretos[v_val][u_val], custo_t_val);
                    matrizAdj[u_val][v_val] = 2;
                    matrizAdj[v_val][u_val] = 2;
                    arestasRequeridasOriginal.insert({min(u_val, v_val), max(u_val,v_val)});
                }
            }
        }
        else if (secaoAtual == "ReA" && linhaProcessada.rfind("A", 0) == 0) { // Serviços em Arcos Requeridos
            char id_str[20];
            int u_val, v_val, custo_t_val, demanda_val, custo_s_val;
            if (sscanf(linhaProcessada.c_str(), "%s %d %d %d %d %d", id_str, &u_val, &v_val, &custo_t_val, &demanda_val, &custo_s_val) == 6) {
                Servico s;
                s.id_original = id_str;
                s.id_numerico_sequencial = contador_id_servico++;
                s.tipo = Servico::ARCO;
                s.u = u_val; s.v = v_val;
                s.demanda = demanda_val;
                s.custo_percurso = custo_t_val;
                s.custo_servico = custo_s_val;
                servicosRequeridos.push_back(s);
                if (u_val > 0 && u_val <= numVertices && v_val > 0 && v_val <= numVertices) {
                    custosDiretos[u_val][v_val] = min(custosDiretos[u_val][v_val], custo_t_val);
                    matrizAdj[u_val][v_val] = 1;
                    arcosRequeridosOriginal.insert({u_val,v_val});
                }
            }
        }
        else if (secaoAtual == "EDGE" && linhaProcessada.rfind("NrE", 0) == 0) { // Arestas Não Requeridas
            int no_de, no_para, custo_val;
            if (sscanf(linhaProcessada.c_str(), "NrE%*d %d %d %d", &no_de, &no_para, &custo_val) == 3) {
                if (no_de > 0 && no_de <= numVertices && no_para > 0 && no_para <= numVertices) {
                    custosDiretos[no_de][no_para] = min(custosDiretos[no_de][no_para], custo_val);
                    custosDiretos[no_para][no_de] = min(custosDiretos[no_para][no_de], custo_val);
                    if (matrizAdj[no_de][no_para] == 0) matrizAdj[no_de][no_para] = 2;
                    if (matrizAdj[no_para][no_de] == 0) matrizAdj[no_para][no_de] = 2;
                }
            }
        }
        else if (secaoAtual == "ARC" && linhaProcessada.rfind("NrA", 0) == 0) { // Arcos Não Requeridos
            int no_de, no_para, custo_val;
            if (sscanf(linhaProcessada.c_str(), "NrA%*d %d %d %d", &no_de, &no_para, &custo_val) == 3) {
                if (no_de > 0 && no_de <= numVertices && no_para > 0 && no_para <= numVertices) {
                    custosDiretos[no_de][no_para] = min(custosDiretos[no_de][no_para], custo_val);
                    if (matrizAdj[no_de][no_para] == 0) matrizAdj[no_de][no_para] = 1;
                }
            }
        }
    }
}

// Implementação da heurística 2-opt para otimização local das rotas.
void Grafo::aplicar2Opt(vector<Rota>& todasAsRotas) {
    // Itera sobre cada rota para tentar otimizá-la individualmente.
    for (auto& rota : todasAsRotas) {
        // A troca só é aplicável em rotas com pelo menos duas arestas de deslocamento não adjacentes.
        if (rota.paradas.size() < 4) {
            continue;
        }

        bool melhoriaEncontradaNesteCiclo = true;
        int pass_count = 0;
        
        // Continua a aplicar o 2-opt na mesma rota até que não haja mais melhorias (convergência local).
        while (melhoriaEncontradaNesteCiclo && pass_count < MAX_2OPT_PASSES) {
            melhoriaEncontradaNesteCiclo = false;
            pass_count++;
            
            long long melhorDeltaParaRota = 0; // Armazena a maior redução de custo encontrada nesta passagem.
            size_t melhor_i = -1, melhor_j = -1; // Armazena os índices da melhor troca.

            // Seleciona a primeira aresta de deslocamento (entre a paragem i e i+1).
            for (size_t i = 0; i < rota.paradas.size() - 2; ++i) {
                // Seleciona a segunda aresta (entre j e j+1), garantindo que não seja adjacente à primeira.
                for (size_t j = i + 2; j < rota.paradas.size() - 1; ++j) {
                    
                    ParadaRota& p_i = rota.paradas[i];
                    ParadaRota& p_i_mais_1 = rota.paradas[i+1];
                    ParadaRota& p_j = rota.paradas[j];
                    ParadaRota& p_j_mais_1 = rota.paradas[j+1];

                    // Custo do percurso atual: (i -> i+1) + (j -> j+1)
                    long long custo_atual_segmento = 0;
                    if (dist[p_i.v][p_i_mais_1.u] == INF || dist[p_j.v][p_j_mais_1.u] == INF) {
                         custo_atual_segmento = INF;
                    } else {
                        custo_atual_segmento = (long long)dist[p_i.v][p_i_mais_1.u] + dist[p_j.v][p_j_mais_1.u];
                    }
                    
                    // Custo do percurso após a troca: (i -> j) + (i+1 -> j+1)
                    long long custo_novo_segmento = 0;
                    if (dist[p_i.v][p_j.u] == INF || dist[p_i_mais_1.v][p_j_mais_1.u] == INF) {
                        custo_novo_segmento = INF;
                    } else {
                        custo_novo_segmento = (long long)dist[p_i.v][p_j.u] + dist[p_i_mais_1.v][p_j_mais_1.u];
                    }

                    if (custo_atual_segmento == INF || custo_novo_segmento == INF) continue;

                    // Calcula a diferença de custo. Um valor negativo indica uma melhoria.
                    long long delta = custo_novo_segmento - custo_atual_segmento;

                    // Adota a estratégia "best improvement": armazena a melhor troca encontrada.
                    if (delta < melhorDeltaParaRota) {
                        melhorDeltaParaRota = delta;
                        melhor_i = i;
                        melhor_j = j;
                    }
                }
            }

            // Se uma melhoria foi encontrada, aplica a troca correspondente.
            if (melhor_i != static_cast<size_t>(-1)) {
                // A troca é efetuada ao inverter o segmento da rota entre os pontos de corte.
                reverse(rota.paradas.begin() + melhor_i + 1, rota.paradas.begin() + melhor_j + 1);
                // Atualiza o custo total da rota com a economia obtida.
                rota.custo_total += melhorDeltaParaRota;
                melhoriaEncontradaNesteCiclo = true; // Sinaliza para verificar novamente a rota em busca de mais melhorias.
            }
        }
    }
}

// Procura o serviço não atendido mais próximo que seja viável em termos de capacidade.
int Grafo::encontrarServicoMaisProximo(int localizacaoAtual, int capacidadeAtual) {
    int melhorServicoIdx = -1;
    int menorCustoParaServico = INF;

    // Itera sobre todos os serviços para avaliar cada um como candidato.
    for (size_t i = 0; i < servicosRequeridos.size(); ++i) {
        // O serviço só é um candidato se ainda não tiver sido atendido.
        if (!servicosRequeridos[i].atendido) {
            int noInicioServico = servicosRequeridos[i].u;

            if (localizacaoAtual < 0 || localizacaoAtual > numVertices || noInicioServico < 0 || noInicioServico > numVertices) {
                continue;
            }
            int custoParaAlcancar = dist[localizacaoAtual][noInicioServico];

            // Critérios de viabilidade: o serviço deve ser alcançável e a sua demanda não pode exceder a capacidade restante.
            if (custoParaAlcancar != INF && servicosRequeridos[i].demanda <= capacidadeAtual) {
                // Critérios de seleção: menor custo de deslocamento. Em caso de empate, o ID sequencial menor é usado como desempate.
                if (melhorServicoIdx == -1 ||
                    custoParaAlcancar < menorCustoParaServico ||
                    (custoParaAlcancar == menorCustoParaServico && servicosRequeridos[i].id_numerico_sequencial < servicosRequeridos[melhorServicoIdx].id_numerico_sequencial)) {
                    menorCustoParaServico = custoParaAlcancar;
                    melhorServicoIdx = i;
                }
            }
        }
    }
    return melhorServicoIdx; // Retorna o índice do melhor candidato encontrado.
}

// Lê o valor de referência de ciclos de clock a partir de um ficheiro CSV externo.
long long Grafo::lerClockRefDoCSV(const string& nomeInstanciaBase, int indiceColuna) {
    ifstream csvFile("dados/reference_values.csv");
    string linha;

    if (!csvFile.is_open()) {
        cerr << "AVISO: Nao foi possivel abrir 'reference_values.csv'." << endl;
        return 0;
    }

    getline(csvFile, linha); // Ignora a linha de cabeçalho.

    while (getline(csvFile, linha)) {
        stringstream ss(linha);
        string campo;
        vector<string> campos;
        while(getline(ss, campo, ',')) {
            campos.push_back(limparEspacosGlobal(campo));
        }

        if (campos.size() > static_cast<size_t>(indiceColuna) && campos[0] == nomeInstanciaBase) {
            csvFile.close();
            try {
                return stoll(campos[indiceColuna]);
            } catch (const std::exception& e) {
                cerr << "Erro ao converter valor do CSV (col " << indiceColuna << ") para " << nomeInstanciaBase
                     << " (valor: '" << campos[indiceColuna] << "'): " << e.what() << endl;
                return 0;
            }
        }
    }
    csvFile.close();
    return 0;
}

// Gera e guarda um ficheiro CSV com as principais métricas do grafo para análise.
void Grafo::salvarEstatisticas() {
    ofstream resultados("output/resultados.csv");
    if (!resultados.is_open()) {
        cerr << "ERRO: Nao foi possivel criar 'resultados.csv' para salvar estatisticas." << endl;
        return;
    }
    resultados << "Metrica,Valor" << endl;
    
    resultados << "Numero total de vertices," << numVertices << endl;
    resultados << "Numero total de arestas," << contarArestas() << endl;
    resultados << "Numero total de arcos," << contarArcos() << endl;
    resultados << "Numero de vertices requeridos," << verticesRequeridos.size() << endl;
    resultados << "Numero de arestas requeridas," << arestasRequeridasOriginal.size() << endl;
    resultados << "Numero de arcos requeridos," << arcosRequeridosOriginal.size() << endl;
    resultados << "Densidade do grafo," << fixed << setprecision(4) << calcularDensidade() << endl;
    resultados << "Componentes conexos," << contarComponentesConexos() << endl;
    resultados << "Grau minimo," << calcularGrauMinimo() << endl;
    resultados << "Grau maximo," << calcularGrauMaximo() << endl;

    resultados.close();
}

// Conta o número de arestas (não direcionadas) no grafo.
int Grafo::contarArestas() {
    int total = 0;
    for (int i = 1; i <= numVertices; ++i) {
        for (int j = i + 1; j <= numVertices; ++j) {
            if (matrizAdj[i][j] == 2) total++;
        }
    }
    return total;
}

// Conta o número de arcos (direcionados) no grafo.
int Grafo::contarArcos() {
    int total = 0;
    for (int i = 1; i <= numVertices; ++i) {
        for (int j = 1; j <= numVertices; ++j) {
            if (matrizAdj[i][j] == 1) total++;
        }
    }
    return total;
}

// Calcula a densidade do grafo.
double Grafo::calcularDensidade() {
    int totalConexoes = contarArestas() * 2 + contarArcos();
    double maxConexoes = static_cast<double>(numVertices) * (numVertices - 1);
    if (maxConexoes == 0) return 0.0;
    return totalConexoes / maxConexoes;
}

// Função recursiva de busca em profundidade (DFS) para percorrer um componente conexo.
void Grafo::dfs(int v, vector<bool>& visitado) {
    visitado[v] = true;
    for (int i = 1; i <= numVertices; ++i) {
        if (!visitado[i] && (matrizAdj[v][i] > 0 || matrizAdj[i][v] > 0)) {
            dfs(i, visitado);
        }
    }
}

// Usa a DFS para determinar o número de componentes conexos do grafo.
int Grafo::contarComponentesConexos() {
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

// Implementação do algoritmo de Floyd-Warshall para pré-calcular os caminhos mínimos entre todos os pares de vértices.
void Grafo::calcularCaminhosMinimosComCustos() {
    if (numVertices == 0) return;
    dist.assign(numVertices + 1, vector<int>(numVertices + 1, INF));
    pred.assign(numVertices + 1, vector<int>(numVertices + 1, -1));

    // Fase de inicialização: preenche as matrizes com os custos das ligações diretas.
    for (int i = 0; i <= numVertices; ++i) {
        for (int j = 0; j <= numVertices; ++j) {
            if (i == j) {
                dist[i][j] = 0;
                pred[i][j] = i;
            } else if (i > 0 && j > 0 && i <= numVertices && j <= numVertices && custosDiretos[i][j] != INF) {
                dist[i][j] = custosDiretos[i][j];
                pred[i][j] = i;
            }
        }
    }

    // Loop principal do Floyd-Warshall, que relaxa as arestas iterativamente.
    for (int k = 0; k <= numVertices; ++k) { // Vértice intermediário `k`
        for (int i = 0; i <= numVertices; ++i) { // Vértice de origem `i`
            for (int j = 0; j <= numVertices; ++j) { // Vértice de destino `j`
                // Verifica se o caminho de `i` para `j` via `k` é mais curto que o caminho conhecido.
                if (dist[i][k] != INF && dist[k][j] != INF &&
                    (static_cast<long long>(dist[i][k]) + dist[k][j] < dist[i][j])) {
                    // Se for mais curto, atualiza a distância e o predecessor.
                    dist[i][j] = dist[i][k] + dist[k][j];
                    pred[i][j] = pred[k][j];
                }
            }
        }
    }
}

// Calcula o comprimento médio do caminho entre todos os pares de nós alcançáveis.
void Grafo::calcularCaminhoMedio() {
    ofstream resultados("output/resultados.csv", ios::app);
    long long soma = 0;
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

// Calcula o diâmetro do grafo, que corresponde ao maior dos caminhos mínimos.
void Grafo::calcularDiametro() {
    ofstream resultados("output/resultados.csv", ios::app);
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

// Calcula a métrica de centralidade de intermediação (betweenness centrality) para cada vértice.
void Grafo::calcularIntermediacao() {
    ofstream resultados("output/resultados.csv", ios::app);
    vector<double> intermediacao(numVertices + 1, 0.0);
    
    for (int s = 1; s <= numVertices; ++s) {
        for (int t = 1; t <= numVertices; ++t) {
            if (s != t && dist[s][t] != INF) {
                vector<int> caminho_reverso;
                int curr = t;
                while (curr != s && curr != -1) {
                    caminho_reverso.push_back(curr);
                    curr = pred[s][curr];
                }
                if (curr == s) {
                    for (size_t i = 0; i < caminho_reverso.size() -1 ; ++i) {
                        if (caminho_reverso[i] != s && caminho_reverso[i] != t) {
                            intermediacao[caminho_reverso[i]] += 1.0;
                        }
                    }
                }
            }
        }
    }

    double totalPares = static_cast<double>(numVertices) * (numVertices - 1);
    if (totalPares > 0) {
        for (int v = 1; v <= numVertices; ++v) {
            intermediacao[v] /= totalPares;
        }
    }

    resultados << "Intermediacao dos vertices (normalizada):" << endl;
    for (int v = 1; v <= numVertices; ++v) {
        resultados << "Vertice " << v << "," << fixed << setprecision(4) << intermediacao[v] << endl;
    }
    resultados.close();
}

// Calcula o grau mínimo entre todos os vértices do grafo.
int Grafo::calcularGrauMinimo() {
    int min_grau = INF;
    if (numVertices == 0) return 0;
    for(int i = 1; i <= numVertices; i++) {
        int grau = 0;
        for(int j = 1; j <= numVertices; j++) {
            if(matrizAdj[i][j] != 0 || matrizAdj[j][i] != 0) grau++;
        }
        if (grau < min_grau) min_grau = grau;
    }
    return min_grau;
}

// Calcula o grau máximo entre todos os vértices do grafo.
int Grafo::calcularGrauMaximo() {
    int max_grau = 0;
    if (numVertices == 0) return 0;
    for(int i = 1; i <= numVertices; i++) {
        int grau = 0;
        for(int j = 1; j <= numVertices; j++) {
            if(matrizAdj[i][j] != 0 || matrizAdj[j][i] != 0) grau++;
        }
        if (grau > max_grau) max_grau = grau;
    }
    return max_grau;
}

// Método principal que coordena a construção da solução, a sua otimização e o armazenamento do resultado.
void Grafo::construirESalvarSolucaoVM(const string& nomeInstancia, const string& pastaDeSaida) {
    // Verificações de pré-condições para a execução.
    if (numVertices == 0 || (noDeposito == 0 && numVertices > 0) ) {
        cerr << "AVISO: Dados insuficientes para construir solucao para " << nomeInstancia << ". Abortando." << endl;
        return;
    }
    // Caso trivial: se não há serviços, a solução tem custo zero e nenhuma rota.
    if (servicosRequeridos.empty()) {
        string nomeArquivoSaida = pastaDeSaida + "/sol-" + nomeInstancia + ".dat";
        ofstream arquivoSaida(nomeArquivoSaida);
        if (arquivoSaida.is_open()) {
            unsigned long long inicio_total_algoritmo_ciclos_vazio = __rdtsc();
            unsigned long long fim_vazio_ciclos = __rdtsc();
            arquivoSaida << "0" << endl;
            arquivoSaida << "0" << endl;
            arquivoSaida << (fim_vazio_ciclos - inicio_total_algoritmo_ciclos_vazio) << endl;
            arquivoSaida << lerClockRefDoCSV(nomeInstancia, 4) << endl;
            arquivoSaida.close();
        }
        return;
    }

    if (capacidadeVeiculo <= 0) {
        cerr << "AVISO: Capacidade do veiculo invalida (" << capacidadeVeiculo << ") para " << nomeInstancia << ". Abortando." << endl;
        return;
    }

    // --- Início do processo de construção da solução ---
    unsigned long long inicio_total_algoritmo_ciclos = __rdtsc();

    // Passo 1: Pré-cálculo dos caminhos mínimos, fundamental para a heurística construtiva.
    calcularCaminhosMinimosComCustos();

    vector<Rota> todasAsRotas;
    int servicosAtendidos = 0;
    int totalServicos = servicosRequeridos.size();

    // Reinicia o estado de todos os serviços para "não atendido".
    for(auto& s : servicosRequeridos) {
        s.atendido = false;
    }

    int contadorIdRota = 1;

    // Loop principal: continua a criar rotas até que todos os serviços sejam atendidos.
    while (servicosAtendidos < totalServicos) {
        Rota rotaAtual;
        rotaAtual.id_rota = contadorIdRota;

        int cargaAtual = capacidadeVeiculo;
        int localizacaoAtual = noDeposito;
        // Todas as rotas iniciam-se no depósito.
        rotaAtual.paradas.push_back({'D', "0", noDeposito, noDeposito});

        bool servicoAdicionadoNestaRota = false;

        // Loop interno: constrói uma rota individual, adicionando serviços sequencialmente.
        while (true) {
            // Passo 2: Aplicação da heurística construtiva do Vizinho Mais Próximo.
            int proximoServicoIdx = encontrarServicoMaisProximo(localizacaoAtual, cargaAtual);

            // Se não for encontrado um serviço viável, a rota atual é finalizada.
            if (proximoServicoIdx == -1) {
                break;
            }

            Servico& servico = servicosRequeridos[proximoServicoIdx];
            
            int custoParaAlcancarInicioServico = INF;
            if (localizacaoAtual >= 0 && localizacaoAtual <= numVertices && servico.u >= 0 && servico.u <= numVertices){
                custoParaAlcancarInicioServico = dist[localizacaoAtual][servico.u];
            }
            
            if (custoParaAlcancarInicioServico == INF) {
                break; // Medida de segurança: o serviço escolhido é inalcançável.
            }

            // Atualização dos custos, da capacidade e do estado do serviço.
            rotaAtual.custo_total += custoParaAlcancarInicioServico;
            rotaAtual.custo_total += servico.custo_percurso;
            rotaAtual.custo_total += servico.custo_servico;
            rotaAtual.demanda_total += servico.demanda;
            cargaAtual -= servico.demanda;
            
            servico.atendido = true;
            servicosAtendidos++;
            localizacaoAtual = servico.v; // A localização atual do veículo passa a ser o fim do serviço.
            servicoAdicionadoNestaRota = true;
            
            // Adiciona a paragem de serviço à sequência da rota.
            rotaAtual.paradas.push_back({'S', to_string(servico.id_numerico_sequencial), servico.u, servico.v});

            if (servicosAtendidos >= totalServicos) break;
            if (rotaAtual.paradas.size() > static_cast<size_t>(totalServicos) + 20) {
                break; // Mecanismo de segurança para evitar loops excessivos.
            }
        }
        
        // Verificação para evitar que o algoritmo entre em loop infinito se não conseguir progredir.
        if (!servicoAdicionadoNestaRota && servicosAtendidos < totalServicos) {
            bool algumServicoRestanteViavel = false;
            for(const auto& s_check : servicosRequeridos) {
                if (!s_check.atendido && s_check.demanda <= capacidadeVeiculo) {
                    if (noDeposito >=0 && noDeposito <=numVertices && s_check.u >=0 && s_check.u <=numVertices && dist[noDeposito][s_check.u] != INF) {
                        algumServicoRestanteViavel = true;
                        break;
                    }
                }
            }
            if (!algumServicoRestanteViavel) break;
            if (rotaAtual.paradas.size() <= 1) break;
        }

        // Se a rota atendeu pelo menos um serviço, é considerada válida.
        if (servicoAdicionadoNestaRota) {
            // Adiciona o custo de regresso ao depósito.
            int custoParaRetornarAoDeposito = (localizacaoAtual >=0 && localizacaoAtual <=numVertices && noDeposito >=0 && noDeposito <=numVertices) ? dist[localizacaoAtual][noDeposito] : INF;
            if (custoParaRetornarAoDeposito == INF) {
                rotaAtual.custo_total += INF / 2; // Penaliza rotas que não conseguem regressar.
            } else {
                rotaAtual.custo_total += custoParaRetornarAoDeposito;
            }
            // Finaliza a rota com o regresso ao depósito.
            rotaAtual.paradas.push_back({'D', "0", noDeposito, noDeposito});
            todasAsRotas.push_back(rotaAtual);
            contadorIdRota++;
        } else if (servicosAtendidos < totalServicos) {
            break;
        }

        if (contadorIdRota > totalServicos + 5 && totalServicos > 0) {
            break; // Outro mecanismo de segurança para limitar o número de rotas.
        }
    }

    // Passo 3: Aplicação da heurística de melhoria 2-opt.
    aplicar2Opt(todasAsRotas);

    // O custo total da solução é a soma dos custos de todas as rotas otimizadas.
    long long custoTotalSolucao = 0;
    for(const auto& rota : todasAsRotas) {
        custoTotalSolucao += rota.custo_total;
    }

    unsigned long long fim_total_algoritmo_ciclos = __rdtsc();
    unsigned long long ciclos_seu_algoritmo_total = fim_total_algoritmo_ciclos - inicio_total_algoritmo_ciclos;
    
    long long clock_ref_melhor_sol_csv = lerClockRefDoCSV(nomeInstancia, 4);

    // Passo 4: Escrita da solução final no ficheiro de saída, conforme o formato especificado.
    string nomeArquivoSaida = pastaDeSaida + "/sol-" + nomeInstancia + ".dat";
    ofstream arquivoSaida(nomeArquivoSaida);

    if (!arquivoSaida.is_open()) {
        cerr << "Erro ao criar o ficheiro de saida: " << nomeArquivoSaida << endl;
        return;
    }

    arquivoSaida << custoTotalSolucao << endl;
    arquivoSaida << todasAsRotas.size() << endl;
    arquivoSaida << ciclos_seu_algoritmo_total << endl;
    arquivoSaida << clock_ref_melhor_sol_csv << endl; 

    // Escreve os detalhes de cada rota no ficheiro.
    for (const auto& rota : todasAsRotas) {
        arquivoSaida << 0 << " 1 "
                     << rota.id_rota << " "
                     << rota.demanda_total << " "
                     << rota.custo_total << " "
                     << rota.paradas.size();
        for (const auto& parada : rota.paradas) {
            arquivoSaida << " (" << parada.tipo_parada << " "
                         << parada.id_servico << ","
                         << parada.u << ","
                         << parada.v << ")";
        }
        arquivoSaida << endl;
    }
    arquivoSaida.close();
}
