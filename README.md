# Trabalho Prático de Grafos com C++

Este projeto processa grafos a partir de arquivos .dat usando C++ e Python para visualização, e desenvolve um algoritmo construtivo para o problema MCGRP que gera rotas viáveis respeitando limites de capacidade e atendendo cada serviço uma única vez.

## 📁 Estrutura do Projeto

```
📁 **projeto/**
├── 📂 **dados/**                       # Contém arquivos de dados para o projeto.
│   ├── 📂 **MCGRP/**                   # Instâncias de grafos no formato .dat para teste do problema MCGRP (Etapa 2 e 3).
│   │   └── BHW1.dat
│   │   └── BHW2.dat
│   │   └── ...
│   ├── 📂 **padrao_solucoes/**         # Soluções de referência para comparação, incluindo o formato de escrita esperado.
│   │   └── padrao_escrita.dat
│   │   └── sol-BHW1.dat
│   └── 📄 `reference_values.csv`       # Arquivo CSV com valores de referência para diversas instâncias.
├── 📂 **instancias/**                  # Arquivos .dat para teste (Etapa 1).
├── 📂 **output/**                      # Diretório para os arquivos de saída gerados pelo programa C++.
│   ├── 📂 **arquivos.zip/**            # Soluções em formato .zip
│   ├── 📂 **solucoes_etapa2/**         # Soluções geradas para a Etapa 2.
│   ├── 📂 **solucoes_etapa3/**         # Soluções geradas para a Etapa 3.
│   ├── 📄 `codigo.exe`
│   └── 📄 `resultados.csv`             # Arquivo CSV com as métricas e estatísticas do grafo geradas.
├── 📄 `README.md`
├── 📄 `Grafo.cpp`                      # Implementação das classes e funções relacionadas a grafos em C++.
├── 📄 `Grafo.h`                        # Definição da classe Grafo e estruturas de dados em C++.
├── 📄 `main.cpp`                       # Ponto de entrada do programa C++, responsável pela execução e coordenação.
└── 📄 `visualizacao.ipynb`             # Notebook Jupyter para visualização e análise dos resultados em Python. 
```

---

## 🧠 Funcionalidades

### 📍 Etapa 1: Pré-processamento e Análise de Grafos

#### 🧩 Parte C++ (`main.cpp`, `Grafo.h`, `Grafo.cpp`)

O programa C+ realiza as seguintes operações:

- Leitura de arquivos `.dat` com definição de grafos
- Construção da matriz de adjacência
- Identificação de vértices, arestas e arcos (requeridos e opcionais)
- Cálculo e exportação para CSV das seguintes métricas:

| Métrica                      | Descrição |
|-----------------------------|-----------|
| Número total de vértices    | Total de nós no grafo |
| Número de arestas e arcos   | Quantidade total de conexões |
| Densidade                   | Grau de conectividade do grafo |
| Componentes conexos         | Número de subgrafos conexos |
| Grau mínimo e máximo        | Grau mínimo/máximo entre os vértices |
| Caminho médio               | Média dos menores caminhos entre pares de vértices |
| Diâmetro                    | Maior menor caminho possível entre dois nós |
| Intermediação               | Centralidade de intermediação normalizada dos nós |

> Todos os dados são salvos no arquivo `resultados.csv`.

#### 📊 Parte Python (`visualizacao.ipynb`)

Este script realiza:

- Leitura do arquivo `resultados.csv` gerado pelo C++
- Visualização gráfica das principais métricas do grafo com **matplotlib**
- Geração de um gráfico de barras com:
  - Número de vértices
  - Número de arestas/arcos
  - Grau máximo/mínimo
  - Densidade
  - Componentes conexos
  - Caminho médio
  - Diâmetro

### 📍 Etapa 2: Solução Construtiva Inicial

Esta etapa foca no desenvolvimento de um algoritmo construtivo para gerar uma solução inicial para o problema MCGRP. As operações incluem:

- Leitura de arquivos `.dat` com definição de grafos.
- Construção da matriz de adjacência com custos diretos.
- Identificação de vértices, arestas e arcos (requeridos e opcionais).
- Cálculo de caminhos mínimos entre todos os pares de vértices utilizando o algoritmo de **Floyd-Warshall**.
- Geração de uma solução inicial viável utilizando a heurística do **Vizinho Mais Próximo**. Este algoritmo constrói rotas respeitando as seguintes restrições:
    - Capacidade máxima dos veículos por rota não é excedida.
    - Cada serviço requerido é atendido por exatamente uma rota.
    - O custo de demanda e serviço de um serviço são contados apenas uma vez, mesmo que a rota passe por ele múltiplas vezes.
  * **Por que o Vizinho Mais Próximo?** Esta heurística é escolhida por sua simplicidade e rapidez em gerar uma solução inicial, especialmente para instâncias maiores. Embora não seja ótima, ela oferece uma base funcional para otimizações futuras.
