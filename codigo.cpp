#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <string>
#include <iomanip>
#include <algorithm>
#include <x86intrin.h>

#ifdef _WIN32
#include <direct.h>
#else
#endif

using namespace std;

const int INF = 1e9;

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

struct ParadaRota {
    char tipo_parada;   
    string id_servico;    
    int u;              
    int v;              
};

struct Rota {
    int id_rota;
    int demanda_total = 0;
    int custo_total = 0;
    vector<ParadaRota> paradas;
};

string limparEspacosGlobal(const string& s) {
    size_t inicio = s.find_first_not_of(" \t\r\n");
    size_t fim = s.find_last_not_of(" \t\r\n");
    return (inicio == string::npos || fim == string::npos) ? "" : s.substr(inicio, fim - inicio + 1);
}

vector<string> lerNomesBaseInstanciasDoCSV(const string& caminhoCSV) {
    vector<string> nomes;
    ifstream csvFile(caminhoCSV);
    string linha;

    if (!csvFile.is_open()) {
        cerr << "ERRO: Nao foi possivel abrir '" << caminhoCSV << "' para ler nomes de instancias." << endl;
        return nomes;
    }

    getline(csvFile, linha);

    while (getline(csvFile, linha)) {
        stringstream ss(linha);
        string nomeInstanciaBase;
        getline(ss, nomeInstanciaBase, ',');
        if (!nomeInstanciaBase.empty()) {
            nomes.push_back(limparEspacosGlobal(nomeInstanciaBase));
        }
    }
    csvFile.close();
    if (nomes.empty()) {
        cerr << "AVISO: Nenhum nome de instancia lido de '" << caminhoCSV << "'" << endl;
    }
    return nomes;
}


class Grafo {
private:
    int numVertices = 0;
    vector<vector<int>> matrizAdj;
    set<int> verticesRequeridos;
    set<pair<int, int>> arestasRequeridasOriginal;
    set<pair<int, int>> arcosRequeridosOriginal;

    vector<vector<int>> dist;
    vector<vector<int>> pred;

    int capacidadeVeiculo = 0;  
    int noDeposito = 0;         
    vector<vector<int>> custosDiretos;
    vector<Servico> servicosRequeridos;

    int encontrarServicoMaisProximo(int localizacaoAtual, int capacidadeAtual) {
        int melhorServicoIdx = -1;
        int menorCustoParaServico = INF;

        for (size_t i = 0; i < servicosRequeridos.size(); ++i) {
            if (!servicosRequeridos[i].atendido) {
                int noInicioServico = servicosRequeridos[i].u;

                if (localizacaoAtual < 0 || localizacaoAtual > numVertices || noInicioServico < 0 || noInicioServico > numVertices) {
                    continue;
                }
                int custoParaAlcancar = dist[localizacaoAtual][noInicioServico];

                if (custoParaAlcancar != INF && servicosRequeridos[i].demanda <= capacidadeAtual) {
                    if (melhorServicoIdx == -1 ||
                        custoParaAlcancar < menorCustoParaServico ||
                        (custoParaAlcancar == menorCustoParaServico && servicosRequeridos[i].id_numerico_sequencial < servicosRequeridos[melhorServicoIdx].id_numerico_sequencial)) {
                        menorCustoParaServico = custoParaAlcancar;
                        melhorServicoIdx = i;
                    }
                }
            }
        }
        return melhorServicoIdx;
    }

