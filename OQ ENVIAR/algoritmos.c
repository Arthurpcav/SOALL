#include <stdlib.h>
#include <limits.h>
#include "algoritmos.h"

// ponteiro para o próximo frame a ser substituído no algoritmo FIFO
static int ponteiro_fifo = 0;

// LRU
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

// LFU
// Encontra o quadro que foi acessado o menor número de vezes
// em caso de empate, usa LRU como critério
int encontrar_vitima_lfu(Frame* memoria_fisica, int num_quadros) {
    long menor_frequencia = LONG_MAX;
    int indice_vitima = 0;
    for (int i = 0; i < num_quadros; i++) {
        if (memoria_fisica[i].frequencia < menor_frequencia) {
            menor_frequencia = memoria_fisica[i].frequencia;
            indice_vitima = i;
        }

        else if (memoria_fisica[i].frequencia == menor_frequencia) {
             if (memoria_fisica[i].ultimo_acesso < memoria_fisica[indice_vitima].ultimo_acesso) {
                indice_vitima = i;
             }
        }
    }
    return indice_vitima;
}

// FIFO
// Substitui a página que está na memória há mais tempo, usando um ponteiro circular
int encontrar_vitima_fifo(Frame* memoria_fisica, int num_quadros) {
    (void)memoria_fisica; // evitar warning unused parameter
    int vitima = ponteiro_fifo;
    ponteiro_fifo = (ponteiro_fifo + 1) % num_quadros;
    return vitima;
}

// Random
int encontrar_vitima_random(Frame* memoria_fisica, int num_quadros) {
    (void)memoria_fisica; // evitar warning unused parameter
    return random() % num_quadros;
}