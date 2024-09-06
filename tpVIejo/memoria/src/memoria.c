#include "memoria.h"
#define MAX_LINEA 100

t_dictionary* diccionario_pids;
sem_t mx_diccionario_pids;
t_log* debugger;

int main(void) {


    debugger = log_create("/home/utnso/tp-2024-1c-1/debug.log", "MEMORIA", false, LOG_LEVEL_DEBUG);

    t_config* config;

    config = config_create("./memoria.config");
    if (config == NULL){
        printf("\nConfiguracion no encontrada.\n");
    }
    printf("\nConfiguracion creada.\n");

    //Inicializo el diccionario de pids y su mx(solo validos hasta que este desarrollada bien la memoria)
    diccionario_pids = dictionary_create(); //NO LIBERADO
    sem_init(&mx_diccionario_pids, 0, 1);

    //Iniciar servidor
    char* puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    int fd_escucha = iniciar_servidor(puerto_escucha); // ??????????????

    //Establecer conexion con la CPU
    int* fd_cpu = malloc(sizeof(int)); // SE LIBERA? Porque esta el malloc?
    *fd_cpu = esperar_cliente(fd_escucha); // ??????????????
    pthread_t tid_cpu;
    pthread_create(&tid_cpu, NULL, (void* ) atender_cpu, (void*) fd_cpu); // ??????????????
    //pthread_detach(tid_cpu);
    
    //Establecer conexion con el KERNEL
    int* fd_kernel = malloc(sizeof(int)); // SE LIBERA? Porque esta el malloc?
    *fd_kernel = esperar_cliente(fd_escucha); // ??????????????
    printf("Recibi a kernel\n");
    pthread_t tid_kernel;
    pthread_create(&tid_kernel, NULL, (void* ) atender_kernel, (void*) fd_kernel); // ??????????????
    //pthread_detach(tid_kernel);
   
    while(1){ /*no hagas mas nada*/ }

    
	return EXIT_SUCCESS;
}

void* atender_cpu(void* arg){
    int* fd_cpu = (int*) arg;
    t_handshake handshake_cpu = CP_ME;
    bool resultado_handshake = esperar_handshake(fd_cpu, handshake_cpu);
    if(resultado_handshake == true){
        printf("Se conecto la cpu.\n");
    }
    
    while(1){
        //recibir pedido y desempaquetarlo
        t_paquete* pedido_empaquetado = malloc(sizeof(t_paquete)); // SE LIBERA?
        recv_paquete(fd_cpu, pedido_empaquetado);
        t_pedir_instruccion* pedido_de_cpu = malloc(sizeof(t_pedir_instruccion));
        deserializar_pedido(pedido_de_cpu, pedido_empaquetado);
        int pid = pedido_de_cpu->pid;
        int pc = pedido_de_cpu->pc;

        //buscar instruccion
        t_list* instrucciones_proceso_actual = dictionary_get(diccionario_pids, string_itoa(pid));
        t_instruccion* instruccion = list_get(instrucciones_proceso_actual, pc);

        //empaqueto y mando
        t_paquete* paquete_a_mandar = malloc(sizeof(t_paquete));
        serializar_instruccion(instruccion, paquete_a_mandar);
        send_paquete(fd_cpu, paquete_a_mandar);

    }
    return NULL;
}

void* atender_kernel(void* arg){
    int* fd_kernel = (int*) arg;
    t_handshake handshake_kernel = KE_ME;
    bool resultado_handshake = esperar_handshake(fd_kernel, handshake_kernel);
    if(resultado_handshake == 1){
        printf("Se conecto el kernel.\n");
    }
    //esperar del kernel a que mande algo: -> o un nuevo proceso para iniciar o un proceso para borrar
    while(1){
        t_paquete* paquete_recibido = malloc(sizeof(t_paquete)); // SE LIBERA?
        recv_paquete(fd_kernel, paquete_recibido);
        log_debug(debugger, "Recibi paquete del KERNEL");
        op_code codigo_de_operacion = paquete_recibido->codigo_operacion;
        switch(codigo_de_operacion){
            case NUEVO_PROCESO:
                bool resultado_de_guardar;
                int err = guardar_nuevo_proceso(paquete_recibido); //devuelve 0 si sale bien
                resultado_de_guardar = (err == 0); //si error es 0 da true -> salio todo bien
                send(*fd_kernel, &resultado_de_guardar, sizeof(bool), 0);
                break;
            default:
                //PAQUETE NO CORRECTO
                break;
        }
    }
    return NULL;
}

int guardar_nuevo_proceso(t_paquete* nuevo_proceso_empaquetado){
    t_nuevo_proceso* np = malloc(sizeof(t_nuevo_proceso)); //NO ESTA LIBERADO
    FILE* archivo_pseudocodigo;
    char* linea_leida = malloc(sizeof(char) * MAX_LINEA);
    deserializar_nuevo_proceso(np, nuevo_proceso_empaquetado); 
    
    log_debug(debugger, "Abriendo el path %s", np->path);
    archivo_pseudocodigo = fopen((const char*) np->path, "r");
    if(archivo_pseudocodigo == NULL){
        //SI EL ARCHIVO NO EXISTE
        return 1;
    }
    t_list* lista_de_instrucciones = list_create(); 
    while( fgets(linea_leida, MAX_LINEA, archivo_pseudocodigo) != NULL ){
        t_instruccion* instruccion_leida = str_to_instruccion(linea_leida);
        list_add(lista_de_instrucciones, instruccion_leida);
    }

    sem_wait(&mx_diccionario_pids);
    dictionary_put(diccionario_pids, string_itoa(np->pid), lista_de_instrucciones);
    sem_post(&mx_diccionario_pids);

    free_nuevo_proceso(np);
    free(linea_leida);
    fclose(archivo_pseudocodigo);
    return 0;
}


inst_code str_to_inst_code(char* string){
    char* vector[19] = {
        "SET",
        "MOV_IN",  
        "MOV_OUT",  
        "SUM",
        "SUB",
        "JNZ",
        "RESIZE",  
        "COPY_STRING",  
        "WAIT",  
        "SIGNAL",  
        "IO_GEN_SLEEP",
        "IO_STDIN_READ",  
        "IO_STDOUT_WRITE",  
        "IO_FS_DELETE",  
        "IO_FS_CREATE",  
        "IO_FS_TRUNCATE",  
        "IO_FS_WRITE",  
        "IO_FS_READ", 
        "EXIT",
    };
    for(int i = 0; i < 19; i++){
        if(!strcmp(string, vector[i])){
            return i;
        }
    }
    return INST_ERROR;

}

t_instruccion* str_to_instruccion(char* string){
    t_instruccion* instruccion = malloc(sizeof(t_instruccion));
    char** strtok_save_ptr = malloc(sizeof(char*));
    char* codigo_instruccion_str = strtok_r(string, " ", strtok_save_ptr);
    log_debug(debugger, "Codigo de instruccion obtenido: %s", codigo_instruccion_str);
    instruccion->codigo_instruccion = str_to_inst_code(codigo_instruccion_str);
    instruccion->argumentos = list_create();
    char* token = NULL;
    while( ( token = strtok_r(NULL, " ", strtok_save_ptr) ) != NULL){
        log_debug(debugger, "Argumento: %s", token);
        if( string_ends_with( token, "\n" ) ){
            token = string_replace( token, "\n", "");
        }
        int tamanio = (strlen(token) + 1)* sizeof(char);
        char* argumento = malloc(tamanio);
        memcpy(argumento, token, tamanio);
        list_add(instruccion->argumentos, argumento);
    }
    free(strtok_save_ptr);
    return instruccion;
}
