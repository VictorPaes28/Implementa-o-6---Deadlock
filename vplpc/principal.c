#include "banqueiro.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAM_LINHA 256

static void erro(const char *mensagem) {
    fprintf(stderr, "Erro: %s\n", mensagem);
    exit(1);
}

static void erro_arquivo(FILE *arq, const char *mensagem) {
    if (arq != NULL) {
        fclose(arq);
    }
    erro(mensagem);
}

static int texto_inteiro(const char *texto, int *valor) {
    const char *p;
    long n = 0;

    if (texto == NULL || texto[0] == '\0') {
        return 0;
    }

    for (p = texto; *p != '\0'; p++) {
        if (!isdigit((unsigned char)*p)) {
            return 0;
        }
        if (n > (2147483647L - (*p - '0')) / 10) {
            return 0;
        }
        n = n * 10 + (*p - '0');
    }

    if (valor != NULL) {
        *valor = (int)n;
    }
    return 1;
}

static void tirar_fim(char *linha) {
    size_t n = strlen(linha);

    while (n > 0 && (linha[n - 1] == '\n' || linha[n - 1] == '\r')) {
        linha[--n] = '\0';
    }
}

static int linha_cheia(const char *linha) {
    return strchr(linha, '\n') == NULL && strchr(linha, '\r') == NULL;
}

static int linha_de_cliente(const char *linha, int *quantidade) {
    int n = 1;
    const char *p;

    if (linha[0] == '\0' || linha[0] == ',') {
        return 0;
    }

    for (p = linha; *p != '\0'; p++) {
        if (*p == ',') {
            if (p[1] == ',' || p[1] == '\0') {
                return 0;
            }
            n++;
        } else if (!isdigit((unsigned char)*p)) {
            return 0;
        }
    }

    *quantidade = n;
    return 1;
}

static int ler_clientes(int recursos) {
    FILE *arq = fopen("customer.txt", "r");
    char linha[TAM_LINHA];
    int clientes = 0;
    int colunas = 0;

    if (arq == NULL) {
        erro("nao foi possivel abrir customer.txt.");
    }

    while (fgets(linha, TAM_LINHA, arq) != NULL) {
        int n = 0;

        if (linha_cheia(linha) && !feof(arq)) {
            erro_arquivo(arq, "customer.txt com formato invalido.");
        }
        tirar_fim(linha);
        if (!linha_de_cliente(linha, &n)) {
            erro_arquivo(arq, "customer.txt com formato invalido.");
        }
        if (clientes == 0) {
            colunas = n;
        } else if (n != colunas) {
            erro_arquivo(arq, "customer.txt com formato invalido.");
        }
        clientes++;
    }

    fclose(arq);

    if (clientes == 0) {
        erro("customer.txt com formato invalido.");
    }
    if (colunas != recursos) {
        erro("a quantidade de recursos da chamada nao confere com customer.txt.");
    }
    return clientes;
}

static void conferir_comando(char *linha, int recursos, int clientes) {
    char *tok;
    int cliente;
    int qtd = 0;

    tok = strtok(linha, " \t");
    if (tok == NULL) {
        erro("commands.txt com formato invalido.");
    }

    if (strcmp(tok, "*") == 0) {
        if (strtok(NULL, " \t") != NULL) {
            erro("commands.txt com formato invalido.");
        }
        return;
    }

    if (strcmp(tok, "RQ") != 0 && strcmp(tok, "RL") != 0) {
        erro("commands.txt com formato invalido.");
    }

    tok = strtok(NULL, " \t");
    if (tok == NULL || !texto_inteiro(tok, &cliente)) {
        erro("commands.txt com formato invalido.");
    }
    if (cliente >= clientes) {
        erro("cliente inexistente no commands.txt.");
    }

    tok = strtok(NULL, " \t");
    while (tok != NULL) {
        if (!texto_inteiro(tok, NULL)) {
            erro("commands.txt com formato invalido.");
        }
        qtd++;
        tok = strtok(NULL, " \t");
    }

    if (qtd != recursos) {
        erro("a quantidade de recursos da chamada nao confere com commands.txt.");
    }
}

static void ler_comandos(int recursos, int clientes) {
    FILE *arq = fopen("commands.txt", "r");
    char linha[TAM_LINHA];
    int leu = 0;

    if (arq == NULL) {
        erro("nao foi possivel abrir commands.txt.");
    }

    while (fgets(linha, TAM_LINHA, arq) != NULL) {
        if (linha_cheia(linha) && !feof(arq)) {
            erro_arquivo(arq, "commands.txt com formato invalido.");
        }
        tirar_fim(linha);
        if (linha[0] == '\0') {
            erro_arquivo(arq, "commands.txt com formato invalido.");
        }
        conferir_comando(linha, recursos, clientes);
        leu = 1;
    }

    fclose(arq);

    if (!leu) {
        erro("commands.txt com formato invalido.");
    }
}