    long long lerClockRefDoCSV(const string& nomeInstanciaBase, int indiceColuna) {
        ifstream csvFile("INSIRA O CAMINHO AQUI");
        string linha;

        if (!csvFile.is_open()) {
            return 0;
        }

        getline(csvFile, linha);

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

public:
    Grafo(const string& nomeArquivo) {
        int contador_id_servico = 1;
        ifstream arquivo(nomeArquivo);
        if (!arquivo.is_open()) {
            throw runtime_error("Falha ao abrir arquivo de instancia no construtor do Grafo: " + nomeArquivo);
        }
        string linha;
        string secaoAtual = "";
        
        vector<string> bufferLinhas;
        while (getline(arquivo, linha)) {
            bufferLinhas.push_back(linha);
        }
        arquivo.close();

        for(const string& linhaOriginal : bufferLinhas){
            string linhaProcessada = limparEspacosGlobal(linhaOriginal);
            if (linhaProcessada.empty() || linhaProcessada[0] == '%') continue;

            if (linhaProcessada.find("Capacity:") != string::npos) {
                sscanf(linhaProcessada.c_str(), "Capacity: %d", &capacidadeVeiculo);
            } else if (linhaProcessada.find("Depot Node:") != string::npos) {
                sscanf(linhaProcessada.c_str(), "Depot Node: %d", &noDeposito);
            } else if (linhaProcessada.find("#Nodes:") != string::npos) {
                sscanf(linhaProcessada.c_str(), "#Nodes: %d", &numVertices);
                if (numVertices > 0) {
                    custosDiretos.assign(numVertices + 1, vector<int>(numVertices + 1, INF));
                    for(int i = 0; i <= numVertices; ++i) custosDiretos[i][i] = 0;
                    matrizAdj.assign(numVertices + 1, vector<int>(numVertices + 1, 0));
                } else {
                    cerr << "Erro: Numero de vertices invalido (" << numVertices << ") no arquivo " << nomeArquivo << endl;
                    throw runtime_error("Numero de vertices invalido.");
                }
            }

            else if (linhaProcessada.find("ReN.") != string::npos) { secaoAtual = "ReN"; continue; }
            else if (linhaProcessada.find("ReE.") != string::npos) { secaoAtual = "ReE"; continue; }
            else if (linhaProcessada.find("ReA.") != string::npos) { secaoAtual = "ReA"; continue; }
            else if (linhaProcessada.find("EDGE") != string::npos) { secaoAtual = "EDGE"; continue; }
            else if (linhaProcessada.find("ARC") != string::npos) { secaoAtual = "ARC"; continue; }
            else if (linhaProcessada.find("END") != string::npos) { break; }

            if (numVertices == 0 && (secaoAtual == "ReN" || secaoAtual == "ReE" || secaoAtual == "ReA" || secaoAtual == "EDGE" || secaoAtual == "ARC")) {
                cerr << "Erro: Dados de secao encontrados antes de #Nodes ser definido para " << nomeArquivo << endl;
                throw runtime_error("Dados de secao encontrados antes de #Nodes.");
            }

            if (secaoAtual == "ReN" && linhaProcessada.rfind("N", 0) == 0) {
                char id_str[20];
                int demanda_val, custo_s_val;
                if (sscanf(linhaProcessada.c_str(), "%s %d %d", id_str, &demanda_val, &custo_s_val) == 3) {
                    int no_num = atoi(id_str + 1);
                    Servico s;
                    s.id_original = id_str;
                    s.id_numerico_sequencial = no_num;
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
            else if (secaoAtual == "ReE" && linhaProcessada.rfind("E", 0) == 0) {
                char id_str[20];
                int u_val, v_val, custo_t_val, demanda_val, custo_s_val;
                if (sscanf(linhaProcessada.c_str(), "%s %d %d %d %d %d", id_str, &u_val, &v_val, &custo_t_val, &demanda_val, &custo_s_val) == 6) {
                    int edge_num = atoi(id_str + 1);
                    Servico s;
                    s.id_original = id_str;
                    s.id_numerico_sequencial = edge_num;
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
            else if (secaoAtual == "ReA" && linhaProcessada.rfind("A", 0) == 0) {
                char id_str[20];
                int u_val, v_val, custo_t_val, demanda_val, custo_s_val;
                if (sscanf(linhaProcessada.c_str(), "%s %d %d %d %d %d", id_str, &u_val, &v_val, &custo_t_val, &demanda_val, &custo_s_val) == 6) {
                    int arc_num = atoi(id_str + 1);
                    Servico s;
                    s.id_original = id_str;
                    s.id_numerico_sequencial = arc_num;
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
            else if (secaoAtual == "EDGE" && linhaProcessada.rfind("NrE", 0) == 0) {
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
            else if (secaoAtual == "ARC" && linhaProcessada.rfind("NrA", 0) == 0) {
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

    void salvarEstatisticas() {
        ofstream resultados("resultados.csv");
        resultados << "Metrica,Valor" << endl;
        
        resultados << "Numero total de vertices," << numVertices << endl;
        resultados << "Numero total de arestas," << contarArestas() << endl;
        resultados << "Numero total de arcos," << contarArcos() << endl;
        resultados << "Numero de vertices requeridos," << verticesRequeridos.size() << endl;
        resultados << "Numero de arestas requeridas," << arestasRequeridasOriginal.size() << endl;
        resultados << "Numero de arcos requeridos," << arcosRequeridosOriginal.size() << endl;
        resultados << "Densidade do grafo," << calcularDensidade() << endl;
        resultados << "Componentes conexos," << contarComponentesConexos() << endl;
        resultados << "Grau minimo," << calcularGrauMinimo() << endl;
        resultados << "Grau maximo," << calcularGrauMaximo() << endl;

        resultados.close();
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
        double maxConexoes = static_cast<double>(numVertices) * (numVertices - 1);
        if (maxConexoes == 0) return 0.0;
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

    void calcularCaminhosMinimosComCustos() {
        if (numVertices == 0) return;
        dist.assign(numVertices + 1, vector<int>(numVertices + 1, INF));
        pred.assign(numVertices + 1, vector<int>(numVertices + 1, -1));

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

        for (int k = 0; k <= numVertices; ++k) {
            for (int i = 0; i <= numVertices; ++i) {
                for (int j = 0; j <= numVertices; ++j) {
                    if (dist[i][k] != INF && dist[k][j] != INF &&
                        (static_cast<long long>(dist[i][k]) + dist[k][j] < dist[i][j])) {
                        dist[i][j] = dist[i][k] + dist[k][j];
                        pred[i][j] = pred[k][j];
                    }
                }
            }
        }
    }

    void calcularCaminhoMedio() {
        ofstream resultados("resultados.csv", ios::app);
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

    void calcularIntermediacao() {
        ofstream resultados("resultados.csv", ios::app);
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

    int calcularGrauMinimo() {
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
    
    int calcularGrauMaximo() {
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
    
    void construirESalvarSolucaoVM(const string& nomeInstancia, const string& pastaDeSaida) {
        if (numVertices == 0 || (noDeposito == 0 && numVertices > 0) ) {
            if (numVertices == 0) cerr << "AVISO: numVertices eh 0 para " << nomeInstancia << ". ";
            if (noDeposito == 0 && numVertices > 0) cerr << "AVISO: noDeposito eh 0 para " << nomeInstancia << ". ";
            cerr << "Abortando construcao de solucao." << endl;
            return;
        }
        if (capacidadeVeiculo <= 0 && !servicosRequeridos.empty()) {
            cerr << "AVISO: Capacidade do veiculo invalida (" << capacidadeVeiculo << ") para " << nomeInstancia << ". Abortando." << endl;
            return;
        }

        unsigned long long inicio_total_algoritmo_ciclos = __rdtsc();

        calcularCaminhosMinimosComCustos();

        if (servicosRequeridos.empty()) {
            string nomeArquivoSaida = pastaDeSaida + "/sol-" + nomeInstancia + ".dat";
            ofstream arquivoSaida(nomeArquivoSaida);
            if (arquivoSaida.is_open()) {
                unsigned long long fim_vazio_ciclos = __rdtsc();
                arquivoSaida << "0" << endl;
                arquivoSaida << "0" << endl;
                arquivoSaida << (fim_vazio_ciclos - inicio_total_algoritmo_ciclos) << endl;
                arquivoSaida << "0" << endl;
                arquivoSaida.close();
            } else {
                cerr << "Erro ao criar o arquivo de saida para solucao vazia: " << nomeArquivoSaida << endl;
            }
            return;
        }

        vector<Rota> todasAsRotas;
        int servicosAtendidos = 0;
        int totalServicos = servicosRequeridos.size();

        for(auto& s : servicosRequeridos) {
            s.atendido = false;
        }

        int contadorIdRota = 1;
        int custoTotalSolucao = 0;

        while (servicosAtendidos < totalServicos) {
            Rota rotaAtual;
            rotaAtual.id_rota = contadorIdRota;

            int cargaAtual = capacidadeVeiculo;
            int localizacaoAtual = noDeposito;
            rotaAtual.paradas.push_back({'D', "0", noDeposito, noDeposito});

            bool servicoAdicionadoNestaRota = false;

            while (true) {
                int proximoServicoIdx = encontrarServicoMaisProximo(localizacaoAtual, cargaAtual);

                if (proximoServicoIdx == -1) {
                    break;
                }

                Servico& servico = servicosRequeridos[proximoServicoIdx];

                int custoParaAlcancarInicioServico = INF;
                if (localizacaoAtual >=0 && localizacaoAtual <=numVertices && servico.u >=0 && servico.u <=numVertices){
                    custoParaAlcancarInicioServico = dist[localizacaoAtual][servico.u];
                }

                if (custoParaAlcancarInicioServico == INF) {
                    servico.atendido = true;
                    servicosAtendidos++;
                    if (servicosAtendidos >= totalServicos && !servicoAdicionadoNestaRota) {
                        goto fim_loop_principal_vm;
                    }
                    continue;
                }

                rotaAtual.custo_total += custoParaAlcancarInicioServico;
                rotaAtual.custo_total += servico.custo_percurso;
                rotaAtual.custo_total += servico.custo_servico;
                rotaAtual.demanda_total += servico.demanda;
                cargaAtual -= servico.demanda;
                
                servico.atendido = true;
                servicosAtendidos++;
                localizacaoAtual = servico.v;
                servicoAdicionadoNestaRota = true;
                
                rotaAtual.paradas.push_back({'S', to_string(servico.id_numerico_sequencial), servico.u, servico.v});

                if (servicosAtendidos >= totalServicos) break;
                if (rotaAtual.paradas.size() > static_cast<size_t>(totalServicos) + 20) {
                    break;
                }
            }

            fim_loop_principal_vm:;

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
                if (!algumServicoRestanteViavel) {
                    break;
                }

                if (rotaAtual.paradas.size() <= 1) {
                    break;
                }
            }

            if (servicoAdicionadoNestaRota) {
                int custoParaRetornarAoDeposito = (localizacaoAtual >=0 && localizacaoAtual <=numVertices && noDeposito >=0 && noDeposito <=numVertices) ? dist[localizacaoAtual][noDeposito] : INF;
                if (custoParaRetornarAoDeposito == INF) {
                    rotaAtual.custo_total += INF / 2;
                } else {
                    rotaAtual.custo_total += custoParaRetornarAoDeposito;
                }
                rotaAtual.paradas.push_back({'D', "0", noDeposito, noDeposito});
                todasAsRotas.push_back(rotaAtual);
                custoTotalSolucao += rotaAtual.custo_total;
                contadorIdRota++;
            } else if (servicosAtendidos < totalServicos) {
                break;
            }

            if (contadorIdRota > totalServicos + 5 && totalServicos > 0) {
                break;
            }
        }

        unsigned long long fim_total_algoritmo_ciclos = __rdtsc();
        unsigned long long ciclos_seu_algoritmo_total = fim_total_algoritmo_ciclos - inicio_total_algoritmo_ciclos;
        
        long long clock_ref_melhor_sol_csv = lerClockRefDoCSV(nomeInstancia, 4);

        string nomeArquivoSaida = pastaDeSaida + "/sol-" + nomeInstancia + ".dat";
        ofstream arquivoSaida(nomeArquivoSaida);

        if (!arquivoSaida.is_open()) {
            cerr << "Erro ao criar o arquivo de saida: " << nomeArquivoSaida << endl;
            return;
        }

        arquivoSaida << custoTotalSolucao << endl;
        arquivoSaida << todasAsRotas.size() << endl;
        arquivoSaida << ciclos_seu_algoritmo_total << endl;
        arquivoSaida << clock_ref_melhor_sol_csv << endl;  

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
};

int main() {
    // Uso para uma instância de cada vez
    string nomeInstanciaBase = "INSIRA O NOME DA INSTANCIA";
    string caminhoCompletoInstancia = "INSIRA O CAMINHO AQUI" + nomeInstanciaBase + ".dat";
    string pastaDeSaidaParaSolucoes = "solucoes_individuais";

    cout << "Processando instancia: " << nomeInstanciaBase << endl;
    cout << "Arquivo de entrada: " << caminhoCompletoInstancia << endl;

    try {
        Grafo g(caminhoCompletoInstancia);
        
        g.salvarEstatisticas();
        g.calcularCaminhosMinimosComCustos();
        g.calcularCaminhoMedio();
        g.calcularDiametro();
        g.calcularIntermediacao();

        g.construirESalvarSolucaoVM(nomeInstanciaBase, pastaDeSaidaParaSolucoes);

    } catch (const std::exception& e) {
        cerr << "ERRO CRITICO ao processar instancia " << nomeInstanciaBase << ": " << e.what() << endl;
    }

    /*
    // Uso para todas as instâncias de uma vez
    string caminhoArquivoReferencias = "INSIRA O CAMINHO DO ARQ DE REFERENCIA";
    string pastaDasInstancias = "INSIRA O CAMINHO DA PASTA COM AS INSTANCIAS";
    string pastaDeSaidaParaTodasSolucoes = "solucoes"; // Pasta para todas as soluções

    for (const string& nomeInstanciaAtual : nomesDasInstancias) {
        cout << "\n=============================================" << endl;
        cout << "Processando instancia: " << nomeInstanciaAtual << endl;

        string arquivoInstanciaCompleto;
        char ultimoChar = pastaDasInstancias.back();
        if (ultimoChar != '/' && ultimoChar != '\\') {
            arquivoInstanciaCompleto = pastaDasInstancias + "/" + nomeInstanciaAtual + ".dat";
        } else {
            arquivoInstanciaCompleto = pastaDasInstancias + nomeInstanciaAtual + ".dat";
        }

        ifstream checkFile(arquivoInstanciaCompleto);
        if (!checkFile.good()) {
            cerr << "ERRO: Arquivo de instancia '" << arquivoInstanciaCompleto << "' nao encontrado ou nao pode ser aberto." << endl;
            cerr << "Pulando esta instancia." << endl;
            checkFile.close();
            continue;
        }
        checkFile.close();

        try {
            Grafo g_multi(arquivoInstanciaCompleto);
            // g_multi.salvarEstatisticas();
            g_multi.calcularCaminhosMinimosComCustos();
            // g_multi.calcularCaminhoMedio();
            // g_multi.calcularDiametro();
            // g_multi.calcularIntermediacao();

            g_multi.construirESalvarSolucaoVM(nomeInstanciaAtual, pastaDeSaidaParaTodasSolucoes);

        } catch (const std::exception& e) {
            cerr << "ERRO CRITICO ao processar instancia " << nomeInstanciaAtual << ": " << e.what() << endl;
            cerr << "Pulando para a proxima instancia." << endl;
        }
    }

    cout << "\n\nProcessamento de todas as " << nomesDasInstancias.size() << " instancias concluido." << endl;
    cout << "Verifique a pasta: " << pastaDeSaidaParaTodasSolucoes << endl;
    */

    return 0;
}