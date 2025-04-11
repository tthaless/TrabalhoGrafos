# Trabalho Prático de Grafos com C++ e Visualização em Python

Este projeto realiza a leitura, processamento e análise de grafos a partir de arquivos `.dat`, utilizando C++ para o processamento e Python para visualização dos resultados.

## 📁 Estrutura do Projeto

```
 📂 projeto/ 
├── instancias # todas as instâncias para testes 
├── output # Saída para executável e CSV
|    └── codigo.exe 
|    └── resultados.csv
├── README.md
├── codigo.cpp # Código principal em C++
└── visualizacao.ipynb # Visualização do código principal
```

---

## 🧠 Funcionalidades

### 🧩 Parte C++ (`codigo.cpp`)

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

---

### 📊 Parte Python (`visualizacao.ipynb`)

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

---

## 🛠️ Requisitos

### C++

- Compilador compatível com C++11 ou superior (ex: `g++`)
- Arquivo de entrada `.dat` formatado corretamente

### Python

- Python 3.x
- Bibliotecas:
```bash
pip install pandas matplotlib
```

## 👨‍💻 Autores

- Arienne Alves Navarro
- Thales Rodrigues Resende