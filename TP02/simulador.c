#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "memoria.h"
#include "algoritmos.h"
#include "pagetable.h"

// Variável global definida em memoria.c
extern unsigned long total_lookup_cost;

int calcular_deslocamento(int tam_pagina_kb) {
    int tmp = tam_pagina_kb * 1024;
    int s = 0;
    while (tmp > 1) {
        tmp = tmp >> 1;
        s++;
    }
    return s;
}

int main(int argc, char *argv[]) {
    // A validação de argumentos volta para o formato original
    if (argc < 5 || argc > 6) {
        fprintf(stderr, "Uso: %s <alg_subst> <arquivo.log> <tam_pag_kb> <tam_mem_kb> [debug]\n", argv[0]);
        fprintf(stderr, "  alg_subst: lru, lfu, fifo, random\n");
        fprintf(stderr, "  Para selecionar a tabela de páginas, defina a variável de ambiente PAGE_TABLE_TYPE.\n");
        fprintf(stderr, "  Ex: PAGE_TABLE_TYPE=invertida %s lru ...\n", argv[0]);
        return 1;
    }
    
    char* nome_algoritmo_subst = argv[1];
    char* nome_arquivo = argv[2];
    int tam_pagina_kb = atoi(argv[3]);
    int tam_memoria_kb = atoi(argv[4]);

    if (argc == 6 && strcmp(argv[5], "debug") == 0) {
        debug_mode = 1;
    }

    // --- Seleção do Algoritmo de Substituição ---
    int (*algoritmo_selecionado)(Frame*, int);
    if (strcmp(nome_algoritmo_subst, "lru") == 0) algoritmo_selecionado = encontrar_vitima_lru;
    else if (strcmp(nome_algoritmo_subst, "lfu") == 0) algoritmo_selecionado = encontrar_vitima_lfu;
    else if (strcmp(nome_algoritmo_subst, "fifo") == 0) algoritmo_selecionado = encontrar_vitima_fifo;
    else if (strcmp(nome_algoritmo_subst, "random") == 0) {
        srandom(time(NULL));
        algoritmo_selecionado = encontrar_vitima_random;
    } else {
        fprintf(stderr, "Erro: Algoritmo de substituição '%s' desconhecido.\n", nome_algoritmo_subst);
        return 1;
    }
    
    // --- Cálculos de Parâmetros ---
    int deslocamento_s = calcular_deslocamento(tam_pagina_kb);
    int num_quadros = (tam_memoria_kb * 1024) / (tam_pagina_kb * 1024);

    // --- Criação da Tabela de Páginas via Variável de Ambiente ---
    PageTable* pt = NULL;
    char* nome_tipo_tabela = getenv("PAGE_TABLE_TYPE");

    // Se a variável não estiver definida, usa um padrão e avisa o usuário.
    if (nome_tipo_tabela == NULL) {
        printf("Aviso: Variável de ambiente PAGE_TABLE_TYPE não definida. Usando 'hierarquica2' como padrão.\n");
        nome_tipo_tabela = "hierarquica2";
    }

    if (strcmp(nome_tipo_tabela, "densa") == 0) {
        pt = pagetable_densa_create(deslocamento_s);
    } else if (strcmp(nome_tipo_tabela, "hierarquica2") == 0) {
        pt = pagetable_hierarquica_create(2, deslocamento_s);
    } else if (strcmp(nome_tipo_tabela, "hierarquica3") == 0) {
        pt = pagetable_hierarquica_create(3, deslocamento_s);
    } else if (strcmp(nome_tipo_tabela, "invertida") == 0) {
        pt = pagetable_invertida_create(num_quadros);
    } else {
        fprintf(stderr, "Erro: Tipo de tabela de páginas '%s' (de PAGE_TABLE_TYPE) desconhecido.\n", nome_tipo_tabela);
        return 1;
    }

    // --- Inicialização ---
    // (O resto do código é idêntico ao anterior)
    inicializar_memoria(num_quadros);
    FILE* arquivo_log = fopen(nome_arquivo, "r");
    if (!arquivo_log) {
        perror("Erro ao abrir o arquivo de log");
        pt->destroy(pt);
        liberar_memoria();
        return 1;
    }

    // --- Loop Principal ---
    printf("Executando o simulador...\n");
    unsigned int addr;
    char rw;
    unsigned long total_acessos = 0;

    while (fscanf(arquivo_log, "%x %c", &addr, &rw) == 2) {
        total_acessos++;
        unsigned int numero_pagina = addr >> deslocamento_s;
        acessar_endereco(numero_pagina, rw, pt, num_quadros, algoritmo_selecionado);
    }
    
    fclose(arquivo_log);

    // --- Relatório Final ---
    printf("\n--- Relatório Final ---\n");
    printf("Configuração:\n");
    printf("  Arquivo de entrada: %s\n", nome_arquivo);
    printf("  Tamanho da memória: %d KB\n", tam_memoria_kb);
    printf("  Tamanho das páginas: %d KB\n", tam_pagina_kb);
    printf("  Algoritmo de substituição: %s\n", nome_algoritmo_subst);
    printf("  Estrutura da Tabela de Páginas: %s (via PAGE_TABLE_TYPE)\n\n", nome_tipo_tabela);
    printf("Resultados da Simulação:\n");
    printf("  Total de acessos à memória: %lu\n", total_acessos);
    printf("  Total de page faults (páginas lidas): %u\n", paginas_lidas);
    printf("  Total de páginas escritas (dirty pages): %u\n\n", paginas_escritas);
    printf("Análise da Tabela de Páginas:\n");
    printf("  Custo de memória da tabela: %.2f KB\n", (double)pt->memory_cost(pt) / 1024.0);
    printf("  Custo médio de consulta: %.2f acessos/operação\n", (double)total_lookup_cost / (double)total_acessos);
    printf("-----------------------\n");

    // --- Limpeza ---
    pt->destroy(pt);
    liberar_memoria();
    return 0;
}