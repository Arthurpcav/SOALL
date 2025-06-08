#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pagetable.h"

// Tabela densa

typedef struct {
    int frame_num;
    int valid;
} PTE_Densa;

typedef struct {
    PTE_Densa* entries;
    size_t num_entries;
} DensePageTable;

int lookup_densa(PageTable* pt, unsigned int page_num, int* cost) {
    DensePageTable* impl = (DensePageTable*)pt->impl;
    *cost = 1;
    if (page_num < impl->num_entries && impl->entries[page_num].valid) {
        return impl->entries[page_num].frame_num;
    }
    return -1;
}

void update_densa(PageTable* pt, unsigned int page_num, int frame_num) {
    DensePageTable* impl = (DensePageTable*)pt->impl;
    if (page_num < impl->num_entries) {
        impl->entries[page_num].frame_num = frame_num;
        impl->entries[page_num].valid = (frame_num != -1);
    }
}

void destroy_densa(PageTable* pt) {
    DensePageTable* impl = (DensePageTable*)pt->impl;
    free(impl->entries);
    free(impl);
    free(pt);
}

size_t memory_cost_densa(PageTable* pt) {
    DensePageTable* impl = (DensePageTable*)pt->impl;
    return impl->num_entries * sizeof(PTE_Densa);
}

PageTable* pagetable_densa_create(int page_shift) {
    PageTable* pt = malloc(sizeof(PageTable));
    DensePageTable* impl = malloc(sizeof(DensePageTable));

    pt->impl = impl;
    pt->lookup = lookup_densa;
    pt->update = update_densa;
    pt->destroy = destroy_densa;
    pt->memory_cost = memory_cost_densa;

    impl->num_entries = 1L << (32 - page_shift);
    impl->entries = calloc(impl->num_entries, sizeof(PTE_Densa));
    if (!impl->entries) {
        perror("Falha ao alocar tabela de páginas densa (memória insuficiente)");
        exit(EXIT_FAILURE);
    }
    return pt;
}


// Tabela hierarquica 2 e 3 níveis

typedef struct {
    void* next_level_or_frame;
    int valid;
} PTE_Hierarquica;

typedef struct HierarchicalPageTable {
    PTE_Hierarquica* root;
    int levels;
    int masks[3];
    int shifts[3];
    size_t allocated_tables_count;
    size_t entries_per_table[3];
} HierarchicalPageTable;

int lookup_hierarquica(PageTable* pt, unsigned int page_num, int* cost) {
    HierarchicalPageTable* impl = (HierarchicalPageTable*)pt->impl;
    *cost = 0;

    int idxs[3];
    idxs[0] = (page_num >> impl->shifts[0]) & impl->masks[0];
    
    (*cost)++;
    PTE_Hierarquica* current_table = impl->root;
    if (!current_table[idxs[0]].valid) return -1;
    
    if (impl->levels == 1) { //degenerado
         return (int)(long)current_table[idxs[0]].next_level_or_frame;
    }

    current_table = (PTE_Hierarquica*) current_table[idxs[0]].next_level_or_frame;
    idxs[1] = (page_num >> impl->shifts[1]) & impl->masks[1];

    (*cost)++;
    if (!current_table[idxs[1]].valid) return -1;

    if (impl->levels == 2) {
        return (int)(long)current_table[idxs[1]].next_level_or_frame;
    }
    
    current_table = (PTE_Hierarquica*) current_table[idxs[1]].next_level_or_frame;
    idxs[2] = (page_num >> impl->shifts[2]) & impl->masks[2];

    (*cost)++;
    if (!current_table[idxs[2]].valid) return -1;
    return (int)(long)current_table[idxs[2]].next_level_or_frame;
}

void update_hierarquica(PageTable* pt, unsigned int page_num, int frame_num) {
    HierarchicalPageTable* impl = (HierarchicalPageTable*)pt->impl;
    int is_invalidation = (frame_num == -1);

    int idxs[3];
    idxs[0] = (page_num >> impl->shifts[0]) & impl->masks[0];
    
    PTE_Hierarquica* level1_table = impl->root;
    if (!level1_table[idxs[0]].valid) {
        if(is_invalidation) return;
        level1_table[idxs[0]].next_level_or_frame = calloc(impl->entries_per_table[1], sizeof(PTE_Hierarquica));
        level1_table[idxs[0]].valid = 1;
        impl->allocated_tables_count++;
    }
    
    PTE_Hierarquica* level2_table = (PTE_Hierarquica*) level1_table[idxs[0]].next_level_or_frame;
    idxs[1] = (page_num >> impl->shifts[1]) & impl->masks[1];

    if (impl->levels == 2) {
        level2_table[idxs[1]].next_level_or_frame = (void*)(long)frame_num;
        level2_table[idxs[1]].valid = !is_invalidation;
        return;
    }
    
    if (!level2_table[idxs[1]].valid) {
        if(is_invalidation) return;
        level2_table[idxs[1]].next_level_or_frame = calloc(impl->entries_per_table[2], sizeof(PTE_Hierarquica));
        level2_table[idxs[1]].valid = 1;
        impl->allocated_tables_count++;
    }

    PTE_Hierarquica* level3_table = (PTE_Hierarquica*) level2_table[idxs[1]].next_level_or_frame;
    idxs[2] = (page_num >> impl->shifts[2]) & impl->masks[2];
    level3_table[idxs[2]].next_level_or_frame = (void*)(long)frame_num;
    level3_table[idxs[2]].valid = !is_invalidation;
}

