/*
Este programa gera logs de acesso a memoria em diferentes padroes.
Padroes disponiveis:
1. Sequencial: Acessos sequenciais a memoria.
2. Aleatorio: Acessos aleatorios a memoria.
3. Localidade Temporal: Acessos a um conjunto pequeno de enderecos, repetidos ao longo do tempo.
4. Localidade Espacial: Acessos a blocos de memoria proximos entre si, com mudancas periodicas de base.

Informacoes de como rodar este arquivo de apoio:
- Compilar: gcc gerador_log.c -o gerador_log
- Executar: ./gerador_log
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#define NUMERO_DE_ACESSOS 1000000
#define ARQUIVO_DE_SAIDA_BASE "log.log"
#define HOTSPOT_SIZE 250
#define JANELA_ESPACIAL 4096

char gerar_operacao() {
    return (rand() % 2 == 0) ? 'R' : 'W';
}

void gerar_acesso_sequencial(const char *filename, int num_acessos) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        printf("Erro ao abrir o arquivo %s\n", filename);
        return;
    }

    printf("Gerando padrao SEQUENCIAL em '%s'...\n", filename);
    uint32_t endereco_atual = 0;
    const uint32_t passo = 4;

    for (int i = 0; i < num_acessos; ++i) {
        fprintf(f, "%08x %c\n", endereco_atual, gerar_operacao());
        endereco_atual += passo;
    }

    fclose(f);
    printf("Concluido!\n");
}

void gerar_acesso_aleatorio(const char *filename, int num_acessos) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        printf("Erro ao abrir o arquivo %s\n", filename);
        return;
    }

    printf("Gerando padrao ALEATORIO em '%s'...\n", filename);

    for (int i = 0; i < num_acessos; ++i) {
        uint32_t endereco_aleatorio = rand();
        endereco_aleatorio = (endereco_aleatorio << 17) ^ (rand());
        fprintf(f, "%08x %c\n", endereco_aleatorio, gerar_operacao());
    }

    fclose(f);
    printf("Concluido!\n");
}

void gerar_localidade_temporal(const char *filename, int num_acessos) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        printf("Erro ao abrir o arquivo %s\n", filename);
        return;
    }

    printf("Gerando padrao com LOCALIDADE TEMPORAL em '%s'...\n", filename);
    uint32_t hotspot[HOTSPOT_SIZE];

    for (int i = 0; i < HOTSPOT_SIZE; ++i) {
        uint32_t endereco_aleatorio = rand();
        hotspot[i] = (endereco_aleatorio << 17) ^ (rand());
    }

    for (int i = 0; i < num_acessos; ++i) {
        uint32_t endereco_escolhido = hotspot[rand() % HOTSPOT_SIZE];
        fprintf(f, "%08x %c\n", endereco_escolhido, gerar_operacao());
    }

    fclose(f);
    printf("Concluido!\n");
}

void gerar_localidade_espacial(const char *filename, int num_acessos) {
    FILE *f = fopen(filename, "w");
    if (f == NULL) {
        printf("Erro ao abrir o arquivo %s\n", filename);
        return;
    }

    printf("Gerando padrao com LOCALIDADE ESPACIAL em '%s'...\n", filename);

    uint32_t endereco_base_rand = rand();
    uint32_t endereco_base = (endereco_base_rand << 17) ^ (rand());
    if (endereco_base > (UINT32_MAX - JANELA_ESPACIAL)) {
        endereco_base = UINT32_MAX - JANELA_ESPACIAL;
    }

    for (int i = 0; i < num_acessos; ++i) {
        if (i > 0 && i % (num_acessos / 4) == 0) {
            endereco_base_rand = rand();
            endereco_base = (endereco_base_rand << 17) ^ (rand());
            if (endereco_base > (UINT32_MAX - JANELA_ESPACIAL)) {
                endereco_base = UINT32_MAX - JANELA_ESPACIAL;
            }
        }
        uint32_t offset = rand() % JANELA_ESPACIAL;
        fprintf(f, "%08x %c\n", endereco_base + offset, gerar_operacao());
    }

    fclose(f);
    printf("Concluido!\n");
}

int main() {
    srand(time(NULL));
    char escolha;
    char filename[100];

    printf("Escolha o padrao de acesso a memoria para gerar:\n");
    printf("1: Sequencial\n");
    printf("2: Aleatorio\n");
    printf("3: Localidade Temporal\n");
    printf("4: Localidade Espacial\n");
    printf("Digite o numero do padrao desejado: ");
    
    scanf(" %c", &escolha);

    switch (escolha) {
        case '1':
            snprintf(filename, sizeof(filename), "sequencial_%s", ARQUIVO_DE_SAIDA_BASE);
            gerar_acesso_sequencial(filename, NUMERO_DE_ACESSOS);
            break;
        case '2':
            snprintf(filename, sizeof(filename), "aleatorio_%s", ARQUIVO_DE_SAIDA_BASE);
            gerar_acesso_aleatorio(filename, NUMERO_DE_ACESSOS);
            break;
        case '3':
            snprintf(filename, sizeof(filename), "temporal_%s", ARQUIVO_DE_SAIDA_BASE);
            gerar_localidade_temporal(filename, NUMERO_DE_ACESSOS);
            break;
        case '4':
            snprintf(filename, sizeof(filename), "espacial_%s", ARQUIVO_DE_SAIDA_BASE);
            gerar_localidade_espacial(filename, NUMERO_DE_ACESSOS);
            break;
        default:
            printf("Opcao invalida.\n");
            break;
    }

    return 0;
}