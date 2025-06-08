#ifndef ALGORITMOS_H
#define ALGORITMOS_H

#include "memoria.h"

// Assinaturas das funções que implementam os algoritmos de substituição.
// Cada função recebe o vetor de frames e o número de frames, e retorna
// o índice do frame escolhido para substituição (a "vítima").

int encontrar_vitima_lru(Frame* memoria_fisica, int num_quadros);
int encontrar_vitima_lfu(Frame* memoria_fisica, int num_quadros);
int encontrar_vitima_fifo(Frame* memoria_fisica, int num_quadros);
int encontrar_vitima_random(Frame* memoria_fisica, int num_quadros);

#endif // ALGORITMOS_H