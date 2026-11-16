#ifndef BANQUEIRO_H
#define BANQUEIRO_H

#include <stdio.h>

extern int *available;
extern int **maximum;
extern int **allocation;
extern int **need;
extern int numero_clientes;
extern int numero_recursos;

#define CONCEDIDO 0
#define PASSOU_DO_MAXIMO 1
#define FALTA_RECURSO 2
#define INSEGURO 3

void iniciar_banco(int clientes, int recursos, const int disponivel[]);
void liberar_banco(void);
void mostrar_estado(FILE *saida);
int estado_seguro(void);
int request_resources(int cliente, int pedido[]);
void release_resources(int cliente, int liberacao[]);

#endif
