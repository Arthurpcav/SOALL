#include <stdlib.h>
#include <limits.h>
#include "algoritmos.h"

// Ponteiro para o próximo frame a ser substituído no algoritmo FIFO
static int ponteiro_fifo = 0;

// LRU: Least Recently Used (O Menos Recentemente Usado)
// Encontra o quadro cujo último acesso foi o mais antigo no tempo
int encontrar_vitima_lru(Frame* memoria_fisica, int num_quadros) {
    long menor_tempo = LONG_MAX;
    int indice_vitima = 0;
    for (int i = 0; i < num_quadros; i++) {
        if (memoria_fisica[i].ultimo_acesso < menor_tempo) {
            menor_tempo = memoria_fisica[i].ultimo_acesso;
            indice_vitima = i;
        }
    }
    return indice_vitima;
}

// LFU: Least Frequently Used (O Menos Frequentemente Usado)
// Encontra o quadro que foi acessado o menor número de vezes
// Em caso de empate, o LRU é usado como critério de desempate
int encontrar_vitima_lfu(Frame* memoria_fisica, int num_quadros) {
    long menor_frequencia = LONG_MAX;
    int indice_vitima = 0;
    for (int i = 0; i < num_quadros; i++) {
        if (memoria_fisica[i].frequencia < menor_frequencia) {
            menor_frequencia = memoria_fisica[i].frequencia;
            indice_vitima = i;
        }
        // Critério de desempate: se a frequência for a mesma, escolhe o menos recentemente usado
        else if (memoria_fisica[i].frequencia == menor_frequencia) {
             if (memoria_fisica[i].ultimo_acesso < memoria_fisica[indice_vitima].ultimo_acesso) {
                indice_vitima = i;
             }
        }
    }
    return indice_vitima;
}

// FIFO: First-In, First-Out
// Substitui a página que está na memória há mais tempo, usando um ponteiro circular
int encontrar_vitima_fifo(Frame* memoria_fisica, int num_quadros) {
    (void)memoria_fisica; // Evita warning de "unused parameter"
    int vitima = ponteiro_fifo;
    ponteiro_fifo = (ponteiro_fifo + 1) % num_quadros;
    return vitima;
}

// Aleatório: Escolhe uma página para substituir de forma aleatória
int encontrar_vitima_random(Frame* memoria_fisica, int num_quadros) {
    (void)memoria_fisica; // Evita warning de "unused parameter"
    return random() % num_quadros;
}