#ifndef MEMORIA_H
#define MEMORIA_H

#include <stdio.h>
#include "pagetable.h"

typedef struct {
    int ocupado;
    unsigned int numero_pagina_virtual;
    int suja;
    long ultimo_acesso;
    long frequencia;
} Frame;

extern unsigned int paginas_lidas;
extern unsigned int paginas_escritas;
extern int debug_mode;

void inicializar_memoria(int num_quadros);

void acessar_endereco(unsigned int numero_pagina, char tipo_acesso,
                      PageTable* pt, int num_quadros,
                      int (*algoritmo_substituicao)(Frame*, int));

void liberar_memoria();

#endif