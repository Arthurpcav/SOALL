#!/bin/bash

# Este script executa a bateria de testes focada na variação
# do tamanho da memória para todos os algoritmos e logs.

# Garante que o script pare se algum comando falhar
set -e

echo "INÍCIO DOS TESTES DE MEMÓRIA VARIÁVEL"
echo "Data: $(date)"
echo "=========================================================================="
echo ""

# --- PARÂMETROS DE TESTE ---
LOG_FILES="compilador.log matriz.log compressor.log simulador.log"
ALGORITHMS="lru fifo lfu random"
MEM_SIZES="128 256 512 1024 2048"

# --- VARIÁVEIS FIXAS PARA ESTE CENÁRIO ---
PAGE_SIZE=4
TABLE_TYPE="hierarquica2" # Tabela padrão para uma comparação justa

# --- LOOP DE EXECUÇÃO ---
for log in $LOG_FILES
do
    for alg in $ALGORITHMS
    do
        for mem in $MEM_SIZES
        do
            echo ">>> Teste: Log=$log, Alg=$alg, Mem=${mem}KB, Pag=${PAGE_SIZE}KB, Tabela=$TABLE_TYPE"
            PAGE_TABLE_TYPE=$TABLE_TYPE ./simulador $alg $log $PAGE_SIZE $mem
            echo ""
        done
    done
done

echo "=========================================================================="
echo "FIM DOS TESTES DE MEMÓRIA VARIÁVEL."