static void guardar_clientes(void) {
    FILE *arq = fopen("customer.txt", "r");
    char linha[TAM_LINHA];
    int i = 0;

    if (arq == NULL) {
        erro("nao foi possivel abrir customer.txt.");
    }

    while (fgets(linha, TAM_LINHA, arq) != NULL) {
        char *tok;
        int j = 0;

        tirar_fim(linha);
        tok = strtok(linha, ",");
        while (tok != NULL) {
            int valor = 0;

            texto_inteiro(tok, &valor);
            maximum[i][j] = valor;
            need[i][j] = valor;
            j++;
            tok = strtok(NULL, ",");
        }
        i++;
    }

    fclose(arq);
}

static void imprimir_recursos(FILE *saida, const int *valores) {
    int j;

    for (j = 0; j < numero_recursos; j++) {
        fprintf(saida, " %d", valores[j]);
    }
}

static void executar(void) {
    FILE *arq = fopen("commands.txt", "r");
    FILE *saida;
    char linha[TAM_LINHA];

    if (arq == NULL) {
        erro("nao foi possivel abrir commands.txt.");
    }

    saida = fopen("result.txt", "w");
    if (saida == NULL) {
        fclose(arq);
        erro("nao foi possivel criar result.txt.");
    }

    while (fgets(linha, TAM_LINHA, arq) != NULL) {
        char comando[4];
        char *tok;
        int cliente = 0;
        int valores[numero_recursos];
        int j = 0;
        int codigo;

        tirar_fim(linha);
        tok = strtok(linha, " \t");
        strncpy(comando, tok, 3);
        comando[3] = '\0';

        if (strcmp(comando, "*") == 0) {
            mostrar_estado(saida);
            continue;
        }

        tok = strtok(NULL, " \t");
        texto_inteiro(tok, &cliente);
        tok = strtok(NULL, " \t");
        while (tok != NULL && j < numero_recursos) {
            texto_inteiro(tok, &valores[j]);
            j++;
            tok = strtok(NULL, " \t");
        }

        if (strcmp(comando, "RQ") == 0) {
            codigo = request_resources(cliente, valores);
            if (codigo == CONCEDIDO) {
                fprintf(saida, "Allocate to customer %d the resources", cliente);
                imprimir_recursos(saida, valores);
                fprintf(saida, "\n");
            } else if (codigo == PASSOU_DO_MAXIMO) {
                fprintf(saida, "The customer %d request", cliente);
                imprimir_recursos(saida, valores);
                fprintf(saida, " was denied because exceed its maximum allocation\n");
            } else if (codigo == FALTA_RECURSO) {
                fprintf(saida, "The resources");
                imprimir_recursos(saida, available);
                fprintf(saida, " are not enough to customer %d request", cliente);
                imprimir_recursos(saida, valores);
                fprintf(saida, "\n");
            } else {
                fprintf(saida, "The customer %d request", cliente);
                imprimir_recursos(saida, valores);
                fprintf(saida, " was denied because result in an unsafe state\n");
            }
            continue;
        }

        for (j = 0; j < numero_recursos; j++) {
            if (valores[j] > allocation[cliente][j]) {
                fclose(arq);
                fclose(saida);
                liberar_banco();
                erro("o cliente nao tem esses recursos para devolver.");
            }
        }
        release_resources(cliente, valores);
        fprintf(saida, "Release from customer %d the resources", cliente);
        imprimir_recursos(saida, valores);
        fprintf(saida, "\n");
    }

    fclose(arq);
    fclose(saida);
}

int main(int argc, char *argv[]) {
    int recursos;
    int clientes;
    int i;
    int *disponivel;

    if (argc < 2) {
        erro("informe a quantidade de cada recurso na chamada.");
    }

    recursos = argc - 1;
    disponivel = malloc((size_t)recursos * sizeof(int));
    if (disponivel == NULL) {
        erro("sem memoria.");
    }

    for (i = 0; i < recursos; i++) {
        if (!texto_inteiro(argv[i + 1], &disponivel[i])) {
            free(disponivel);
            erro("argumento invalido na chamada.");
        }
    }

    clientes = ler_clientes(recursos);
    ler_comandos(recursos, clientes);
    iniciar_banco(clientes, recursos, disponivel);
    free(disponivel);
    guardar_clientes();
    executar();
    liberar_banco();
    return 0;
}
