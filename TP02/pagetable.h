#ifndef PAGETABLE_H
#define PAGETABLE_H

#include <stdlib.h>

// Estrutura genérica para uma Tabela de Páginas
// Usamos ponteiros de função para implementar polimorfismo em C
typedef struct PageTable {
    void* impl; // Ponteiro para a implementação específica (ex: DensePageTable)

    // Procura uma página e retorna o número do quadro. Retorna -1 se for page fault
    // O custo (número de acessos à memória para a consulta) é retornado por referência
    int (*lookup)(struct PageTable* pt, unsigned int page_num, int* cost);

    // Atualiza a tabela para mapear uma página virtual para um quadro físico
    void (*update)(struct PageTable* pt, unsigned int page_num, int frame_num);

    // Libera toda a memória usada pela tabela de páginas
    void (*destroy)(struct PageTable* pt);
    
    // Retorna o custo de memória da tabela em bytes
    size_t (*memory_cost)(struct PageTable* pt);

} PageTable;

// Funções "construtoras" para cada tipo de tabela de páginas
PageTable* pagetable_densa_create(int page_shift);
PageTable* pagetable_hierarquica_create(int levels, int page_shift);
PageTable* pagetable_invertida_create(int num_frames);

#endif