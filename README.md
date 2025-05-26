# Trabalho Prático de Grafos com C++ e Visualização em Python

Este projeto processa grafos a partir de arquivos .dat usando C++ e Python para visualização, e desenvolve um algoritmo construtivo para o problema MCGRP que gera rotas viáveis respeitando limites de capacidade e atendendo cada serviço uma única vez.

## 📁 Estrutura do Projeto

```
 📂 projeto/ 
├── dados  
|    └── MCGRP                          # Arquivos .dat para teste (etapa 2)
|    |    └── BHW1.dat
|    |    └── BHW2.dat
|    |    └── ...
|    └── padrao_solucoes                # Arquivos .dat para referência
|    |    └── padrao_escrita.dat
|    |    └── sol-BHW1.dat
|    └── reference_values.csv           # CSV com todos os valores de referência
├── instancias                          # Arquivos .dat para teste (etapa 1)
├── output                              # Saída para solucoes, executável e CSV
|    └── solucoes 
|    └── solucoes_individuais                              
|    └── codigo.exe 
|    └── resultados.csv
├── codigo.cpp                          # Código principal em C++
├── visualizacao.ipynb                  # Visualização do código principal
└── README.md                           # Explicações 
```

---

## 🧠 Funcionalidades

### 📍 Etapa 1

#### 🧩 Parte C++ (`codigo.cpp`)

O programa realiza as seguintes operações:

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

### 📍 Etapa 2

- Leitura de arquivos `.dat` com definição de grafos
- Construção da matriz de adjacência com custos diretos
- Identificação de vértices, arestas e arcos (requeridos e opcionais)
- Cálculo de caminhos mínimos com algoritmo de **Floyd-Warshall**
- Geração de solução inicial usando heurística do **Vizinho Mais Próximo**
- Controle de capacidade dos veículos e atendimento de todos os serviços
- Registro do custo total, número de rotas e tempo de execução (ciclos de CPU)
- Exportação das soluções em arquivos `.dat` e das métricas em CSV

---

## 🛠️ Requisitos

### C++

- Compilador compatível com C++11 ou superior (ex: `g++`, MSVC).
- As bibliotecas padrão `iostream`, `fstream`, `sstream`, `vector`, `set`, `string`, `iomanip`, `algorithm` são utilizadas.
- A biblioteca `<x86intrin.h>` é utilizada para a função `__rdtsc()` para medição de ciclos de CPU. Em sistemas Windows, `direct.h` é incluído.
- O grupo pode utilizar estruturas de dados da standard library, mas funções diretamente relacionadas a grafos de frameworks como networkx ou igraph não são permitidas[cite: 35, 36].

### Python

- Python 3.x.
- As bibliotecas `pandas` e `matplotlib` são utilizadas.

## 📖 Referência

- GOLDBARG, Marco; GOLDBARG, Elizabeth; LUNA, Henrique. **Grafos: conceitos, algoritmos e aplicações**. 2. ed. Rio de Janeiro: Elsevier, 2013.

## 👨‍💻 Autores

- Arienne Alves Navarro
- Thales Rodrigues Resende