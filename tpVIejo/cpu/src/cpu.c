#include "cpu.h"

bool hay_interrupcion;
sem_t mx_hay_interrupcion;
t_registros* registros_cpu;
t_log* logger;
t_log* debugger;//

int main(void) {
    
    
    t_config* config;
    
    config = config_create("./cpu.config");
    if (config == NULL){
        printf("\nConfiguracion no encontrada.\n");
        return -1;
    }
    printf("\nConfiguracion creada.\n\n");

    logger = log_create("cpu.log", "CPU", false, LOG_LEVEL_INFO);
    debugger = log_create("/home/utnso/tp-2024-1c-1/debug.log", "CPU", false, LOG_LEVEL_DEBUG);
    

    //Conexion a memoria
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    int fd_memoria = crear_conexion(ip_memoria, puerto_memoria); // ??????????????
    t_handshake handshake_memoria = CP_ME;
    int resultado_handshake_memoria = enviar_handshake(fd_memoria, handshake_memoria); // ??????????????
    if(resultado_handshake_memoria == 1){
        printf("Conectado con memoria!\n");
    }

    //Crear servidores
    char* puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    int fd_escucha_dispatch = iniciar_servidor(puerto_escucha_dispatch); // ??????????????

    char* puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    int fd_escucha_interrupt = iniciar_servidor(puerto_escucha_interrupt); // ??????????????
 

    //Recibir conexion de KERNEL
    int* fd_kernel_dispatch = malloc(sizeof(int)); // SE LIBERA?
    *fd_kernel_dispatch = esperar_cliente(fd_escucha_dispatch); // ??????????????
    t_handshake handshake_kernel_disp = KE_CD;
    bool resultado_kernel_disp = esperar_handshake(fd_kernel_dispatch, handshake_kernel_disp); // ??????????????
    if(resultado_kernel_disp == true){
        printf("Se conecto el kernel con dispatch.\n");
    }

    int* fd_kernel_interrupt = malloc(sizeof(int)); // SE LIBERA?
    *fd_kernel_interrupt = esperar_cliente(fd_escucha_interrupt); // ??????????????
    t_handshake handshake_kernel_int = KE_CI;
    bool resultado_kernel_int = esperar_handshake(fd_kernel_interrupt, handshake_kernel_int); // ??????????????
    if(resultado_kernel_int == true){
        printf("Se conecto el kernel con interrupt.\n");
    }

    
    //Inicio el receptor de interrupcions, su variable y el mx
    pthread_t tid_interrupt;
    hay_interrupcion = false;
    sem_init(&mx_hay_interrupcion, 0, 1);
    pthread_create(&tid_interrupt, NULL, (void *) receptor_de_interrupciones, (void*) fd_kernel_interrupt);

    while(1){
        //RECIBIR PAQUETE CON EL PCB DESDE EL KERNEL Y DESERIALIZARLO
        t_paquete* pcb_empaquetado = malloc(sizeof(t_paquete)); // SE LIBERA?
        
        recv_paquete(fd_kernel_dispatch, pcb_empaquetado);
        
        t_pcb* pcb_recibido = malloc(sizeof(pcb_recibido));// SE LIBERA?
        deserializar_pcb(pcb_recibido, pcb_empaquetado);
        //ACTUALIZAR CONTEXTO DE EJECUCION DE LA CPU (REGISTROS, PC, ...)
        registros_cpu = pcb_recibido->registros_cpu;

        bool exit_ciclo = false;
        //ETAPA FETCH-> CON EL PC Y EL PID PEDIR A MEMORIA LA PROXIMA INSTRUCCION Y SUMAR 1 AL PC
        while(!exit_ciclo){
            t_pedir_instruccion* pedido = malloc(sizeof(t_pedir_instruccion));
            pedido->pid = pcb_recibido->pid;
            pedido->pc = registros_cpu->PC;
            log_info(logger, "PID: %d - FETCH - Program Counter: %d", pedido->pid, pedido->pc);
            registros_cpu->PC++;
            
            t_paquete* pedido_empaquetado = malloc(sizeof(t_paquete));
            serializar_pedido(pedido, pedido_empaquetado); 
            send_paquete(&fd_memoria, pedido_empaquetado);
            log_debug(debugger, "Mande pedido a Memoria");

            t_paquete* instruccion_empaquetada = malloc(sizeof(t_paquete));
            recv_paquete(&fd_memoria, instruccion_empaquetada);
            log_debug(debugger, "Recibi instruccion de Memoria");
            t_instruccion* instruccion_a_ejecutar = malloc(sizeof(t_instruccion));
            deserializar_instruccion(instruccion_a_ejecutar, instruccion_empaquetada); 
            
            //ETAPA DECODE/EXECUTE-> A PARTIR DE LA INSTRUCCION RECIBIDA METER UN SWITCH CON EL INST_CODE Y EJECUTAR LA FUNCION CORRESPONDIENTE
            
            switch(instruccion_a_ejecutar->codigo_instruccion){
                case SET: 
                    log_info(logger, "PID: %d - Ejecutando: SET - %s %s", pedido->pid, (char *) list_get(instruccion_a_ejecutar->argumentos, 0), 
                                                                                        (char *) list_get(instruccion_a_ejecutar->argumentos, 1));
                    ejecutar_set(registros_cpu, instruccion_a_ejecutar->argumentos);
                break;
                case MOV_IN: 
                    // To Do
                break;
                case MOV_OUT: 
                    // To Do
                break;
                case SUM:
                    log_info(logger, "PID: %d - Ejecutando: SUM - %s %s", pedido->pid, (char *) list_get(instruccion_a_ejecutar->argumentos, 0), 
                                                                                            (char *) list_get(instruccion_a_ejecutar->argumentos, 1));
                    ejecutar_sum(registros_cpu, instruccion_a_ejecutar->argumentos);
                break;
                case SUB:
                    log_info(logger, "PID: %d - Ejecutando: SUB - %s %s", pedido->pid, (char *) list_get(instruccion_a_ejecutar->argumentos, 0), 
                                                                                            (char *) list_get(instruccion_a_ejecutar->argumentos, 1)); 
                    ejecutar_sub(registros_cpu, instruccion_a_ejecutar->argumentos);
                break;
                case JNZ:
                    log_info(logger, "PID: %d - Ejecutando: JNZ - %s %s", pedido->pid, (char *) list_get(instruccion_a_ejecutar->argumentos, 0), 
                                                                                            (char *) list_get(instruccion_a_ejecutar->argumentos, 1)); 
                    ejecutar_jnz(registros_cpu, instruccion_a_ejecutar->argumentos);
                break;
                case RESIZE: 
                    // To Do
                break;
                case COPY_STRING: 
                    // To Do
                break;
                case WAIT:
                    log_info(logger, "PID: %d - Ejecutando: WAIT - %s", pedido->pid, (char *) list_get(instruccion_a_ejecutar->argumentos, 0));  
                    ejecutar_wait(fd_kernel_dispatch, pcb_recibido, instruccion_a_ejecutar->argumentos);
                    exit_ciclo = true;
                break;
                case SIGNAL: 
                    log_info(logger, "PID: %d - Ejecutando: SIGNAL - %s", pedido->pid, (char *) list_get(instruccion_a_ejecutar->argumentos, 0));  
                    ejecutar_signal(fd_kernel_dispatch, pcb_recibido, instruccion_a_ejecutar->argumentos);
                    exit_ciclo = true;
                break;
                case IO_GEN_SLEEP:
                    log_info(logger, "PID: %d - Ejecutando: IO_GEN_SLEEP - %s %s", pedido->pid, (char *) list_get(instruccion_a_ejecutar->argumentos, 0), 
                                                                                            (char *) list_get(instruccion_a_ejecutar->argumentos, 1));
                    ejecutar_io_gen_sleep(fd_kernel_dispatch, pcb_recibido, instruccion_a_ejecutar);
                    exit_ciclo = true;
                break;
                case IO_STDIN_READ: 
                    // To Do
                break;
                case IO_STDOUT_WRITE: 
                    // To Do
                break;
                case IO_FS_DELETE: 
                    // To Do
                break;
                case IO_FS_CREATE: 
                    // To Do
                break;
                case IO_FS_TRUNCATE: 
                    // To Do
                break;
                case IO_FS_WRITE: 
                    // To Do
                break;
                case IO_FS_READ: 
                    // To Do
                break;
                case EXIT:
                    log_info(logger, "PID: %d - Ejecutando: EXIT", pedido->pid);
                    ejecutar_exit(fd_kernel_dispatch , pcb_recibido);
                    exit_ciclo = true;
                break;
                case INST_ERROR:
                    // To Do
                break;
            }
            
            //ETAPA CHECK INTERRUPT-> CHEQUEAR SI HAY_INTERRUPCION ES TRUE, EN CASO DE QUE NO NO HACER NADA Y SEGUIR, EN CASO DE QUE SI SETEARLA DE VUELTA EN FALSE Y 
            //HACER LO QUE HAYA QUE HACER PARA DESALOJAR EL PCB 
            if(hay_interrupcion && !exit_ciclo){
                sem_wait(&mx_hay_interrupcion);
                hay_interrupcion = false;
                sem_post(&mx_hay_interrupcion);
                t_desalojo* desalojo_por_int = malloc(sizeof(t_desalojo));
                t_paquete* desalojo_empaquetado = malloc(sizeof(t_paquete));
                desalojo_por_int->pcb = pcb_recibido;
                desalojo_por_int->motivo = FIN_DE_QUANTUM;
                desalojo_por_int->cant_datos = 0;
                desalojo_por_int->datos_adicionales = NULL;
                serializar_desalojo(desalojo_por_int, desalojo_empaquetado);
                send_paquete(fd_kernel_dispatch, desalojo_empaquetado);
                free_desalojo(desalojo_por_int);
                exit_ciclo = true;
            }
        }


    }
	return EXIT_SUCCESS;
}

void* receptor_de_interrupciones(void* arg){
    int* fd_kernel_interrupt = (int*) arg;
    while(1){
        int interrupcion_alert;
        recv(*fd_kernel_interrupt, &interrupcion_alert, sizeof(int), MSG_WAITALL);
        log_debug(debugger, "Recibi una interrupcion por quantum");
        sem_wait(&mx_hay_interrupcion);
        hay_interrupcion = true;
        sem_post(&mx_hay_interrupcion);
    }
    return NULL;
}
