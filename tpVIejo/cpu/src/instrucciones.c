#include "instrucciones.h"

void ejecutar_exit(int* fd_kernel, t_pcb* pcb){
        t_desalojo* desalojo = malloc(sizeof(t_desalojo));

        desalojo->motivo = FINALIZACION_NORMAL;
        desalojo->pcb = pcb;
        desalojo->cant_datos = 0;
        desalojo->datos_adicionales = NULL;

        t_paquete* desalojo_empaquetado = malloc(sizeof(t_paquete));
        serializar_desalojo(desalojo, desalojo_empaquetado);
        send_paquete(fd_kernel, desalojo_empaquetado);
}

void ejecutar_io_gen_sleep(int* fd_kernel, t_pcb* pcb, t_instruccion* instruccion){
        t_desalojo* desalojo = malloc(sizeof(t_desalojo));

        desalojo->motivo = DESALOJO_POR_IO;
        desalojo->pcb = pcb;
        desalojo->cant_datos = 3;
        desalojo->datos_adicionales = malloc(3 * sizeof(char*));

        desalojo->datos_adicionales[0] = malloc(strlen((char *) list_get(instruccion->argumentos, 0)) + 1);
        strcpy(desalojo->datos_adicionales[0], (char *) list_get(instruccion->argumentos, 0)); //interfaz

        desalojo->datos_adicionales[1] = malloc(strlen("IO_GEN_SLEEP") + 1);
        strcpy(desalojo->datos_adicionales[1], "IO_GEN_SLEEP"); //tipo de pedido

        desalojo->datos_adicionales[2] = malloc(strlen((char *) list_get(instruccion->argumentos, 1)) + 1);
        strcpy(desalojo->datos_adicionales[2], (char *) list_get(instruccion->argumentos, 1)); //unidades de trabajo

        
        t_paquete* desalojo_empaquetado = malloc(sizeof(t_paquete));
        serializar_desalojo(desalojo, desalojo_empaquetado);
        
        send_paquete(fd_kernel, desalojo_empaquetado);
        
}

void ejecutar_sum(t_registros* registros_cpu, t_list* argumentos){
        char* registro_destino = list_get(argumentos, 0); // ARGUMENTO 1
        char* registro_origen = list_get(argumentos, 1); // ARGUMENTO 2

        int valor_origen = obtener_valor_registro(registro_origen, registros_cpu);
        int valor_destino = obtener_valor_registro(registro_destino, registros_cpu);

        cambiar_valor_registro(registro_destino, registros_cpu, valor_destino + valor_origen);
        log_debug(debugger, "Sumados los registros %s y %s con resultado %d", registro_destino, registro_origen, obtener_valor_registro(registro_destino, registros_cpu));
}

void ejecutar_set(t_registros* registros_cpu, t_list* argumentos){
        char* registro = list_get(argumentos, 0); // ARGUMENTO 1
        int valor = atoi(list_get(argumentos, 1)); // ARGUMENTO 2

        cambiar_valor_registro(registro, registros_cpu, valor);
        
}

void ejecutar_sub(t_registros* registros_cpu, t_list* argumentos){
        char* registro_destino = list_get(argumentos, 0); // ARGUMENTO 1
        char* registro_origen = list_get(argumentos, 1); // ARGUMENTO 2

        int valor_origen = obtener_valor_registro(registro_origen, registros_cpu);
        int valor_destino = obtener_valor_registro(registro_destino, registros_cpu);

        cambiar_valor_registro(registro_destino, registros_cpu, valor_destino - valor_origen);
        
}

void ejecutar_jnz(t_registros* registros_cpu, t_list* argumentos){
        char* registro = list_get(argumentos, 0); // ARGUMENTO 1
        int valor = atoi(list_get(argumentos, 1)); // ARGUMENTO 2

        if(obtener_valor_registro(registro, registros_cpu) != 0){
                registros_cpu->PC = valor;
        }      
}

