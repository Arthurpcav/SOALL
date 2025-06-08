#include <stdio.h>
#include <stdlib.h>
#include "memoria.h"
#include "algoritmos.h"

static Frame* memoria_fisica = NULL;
static long contador_tempo = 0;

unsigned int paginas_lidas = 0;
unsigned int paginas_escritas = 0;
int debug_mode = 0;
unsigned long total_lookup_cost = 0;


void inicializar_memoria(int num_quadros) {
    memoria_fisica = (Frame*) malloc(num_quadros * sizeof(Frame));
    for (int i = 0; i < num_quadros; i++) {
        memoria_fisica[i].ocupado = 0;
    }
}

void liberar_memoria() {
    if (memoria_fisica != NULL) free(memoria_fisica);
}

void acessar_endereco(unsigned int numero_pagina, char tipo_acesso,
                      PageTable* pt, int num_quadros,
                      int (*algoritmo_substituicao)(Frame*, int)) {
    contador_tempo++;
    int cost = 0;
    int indice_quadro = pt->lookup(pt, numero_pagina, &cost);
    total_lookup_cost += cost;

    // Page Hit
    if (indice_quadro != -1) {
        if (debug_mode) printf("Hit na página %u (quadro %d)\n", numero_pagina, indice_quadro);
        memoria_fisica[indice_quadro].ultimo_acesso = contador_tempo;
        memoria_fisica[indice_quadro].frequencia++;
        if (tipo_acesso == 'W') {
            memoria_fisica[indice_quadro].suja = 1;
        }
        return;
    }

    // Page Fault
    if (debug_mode) printf("Page fault para a página %u\n", numero_pagina);
    paginas_lidas++;

    int quadro_alvo = -1;
    for (int i = 0; i < num_quadros; i++) {
        if (!memoria_fisica[i].ocupado) {
            quadro_alvo = i;
            break;
        }
    }

    if (quadro_alvo == -1) {
        quadro_alvo = algoritmo_substituicao(memoria_fisica, num_quadros);
        if (debug_mode) printf("Substituindo quadro %d (página %u)\n", quadro_alvo, memoria_fisica[quadro_alvo].numero_pagina_virtual);

        // Invalida o mapeamento antigo na tabela de páginas
        pt->update(pt, memoria_fisica[quadro_alvo].numero_pagina_virtual, -1);
        
        if (memoria_fisica[quadro_alvo].suja) {
            paginas_escritas++;
        }
    }

    // Carrega a nova página
    memoria_fisica[quadro_alvo].ocupado = 1;
    memoria_fisica[quadro_alvo].numero_pagina_virtual = numero_pagina;
    memoria_fisica[quadro_alvo].suja = (tipo_acesso == 'W');
    memoria_fisica[quadro_alvo].ultimo_acesso = contador_tempo;
    memoria_fisica[quadro_alvo].frequencia = 1;

    // Atualiza a tabela de páginas com o novo mapeamento
    pt->update(pt, numero_pagina, quadro_alvo);
}