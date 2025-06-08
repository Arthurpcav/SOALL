#!/bin/bash

# Este script executa uma bateria de testes completa no simulador de memória virtual.
# Ele redireciona toda a saída para um arquivo chamado 'resultados_completos.txt'.
# O processo pode levar vários minutos.

# Garante que o script pare se algum comando falhar
set -e

OUTPUT_FILE="resultados_completos.txt"

# Redireciona toda a saída (stdout e stderr) para o arquivo de saída
exec > "$OUTPUT_FILE" 2>&1

echo "INÍCIO DA BATERIA DE TESTES COMPLETA"
echo "Data: $(date)"
echo "=========================================================================="
echo ""

# --- PARTE 1: ANÁLISE DAS ESTRUTURAS DE TABELA DE PÁGINAS ---
# Comparamos o custo de memória e o desempenho de consulta de cada tabela
# em uma configuração "típica".
echo "PARTE 1: ANÁLISE DAS ESTRUTURAS DE TABELA DE PÁGINAS"
echo "--------------------------------------------------------------------------"
LOG_FILE_PT="compressor.log"
MEM_SIZE_PT=1024  # 1MB
PAGE_SIZE_PT=4    # 4KB
ALGORITHM_PT="lru"
for table_type in densa hierarquica2 hierarquica3 invertida
do
    echo ">>> Teste: Tabela=$table_type, Log=$LOG_FILE_PT, Alg=$ALGORITHM_PT, Mem=${MEM_SIZE_PT}KB, Pag=${PAGE_SIZE_PT}KB"
    # A tabela densa pode falhar por falta de memória. O '|| true' evita que o script pare.
    PAGE_TABLE_TYPE=$table_type ./simulador $ALGORITHM_PT $LOG_FILE_PT $PAGE_SIZE_PT $MEM_SIZE_PT || true
    echo ""
done
echo "=========================================================================="
echo ""


# --- PARTE 2: ANÁLISE COM TAMANHO DE MEMÓRIA VARIÁVEL ---
# Avaliamos todos os algoritmos em todos os logs, variando o tamanho da memória.
# Usamos uma tabela de páginas hierárquica de 2 níveis como padrão para estes testes.
echo "PARTE 2: ANÁLISE COM TAMANHO DE MEMÓRIA VARIÁVEL"
echo "--------------------------------------------------------------------------"
LOG_FILES="compilador.log matriz.log compressor.log simulador.log"
ALGORITHMS="lru fifo lfu random"
PAGE_SIZE_MEM=4
MEM_SIZES="128 256 512 1024 2048"
TABLE_TYPE_MEM="hierarquica2"
for log in $LOG_FILES
do
    for alg in $ALGORITHMS
    do
        for mem in $MEM_SIZES
        do
            echo ">>> Teste: Log=$log, Alg=$alg, Mem=${mem}KB, Pag=${PAGE_SIZE_MEM}KB, Tabela=$TABLE_TYPE_MEM"
            PAGE_TABLE_TYPE=$TABLE_TYPE_MEM ./simulador $alg $log $PAGE_SIZE_MEM $mem
            echo ""
        done
    done
done
echo "=========================================================================="
echo ""


# --- PARTE 3: ANÁLISE COM TAMANHO DE PÁGINA VARIÁVEL ---
# Avaliamos o impacto do tamanho da página em todos os logs, usando o LRU como
# algoritmo representativo. Usamos memória e tabela de páginas fixas.
echo "PARTE 3: ANÁLISE COM TAMANHO DE PÁGINA VARIÁVEL"
echo "--------------------------------------------------------------------------"
LOG_FILES_PAGE="compilador.log matriz.log compressor.log simulador.log"
ALGORITHM_PAGE="lru"
MEM_SIZE_PAGE=512 # Memória fixa de 512KB
PAGE_SIZES="2 4 8 16 32 64"
TABLE_TYPE_PAGE="hierarquica2"
for log in $LOG_FILES_PAGE
do
    for page in $PAGE_SIZES
    do
        echo ">>> Teste: Log=$log, Alg=$ALGORITHM_PAGE, Mem=${MEM_SIZE_PAGE}KB, Pag=${page}KB, Tabela=$TABLE_TYPE_PAGE"
        PAGE_TABLE_TYPE=$TABLE_TYPE_PAGE ./simulador $ALGORITHM_PAGE $log $page $MEM_SIZE_PAGE
        echo ""
    done
done
echo "=========================================================================="
echo "FIM DA BATERIA DE TESTES."