void ejecutar_wait(int* fd_kernel, t_pcb* pcb, t_list* argumentos){
        t_desalojo* desalojo = malloc(sizeof(t_desalojo));

        desalojo->pcb = pcb;
        desalojo->motivo = SOLICITUD_WAIT;
        desalojo->cant_datos = 1;

        char* recurso = list_get(argumentos, 0);
        desalojo->datos_adicionales = malloc(sizeof(char*));
        desalojo->datos_adicionales[0] = recurso;

        t_paquete* desalojo_empaquetado = malloc(sizeof(t_paquete));
        serializar_desalojo(desalojo, desalojo_empaquetado);
        
        send_paquete(fd_kernel, desalojo_empaquetado);
}

void ejecutar_signal(int* fd_kernel, t_pcb* pcb, t_list* argumentos){
        t_desalojo* desalojo = malloc(sizeof(t_desalojo));

        desalojo->pcb = pcb;
        desalojo->motivo = SOLICITUD_SIGNAL;
        desalojo->cant_datos = 1;

        char* recurso = list_get(argumentos, 0);
        desalojo->datos_adicionales = malloc(sizeof(char*));
        desalojo->datos_adicionales[0] = recurso;

        t_paquete* desalojo_empaquetado = malloc(sizeof(t_paquete));
        serializar_desalojo(desalojo, desalojo_empaquetado);
        
        send_paquete(fd_kernel, desalojo_empaquetado);
}


int obtener_valor_registro(char* nombre_registro, t_registros* registros_cpu){
        if(!strcmp(nombre_registro, "PC")){
                return registros_cpu->PC;
        }
        else if(!strcmp(nombre_registro, "AX")){
                return registros_cpu->AX;
        }
        else if(!strcmp(nombre_registro, "BX")){
                return registros_cpu->BX;
        }
        else if(!strcmp(nombre_registro, "CX")){
                return registros_cpu->CX;
        }
        else if(!strcmp(nombre_registro, "DX")){
                return registros_cpu->DX;
        }
        else if(!strcmp(nombre_registro, "EAX")){
                return registros_cpu->EAX;
        }
        else if(!strcmp(nombre_registro, "EBX")){
                return registros_cpu->EBX;
        }
        else if(!strcmp(nombre_registro, "ECX")){
                return registros_cpu->ECX;
        }
        else if(!strcmp(nombre_registro, "EDX")){
                return registros_cpu->EDX;
        }
        else if(!strcmp(nombre_registro, "SI")){
                return registros_cpu->SI;
        }
        else if(!strcmp(nombre_registro, "DI")){
                return registros_cpu->DI;
        }
        return 0;
}

void cambiar_valor_registro(char* nombre_registro, t_registros* registros_cpu, int valor){
        if(!strcmp(nombre_registro, "PC")){
                registros_cpu->PC = valor;
        }
        else if(!strcmp(nombre_registro, "AX")){
                registros_cpu->AX = valor;
        }
        else if(!strcmp(nombre_registro, "BX")){
                registros_cpu->BX = valor;
        }
        else if(!strcmp(nombre_registro, "CX")){
                registros_cpu->CX = valor;
        }
        else if(!strcmp(nombre_registro, "DX")){
                registros_cpu->DX = valor;
        }
        else if(!strcmp(nombre_registro, "EAX")){
                registros_cpu->EAX = valor;
        }
        else if(!strcmp(nombre_registro, "EBX")){
                registros_cpu->EBX = valor;
        }
        else if(!strcmp(nombre_registro, "ECX")){
                registros_cpu->ECX = valor;
        }
        else if(!strcmp(nombre_registro, "EDX")){
                registros_cpu->EDX = valor;
        }
        else if(!strcmp(nombre_registro, "SI")){
                registros_cpu->SI = valor;
        }
        else if(!strcmp(nombre_registro, "DI")){
                registros_cpu->DI = valor;
        }
}
