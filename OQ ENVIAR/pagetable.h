#ifndef PAGETABLE_H
#define PAGETABLE_H

#include <stdlib.h>

typedef struct PageTable {
    void* impl; 

    // Procura uma página e retorna o número do quadro. Retorna -1 se for page fault
    //custo (número de acessos à memória para a consulta) é retornado por referência
    int (*lookup)(struct PageTable* pt, unsigned int page_num, int* cost);

    //atualiza a tabela para mapear uma página virtual para um quadro físico
    void (*update)(struct PageTable* pt, unsigned int page_num, int frame_num);

    //libera memoria tabela 
    void (*destroy)(struct PageTable* pt);
    
    //custo de memoria da tabela (bytes)
    size_t (*memory_cost)(struct PageTable* pt);

} PageTable;

// construtores das tabelas
PageTable* pagetable_densa_create(int page_shift);
PageTable* pagetable_hierarquica_create(int levels, int page_shift);
PageTable* pagetable_invertida_create(int num_frames);

#endif