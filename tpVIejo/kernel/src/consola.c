#include "consola.h"

void iniciar_consola(void){
    printf("\nHola! Bienvenido a la consola\n");
    return;
}

char* leer_consola(void){
    char* input = readline("> "); 
    return input;
}

struct Comando convertir_a_comando(char* entrada){
    struct Comando comando;
    char** save_ptr_strtok = malloc(sizeof(char*));

    char* code = strtok_r(entrada, " ", save_ptr_strtok);
    char* arg = strtok_r(NULL, " ", save_ptr_strtok);
    char* resto = strtok_r(NULL, " ", save_ptr_strtok);
    free(save_ptr_strtok);
    comando.codigo = char_a_codigo_comando(code);

    switch(comando.codigo){
        case NO_VALIDO:
            comando.argumento = strcat(code, ": no se reconoce el comando.");
            break;
        case INICIAR_PLANIFICACION:
            caso_no_argumentos(&comando, code, arg);
            break;
        case INICIAR_PROCESO:
            caso_un_argumento(&comando, code, arg, resto);
            break;
        case EJECUTAR_SCRIPT:
            caso_un_argumento(&comando, code, arg, resto);
            break;
        case FINALIZAR_PROCESO:
            caso_un_argumento(&comando, code, arg, resto);
            break;
        case DETENER_PLANIFICACION:
            caso_no_argumentos(&comando, code, arg);
            break;
        case MULTIPROGRAMACION:
            caso_un_argumento(&comando, code, arg, resto);
            break;
        case PROCESO_ESTADO:
            caso_no_argumentos(&comando, code, arg);
            break;
        case EXIT_CONSOLA:
            caso_no_argumentos(&comando, code, arg);
            break;
    }

    return comando;

}

enum comando_code char_a_codigo_comando(char* str){
    char* vector[9] = {
        "NO_VALIDO",
        "EXIT",
        "INICIAR_PLANIFICACION",
        "INICIAR_PROCESO",
        "EJECUTAR_SCRIPT",
        "FINALIZAR_PROCESO",
        "DETENER_PLANIFICACION",
        "MULTIPROGRAMACION",
        "PROCESO_ESTADO"
    };
    enum comando_code respuesta;
    for(int i = 1; i < 9; i++){
        if( !strcmp(vector[i], str) ){
            respuesta = i;
            return respuesta;
        }
    }
    respuesta = NO_VALIDO;
    return respuesta;
    
}

void ejecutar(struct Comando comando){
    switch(comando.codigo){
        case NO_VALIDO:
            //printf("%s\n", comando.argumento);
            break;
        case INICIAR_PROCESO:
            iniciar_proceso(comando.argumento);
            break;
        case INICIAR_PLANIFICACION:
            iniciar_planificacion();
            break;
        case EJECUTAR_SCRIPT:
            break;
        case FINALIZAR_PROCESO:
            break;
        case DETENER_PLANIFICACION:
            detener_planificacion();
            break;
        case MULTIPROGRAMACION:
            cambiar_grado_multip(comando.argumento);
            break;
        case PROCESO_ESTADO:
            listar_procesos_por_estado();
            break;
        default:
            break;
    }
}

void caso_no_argumentos(struct Comando* comando, char* codigo, char* arg){
    if(arg == NULL){
        comando->argumento = NULL;
    }
    else{
        comando->codigo = NO_VALIDO;
        comando->argumento = strcat(codigo, ": el comando no lleva argumentos.");
    }
    return;
}

void caso_un_argumento(struct Comando* comando, char* codigo, char* arg, char* resto){
    if(arg == NULL || resto != NULL){
        comando->codigo = NO_VALIDO;
        comando->argumento = strcat(codigo, ": el comando lleva 1 (un) argumento.");
    }
    else{
        comando->argumento = arg;
    }
    return;
}

