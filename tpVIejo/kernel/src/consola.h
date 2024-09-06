#ifndef CONSOLA_H_
#define CONSOLA_H_

#include<stdio.h>
#include<readline/readline.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<comandos.h>

enum comando_code{
    NO_VALIDO,
    EXIT_CONSOLA,
    INICIAR_PLANIFICACION,
    INICIAR_PROCESO,
    EJECUTAR_SCRIPT,
    FINALIZAR_PROCESO,
    DETENER_PLANIFICACION,
    MULTIPROGRAMACION,
    PROCESO_ESTADO,
};

struct Comando{
    enum comando_code codigo;
    char* argumento;
};

void iniciar_consola(void);
char* leer_consola(void);
struct Comando convertir_a_comando(char*);
void ejecutar(struct Comando);
enum comando_code char_a_codigo_comando(char*);
void caso_no_argumentos(struct Comando* comando, char* codigo, char* arg);
void caso_un_argumento(struct Comando* comando, char* codigo, char* arg, char* resto);

//borrar cuando esten en donde correspondan
void ejecutar_script(char*);
void finalizar_proceso(int pid);
void detener_planificacion(void);
void multiprogramacion(int valor);

#endif 