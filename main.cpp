#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "Grafo.h"

#ifdef _WIN32
#include <direct.h> // Para _mkdir no Windows
#else
#include <sys/stat.h> // Para mkdir em sistemas Unix/Linux
#endif

using namespace std;

// Lê os nomes base das instâncias a partir de um arquivo CSV de referência.
vector<string> lerNomesBaseInstanciasDoCSV(const string& caminhoCSV) {
    vector<string> nomes;
    ifstream csvFile(caminhoCSV);
    string linha;

    if (!csvFile.is_open()) {
        cerr << "ERRO: Nao foi possivel abrir '" << caminhoCSV << "' para ler nomes de instancias." << endl;
        return nomes;
    }

    getline(csvFile, linha); // Ignora a linha de cabeçalho

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

// Função principal.
int main() {

    // Bloco para processar uma única instância.
    /*
    string nomeInstanciaBase_single = "Nome da instância";
    string caminhoCompletoInstancia_single = "dados/MCGRP/" + nomeInstanciaBase_single + ".dat";
    string pastaDeSaidaParaSolucoes_single = "output/solucoes_etapa3/";

    #ifdef _WIN32
        _mkdir(pastaDeSaidaParaSolucoes_single.c_str());
    #else
        mkdir(pastaDeSaidaParaSolucoes_single.c_str(), 0777);
    #endif

    cout << "Processando instancia: " << nomeInstanciaBase_single << endl;
    cout << "Arquivo de entrada: " << caminhoCompletoInstancia_single << endl;

    try {
        Grafo g_single(caminhoCompletoInstancia_single);
        
        g_single.salvarEstatisticas();
        g_single.calcularCaminhosMinimosComCustos();
        g_single.calcularCaminhoMedio();
        g_single.calcularDiametro();
        g_single.calcularIntermediacao();

        g_single.construirESalvarSolucaoVM(nomeInstanciaBase_single, pastaDeSaidaParaSolucoes_single);

        cout << "Solucao para a instancia '" << nomeInstanciaBase_single 
             << "' gerada com sucesso em: " << pastaDeSaidaParaSolucoes_single << endl;

    } catch (const std::exception& e) {
        cerr << "ERRO CRITICO ao processar instancia " << nomeInstanciaBase_single << ": " << e.what() << endl;
    }
    */

    // Bloco para processar todas as instâncias listadas de uma vez.

    string caminhoArquivoReferencias = "dados/reference_values.csv";
    string pastaDasInstancias = "dados/MCGRP/";
    string pastaDeSaidaParaTodasSolucoes = "output/solucoes_etapa3";

    #ifdef _WIN32
        _mkdir(pastaDeSaidaParaTodasSolucoes.c_str());
    #else
        mkdir(pastaDeSaidaParaTodasSolucoes.c_str(), 0777);
    #endif

    vector<string> nomesDasInstancias = lerNomesBaseInstanciasDoCSV(caminhoArquivoReferencias);

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
            
            // Calcula os caminhos mínimos, essencial para a heurística.
            g_multi.calcularCaminhosMinimosComCustos();
            
            // Constrói e salva a solução para a instância atual.
            g_multi.construirESalvarSolucaoVM(nomeInstanciaAtual, pastaDeSaidaParaTodasSolucoes);

        } catch (const std::exception& e) {
            cerr << "ERRO CRITICO ao processar instancia " << nomeInstanciaAtual << ": " << e.what() << endl;
            cerr << "Pulando para a proxima instancia." << endl;
        }
    }

    cout << "\n\nProcessamento de todas as " << nomesDasInstancias.size() << " instancias concluido." << endl;
    cout << "Verifique a pasta: " << pastaDeSaidaParaTodasSolucoes << endl;

    return 0;
}