void destroy_hierarquica_recursive(PTE_Hierarquica* table, int level, HierarchicalPageTable* impl) {
    if (level >= impl->levels) return;
    
    for (size_t i = 0; i < impl->entries_per_table[level]; i++) {
        if (table[i].valid) {
            destroy_hierarquica_recursive((PTE_Hierarquica*)table[i].next_level_or_frame, level + 1, impl);
        }
    }
    free(table);
}

void destroy_hierarquica(PageTable* pt) {
    HierarchicalPageTable* impl = (HierarchicalPageTable*)pt->impl;
    destroy_hierarquica_recursive(impl->root, 1, impl);
    free(impl);
    free(pt);
}

size_t memory_cost_hierarquica(PageTable* pt) {
    HierarchicalPageTable* impl = (HierarchicalPageTable*)pt->impl;
    return impl->allocated_tables_count * impl->entries_per_table[0] * sizeof(PTE_Hierarquica);
}

PageTable* pagetable_hierarquica_create(int levels, int page_shift) {
    PageTable* pt = malloc(sizeof(PageTable));
    HierarchicalPageTable* impl = malloc(sizeof(HierarchicalPageTable));
    pt->impl = impl;
    pt->lookup = lookup_hierarquica;
    pt->update = update_hierarquica;
    pt->destroy = destroy_hierarquica;
    pt->memory_cost = memory_cost_hierarquica;

    impl->levels = levels;
    int page_num_bits = 32 - page_shift;
    
    int bits[3];
    if (levels == 2) {
        bits[0] = page_num_bits / 2;
        bits[1] = page_num_bits - bits[0];
        bits[2] = 0;
    } else { 
        bits[0] = page_num_bits / 3;
        bits[1] = page_num_bits / 3;
        bits[2] = page_num_bits - (bits[0] + bits[1]);
    }

    impl->shifts[2] = 0;
    impl->shifts[1] = bits[2];
    impl->shifts[0] = bits[1] + bits[2];
    
    for(int i=0; i<3; i++){
        impl->masks[i] = (1 << bits[i]) - 1;
        impl->entries_per_table[i] = (1 << bits[i]);
    }
    
    impl->root = calloc(impl->entries_per_table[0], sizeof(PTE_Hierarquica));
    impl->allocated_tables_count = 1;

    return pt;
}


// Tabela invertida

typedef struct IPT_Node {
    unsigned int page_num;
    int frame_num;
    struct IPT_Node* next;
} IPT_Node;

typedef struct {
    IPT_Node** buckets;
    int num_buckets;
    size_t node_count;
} InvertedPageTable;

int lookup_invertida(PageTable* pt, unsigned int page_num, int* cost) {
    InvertedPageTable* impl = (InvertedPageTable*)pt->impl;
    int bucket = page_num % impl->num_buckets;
    IPT_Node* current = impl->buckets[bucket];
    *cost = 1; // custo hash + acesso inicial
    while (current) {
        if (current->page_num == page_num) {
            return current->frame_num;
        }
        current = current->next;
        (*cost)++; // custo por passo na lista
    }
    return -1; // page Fault
}

void update_invertida(PageTable* pt, unsigned int page_num, int frame_num) {
    InvertedPageTable* impl = (InvertedPageTable*)pt->impl;
    int bucket = page_num % impl->num_buckets;

    IPT_Node* current = impl->buckets[bucket];
    IPT_Node* prev = NULL;
    while(current) {
        if (current->page_num == page_num) {
            if (prev) prev->next = current->next;
            else impl->buckets[bucket] = current->next;
            free(current);
            impl->node_count--;
            break;
        }
        prev = current;
        current = current->next;
    }

    if (frame_num != -1) {
        //invalidar mapeamento antigo
        for (int i = 0; i < impl->num_buckets; i++) {
             current = impl->buckets[i];
             prev = NULL;
            while(current) {
                if (current->frame_num == frame_num) {
                    if (prev) prev->next = current->next;
                    else impl->buckets[i] = current->next;
                    free(current);
                    impl->node_count--;
                    current = NULL; //sair while
                } else {
                    prev = current;
                    current = current->next;
                }
            }
        }

        //novo mapeamento
        IPT_Node* new_node = malloc(sizeof(IPT_Node));
        new_node->page_num = page_num;
        new_node->frame_num = frame_num;
        new_node->next = impl->buckets[bucket];
        impl->buckets[bucket] = new_node;
        impl->node_count++;
    }
}

void destroy_invertida(PageTable* pt) {
    InvertedPageTable* impl = (InvertedPageTable*)pt->impl;
    for (int i = 0; i < impl->num_buckets; i++) {
        IPT_Node* current = impl->buckets[i];
        while (current) {
            IPT_Node* tmp = current;
            current = current->next;
            free(tmp);
        }
    }
    free(impl->buckets);
    free(impl);
    free(pt);
}

size_t memory_cost_invertida(PageTable* pt) {
    InvertedPageTable* impl = (InvertedPageTable*)pt->impl;
    size_t cost = impl->num_buckets * sizeof(IPT_Node*);
    cost += impl->node_count * sizeof(IPT_Node);
    return cost;
}

PageTable* pagetable_invertida_create(int num_frames) {
     PageTable* pt = malloc(sizeof(PageTable));
    InvertedPageTable* impl = malloc(sizeof(InvertedPageTable));
    pt->impl = impl;
    pt->lookup = lookup_invertida;
    pt->update = update_invertida;
    pt->destroy = destroy_invertida;
    pt->memory_cost = memory_cost_invertida;
    
    impl->num_buckets = num_frames * 2; 
    impl->buckets = calloc(impl->num_buckets, sizeof(IPT_Node*));
    impl->node_count = 0;
    
    return pt;
}