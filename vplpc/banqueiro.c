#include "banqueiro.h"

#include <stdio.h>
#include <stdlib.h>

int *available = NULL;
int **maximum = NULL;
int **allocation = NULL;
int **need = NULL;
int numero_clientes = 0;
int numero_recursos = 0;

static void falha(const char *mensagem) {
    fprintf(stderr, "Erro: %s\n", mensagem);
    exit(1);
}

static int **criar_matriz(int linhas, int colunas) {
    int **matriz;
    int i;
    int j;

    matriz = malloc((size_t)linhas * sizeof(int *));
    if (matriz == NULL) {
        falha("sem memoria.");
    }

    for (i = 0; i < linhas; i++) {
        matriz[i] = malloc((size_t)colunas * sizeof(int));
        if (matriz[i] == NULL) {
            falha("sem memoria.");
        }
        for (j = 0; j < colunas; j++) {
            matriz[i][j] = 0;
        }
    }

    return matriz;
}

static void liberar_matriz(int **matriz, int linhas) {
    int i;

    if (matriz == NULL) {
        return;
    }
    for (i = 0; i < linhas; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

static void imprimir_vetor(FILE *saida, const int *vetor, int n) {
    int j;

    for (j = 0; j < n; j++) {
        if (j > 0) {
            fprintf(saida, " ");
        }
        fprintf(saida, "%d", vetor[j]);
    }
}

void iniciar_banco(int clientes, int recursos, const int disponivel[]) {
    int j;

    numero_clientes = clientes;
    numero_recursos = recursos;

    available = malloc((size_t)recursos * sizeof(int));
    if (available == NULL) {
        falha("sem memoria.");
    }
    for (j = 0; j < recursos; j++) {
        available[j] = disponivel[j];
    }

    maximum = criar_matriz(clientes, recursos);
    allocation = criar_matriz(clientes, recursos);
    need = criar_matriz(clientes, recursos);
}

void liberar_banco(void) {
    liberar_matriz(maximum, numero_clientes);
    liberar_matriz(allocation, numero_clientes);
    liberar_matriz(need, numero_clientes);
    free(available);

    maximum = NULL;
    allocation = NULL;
    need = NULL;
    available = NULL;
}

int estado_seguro(void) {
    int *trabalho;
    int *terminou;
    int i;
    int j;
    int achou;
    int seguros;

    trabalho = malloc((size_t)numero_recursos * sizeof(int));
    terminou = calloc((size_t)numero_clientes, sizeof(int));
    if (trabalho == NULL || terminou == NULL) {
        free(trabalho);
        free(terminou);
        falha("sem memoria.");
    }

    for (j = 0; j < numero_recursos; j++) {
        trabalho[j] = available[j];
    }

    seguros = 0;
    achou = 1;
    while (achou) {
        achou = 0;
        for (i = 0; i < numero_clientes; i++) {
            int cabe = 1;

            if (terminou[i]) {
                continue;
            }
            for (j = 0; j < numero_recursos; j++) {
                if (need[i][j] > trabalho[j]) {
                    cabe = 0;
                    break;
                }
            }
            if (!cabe) {
                continue;
            }
            for (j = 0; j < numero_recursos; j++) {
                trabalho[j] += allocation[i][j];
            }
            terminou[i] = 1;
            achou = 1;
            seguros++;
        }
    }

    free(trabalho);
    free(terminou);
    return seguros == numero_clientes;
}

int request_resources(int cliente, int pedido[]) {
    int j;

    for (j = 0; j < numero_recursos; j++) {
        if (pedido[j] > need[cliente][j]) {
            return PASSOU_DO_MAXIMO;
        }
    }
    for (j = 0; j < numero_recursos; j++) {
        if (pedido[j] > available[j]) {
            return FALTA_RECURSO;
        }
    }

    for (j = 0; j < numero_recursos; j++) {
        available[j] -= pedido[j];
        allocation[cliente][j] += pedido[j];
        need[cliente][j] -= pedido[j];
    }

    if (!estado_seguro()) {
        for (j = 0; j < numero_recursos; j++) {
            available[j] += pedido[j];
            allocation[cliente][j] -= pedido[j];
            need[cliente][j] += pedido[j];
        }
        return INSEGURO;
    }

    return CONCEDIDO;
}

void release_resources(int cliente, int liberacao[]) {
    int j;

    for (j = 0; j < numero_recursos; j++) {
        allocation[cliente][j] -= liberacao[j];
        need[cliente][j] += liberacao[j];
        available[j] += liberacao[j];
    }
}

void mostrar_estado(FILE *saida) {
    int i;

    fprintf(saida, "MAXIMUM | ALLOCATION | NEED\n");
    for (i = 0; i < numero_clientes; i++) {
        imprimir_vetor(saida, maximum[i], numero_recursos);
        fprintf(saida, " | ");
        imprimir_vetor(saida, allocation[i], numero_recursos);
        fprintf(saida, " | ");
        imprimir_vetor(saida, need[i], numero_recursos);
        fprintf(saida, "\n");
    }
    fprintf(saida, "AVAILABLE ");
    imprimir_vetor(saida, available, numero_recursos);
    fprintf(saida, "\n");
}