- Registro do custo total da solução, o número de rotas geradas e o tempo de execução (medido em ciclos de CPU).
- Exportação das soluções geradas em arquivos `.dat` (seguindo o padrão `sol-nome_instancia.dat`) e das métricas de desempenho em formato CSV.

### 📍 Etapa 3: Melhoria da Solução

Nesta etapa, o algoritmo construtivo inicial da Etapa 2 é aprimorado através de um algoritmo de busca local para otimização da solução. As principais funcionalidades são:

- Aplicação da heurística de otimização **2-opt** sobre as rotas geradas na Etapa 2. O 2-opt busca melhorar o custo total da rota através da inversão de segmentos de sub-rotas, visando eliminar cruzamentos e reduzir distâncias percorridas.
  * **Por que o 2-opt?** O 2-opt é uma técnica eficaz para otimizar rotas existentes. Ela é empregada por sua capacidade de aprimorar significativamente as soluções iniciais (mesmo aquelas geradas por métodos mais diretos) de maneira eficiente, oferecendo um bom equilíbrio entre qualidade da solução e custo computacional.
- As mesmas restrições de capacidade e atendimento de serviços da Etapa 2 são mantidas.
- O custo total da solução, o número de rotas e o tempo de execução (ciclos de CPU) são novamente registrados após a aplicação da heurística de melhoria.
- As soluções melhoradas são exportadas em arquivos `.dat` e as métricas atualizadas em CSV.

---

## 🛠️ Requisitos

### C++

- Compilador compatível com C++11 ou superior (ex: `g++`, MSVC).
- As bibliotecas padrão `iostream`, `fstream`, `sstream`, `vector`, `set`, `string`, `iomanip`, `algorithm` são utilizadas.
- A biblioteca `<x86intrin.h>` é utilizada para a função `__rdtsc()` para medição de ciclos de CPU. Em sistemas Windows, `direct.h` é incluído.

### Python

- Python 3.x.
- As bibliotecas `pandas` e `matplotlib` são utilizadas.

## 🚀 Como Executar o Código

Siga as instruções abaixo para compilar e executar o projeto:

### ⚙️ Compilando o Código C++

1.  **Navegue até o diretório raiz do projeto:**
    ```bash
    cd projeto/
    ```

2.  **Compile os arquivos `.cpp`:**
    Utilize um compilador C++ (como `g++`) para compilar `main.cpp`, `Grafo.cpp` e `Grafo.h`. Certifique-se de incluir a flag `-O2` para otimização e `-std=c++11` (ou superior) para garantir a compatibilidade com os padrões C++ utilizados.

    Exemplo para `g++`:
    ```bash
    g++ -O2 -std=c++11 main.cpp Grafo.cpp -o output/codigo.exe
    ```
    *Se você estiver no Linux/macOS, pode usar `-o output/codigo` para gerar um executável sem a extensão `.exe`.*

### ▶️ Executando o Executável C++

Após a compilação bem-sucedida, o executável `codigo.exe` (ou `codigo` no Linux/macOS) será gerado na pasta `output/`.

1.  **Navegue até o diretório `output`:**
    ```bash
    cd output/
    ```
2.  **Execute o programa:**
    ```bash
    ./codigo.exe
    ```
    *No Linux/macOS, use `./codigo`.*

    O programa processará a instância configurada em `main.cpp` e salvará os resultados e estatísticas na pasta `output/`.

    * **Para processar uma única instância:** O bloco de código responsável por processar uma única instância está atualmente comentado `main.cpp`. Você pode alterar a variável `nomeInstanciaBase_single` para testar outras instâncias individualmente. Para ativá-lo, descomente o bloco de código correspondente no `main.cpp` e recompile o projeto.
    * **Para processar todas as instâncias:** O bloco de código para processar todas as instâncias listadas em `dados/reference_values.csv` está descomentado em `main.cpp`.

### 📊 Visualizando os Resultados com Python (Jupyter Notebook)

Para visualizar as métricas geradas pelo programa C++:

1.  **Certifique-se de ter o Python e as bibliotecas necessárias instaladas:**
    Caso não as tenha, pode instalá-las via pip:
    ```bash
    pip install pandas matplotlib jupyter
    ```

2.  **Navegue até o diretório raiz do projeto:**
    ```bash
    cd projeto/
    ```

3.  **Inicie o Jupyter Notebook:**
    ```bash
    jupyter notebook
    ```

4.  **Abra o notebook `visualizacao.ipynb`:**
    No navegador, selecione e abra o arquivo `visualizacao.ipynb` localizado na raiz do projeto.

5.  **Execute as células do notebook:**
    Execute as células do notebook sequencialmente. Ele lerá o arquivo `output/resultados.csv` e exibirá as tabelas e gráficos das métricas do grafo.

---

## 📖 Referência

- GOLDBARG, Marco; GOLDBARG, Elizabeth; LUNA, Henrique. **Grafos: conceitos, algoritmos e aplicações**. 2. ed. Rio de Janeiro: Elsevier, 2013.

## 👨‍💻 Autores

- Arienne Alves Navarro
- Thales Rodrigues Resende