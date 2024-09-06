#include "kernel.h"


int fd_memoria;
int fd_cpu_dispatch;
int fd_cpu_interrupt;

t_queue* cola_new;
sem_t mx_cola_new;

t_queue* cola_ready;    
sem_t mx_cola_ready;

t_queue* cola_ready_prioritaria;
sem_t mx_cola_ready_prioritaria;

t_dictionary* dicc_de_ios;
sem_t mx_dicc_de_ios;

t_dictionary* dicc_de_procesos_activos;
sem_t mx_dicc_de_procesos;

bool planificacion_activada;
sem_t mx_planificacion_activada;


int cant_de_recursos;
t_recurso** array_recursos;

int proximo_pid;
int quantum;
int grado_multiprogramacion;
char* algoritmo_planificacion;

int procesos_activos;
sem_t mx_procesos_activos;

t_log* logger;
t_log* debugger;



int main(void) {

    t_config* config;

	config = config_create("./kernel.config");
	if(config == NULL){
		printf("\nConfiguracion no encontrada.\n");
		return 1;
	}

	printf("\nConfiguracion creada.\n\n");


    logger = log_create("./kernel.log", "KERNEL", false, LOG_LEVEL_INFO);
    debugger = log_create("/home/utnso/tp-2024-1c-1/debug.log", "KERNEL", false, LOG_LEVEL_DEBUG);

    
    //Conexion a memoria
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    fd_memoria = crear_conexion(ip_memoria, puerto_memoria); //VAR GLOBAL
    t_handshake handshake_memoria = KE_ME;
    int32_t resultado_handshake_memoria = enviar_handshake(fd_memoria, handshake_memoria);
    if(resultado_handshake_memoria == 1){
        printf("Conectado con memoria!\n");
    }
    else if(resultado_handshake_memoria == 0){
        printf("Memoria me rechazo el handshake.\n");
    }


    //Conexion a CPU
    char* ip_cpu = config_get_string_value(config, "IP_CPU");
    char* puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    char* puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
    
    fd_cpu_dispatch = crear_conexion(ip_cpu, puerto_cpu_dispatch); //VAR GLOBAL
    t_handshake handshake_cpu_dis = KE_CD;
    bool resultado_handshake_dispatch = enviar_handshake(fd_cpu_dispatch, handshake_cpu_dis);
    if(resultado_handshake_dispatch == true){
        printf("Conectado con CPU-Dispatch!\n");
    }
    


    fd_cpu_interrupt = crear_conexion(ip_cpu, puerto_cpu_interrupt); 
    t_handshake handshake_cpu_int = KE_CI;
    bool resultado_handshake_interrupt = enviar_handshake(fd_cpu_interrupt, handshake_cpu_int);
    if(resultado_handshake_interrupt == true){
        printf("Conectado con CPU-Interrupt!\n");

    }
    
    algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");


    //Inicio servidor para escuchar las IO y abro el hilo para manejarlas
    char* puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

    int fd_escucha = iniciar_servidor(puerto_escucha);
    pthread_t tid_io_handler;
    pthread_create(&tid_io_handler, NULL, (void*) io_manager, (void*) &fd_escucha); 

    //Creacion de los recursos
    char** nombre_de_recursos = config_get_array_value(config, "RECURSOS");
    char** cant_de_instancias = config_get_array_value(config, "INSTANCIAS_RECURSOS");
    cant_de_recursos = 0;
    char* nombre_recurso;
    while((nombre_recurso = nombre_de_recursos[cant_de_recursos]) != NULL){
        cant_de_recursos++;
    }
    array_recursos = malloc(sizeof(t_recurso*)*cant_de_recursos);
    for(int i = 0; i < cant_de_recursos; i++){
        t_recurso* recurso_nuevo = malloc(sizeof(t_recurso));
        array_recursos[i] = recurso_nuevo;
        recurso_nuevo->nombre = nombre_de_recursos[i];
        recurso_nuevo->instancias = atoi(cant_de_instancias[i]);
        recurso_nuevo->semaforo = recurso_nuevo->instancias;
        recurso_nuevo->pids_asignados = list_create();
        recurso_nuevo->pcbs_bloqueados = queue_create();
        sem_init(&(recurso_nuevo->mx_modificacion), 0, 1);
    }
    

    //Creo la cola de NEW e inicializo su mutex
    cola_new = queue_create(); 
    sem_init(&mx_cola_new, 0, 1);
    proximo_pid = 0; 
    quantum = config_get_int_value(config, "QUANTUM");

    //creo las colas de READY e inicializo su mutex
    cola_ready = queue_create();
    sem_init(&mx_cola_ready, 0, 1);
    cola_ready_prioritaria = queue_create();
    sem_init(&mx_cola_ready_prioritaria, 0, 1);

    //creo el diccionario de procesos activos con su mutex
    dicc_de_procesos_activos = dictionary_create();
    sem_init(&mx_dicc_de_procesos, 0, 1);

    //creo los hilos para planificar e inicio planificacion_activada, grado multiprog y procesos activos
    planificacion_activada = false;
    grado_multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
    int procesos_activos = 0;
    sem_init(&mx_procesos_activos, 0, 1); 
    sem_init(&mx_planificacion_activada, 0, 1); 


    pthread_t tid_planificador_largo;
    pthread_create(&tid_planificador_largo, NULL, (void *) planificador_largo_plazo, NULL);


    pthread_t tid_planificador_corto;
    int arg_fds[2] = {fd_cpu_dispatch, fd_cpu_interrupt};
    pthread_create(&tid_planificador_corto, NULL, (void *) planificador_corto_plazo, (void*) arg_fds); 


    //Inicio la consola
    
    struct Comando comando;
    char* entrada;
    iniciar_consola(); // -----------------------------------------
    while(1){
        entrada = readline("> "); //leer consola
        if(strcmp(entrada, "")){
            comando = convertir_a_comando(entrada);
            if( comando.codigo == EXIT_CONSOLA){
                free(entrada);
                break;
            }
            ejecutar(comando);
        }
        free(entrada);
    }
    
	return EXIT_SUCCESS;
}

void* io_manager(void* arg){

    int* fd_escucha = (int*) arg;

    //inicializo el diccionario de los io_data y su mutex
    dicc_de_ios = dictionary_create();
    sem_init(&mx_dicc_de_ios, 0, 1); 
    //espero clientes y les creo un hilo para atenderlos
    while(1){
        int* fd_io = malloc(sizeof(int)); // SE LIBERA?
        *fd_io = esperar_cliente(*fd_escucha);
        pthread_t tid_io;
        pthread_create(&tid_io, NULL, (void*) io_handler, (void*) fd_io);
    }
    return NULL;
}

void* io_handler(void* arg){
    int* fd_io = (int*) arg;
    //recibir handshake y verificar ok
    t_handshake handshake_io = IO_KE;
    bool resultado_handshake_io = esperar_handshake(fd_io, handshake_io);
    //recibir nombre y tipo
    t_paquete* id_io_empaquetada = malloc(sizeof(t_paquete)); // SE LIBERA?
    recv_paquete(fd_io, id_io_empaquetada);
    t_identificacion_io* id_io = malloc(sizeof(t_identificacion_io)); // SE LIBERA?
    deserializar_identificacion_io(id_io, id_io_empaquetada);

    //crear estructura de io
    t_io_data* io_data = malloc(sizeof(io_data));
    io_data->io_nombre = id_io->nombre;
    io_data->io_tipo = id_io->tipo;

    t_queue* cola_block = queue_create(); 
    sem_t* mx_cola_block = malloc(sizeof(sem_t)); // SE LIBERA?
    sem_init(mx_cola_block, 0, 1);

    io_data->cola_block = cola_block;
    io_data->mx_cola_block = mx_cola_block;


    sem_wait(&mx_dicc_de_ios);
    if( dictionary_has_key(dicc_de_ios,id_io->nombre) ){
        //ya existe la io con ese nombre
        sem_post(&mx_dicc_de_ios);
        return NULL;
    }
    dictionary_put(dicc_de_ios, id_io->nombre, io_data);
    sem_post(&mx_dicc_de_ios);

    while(1){
        //chequear si hay procesos en block (SOLO LECTURA)
        bool cola_esta_vacia = queue_is_empty(io_data->cola_block);

        if(!cola_esta_vacia){
            sem_wait(mx_cola_block);
            t_pcb_bloqueado* pcb_bloqueado = queue_pop(io_data->cola_block);
            sem_post(mx_cola_block);
            //enviar peticion a io
            t_paquete* peticion_paquete = malloc(sizeof(t_paquete));
            if(!strcmp(pcb_bloqueado->datos_peticion[1], "IO_GEN_SLEEP")){
                t_peticion_gen_sleep* peticion_gen_sleep = malloc(sizeof(t_peticion_gen_sleep)); 
                peticion_gen_sleep->pid = pcb_bloqueado->pcb->pid;
                peticion_gen_sleep->unidades_de_sleep = atoi(pcb_bloqueado->datos_peticion[2]);
                serializar_peticion_gen_sleep(peticion_gen_sleep, peticion_paquete);
            }

            send_paquete(fd_io, peticion_paquete);
            //esperar respuesta de io
            t_paquete* respuesta_empaquetada = malloc(sizeof(t_paquete)); // SE LIBERA?
            recv_paquete(fd_io, respuesta_empaquetada);
            t_respuesta_gen_sleep* rta = malloc(sizeof(t_respuesta_gen_sleep)); // SE LIBERA?
            deserializar_respuesta_gen_sleep(rta, respuesta_empaquetada);

            //una vez recibida la respuesta de io poner el proceso en ready SI ES QUE LA PLANIFICACION ESTA ACTIVADA???, sino esperar a que se reactive
            while(!planificacion_activada){/* NADA */}

            if(!strcmp(algoritmo_planificacion, "VRR")){
                log_info(logger, "PID: %d - Estado Anterior: BLOQUEADO - Estado Actual: LISTO", pcb_bloqueado->pcb->pid);
                mover_a_ready_prioritaria(pcb_bloqueado->pcb);
                logear_cola_ready_prioritaria();
            }else{
                log_info(logger, "PID: %d - Estado Anterior: BLOQUEADO - Estado Actual: LISTO", pcb_bloqueado->pcb->pid);
                mover_a_ready(pcb_bloqueado->pcb);
                logear_cola_ready();
            }
        }
    }
    return NULL;
}