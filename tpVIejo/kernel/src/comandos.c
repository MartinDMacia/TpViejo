#include "comandos.h"

t_temporal* clock_rr;
bool interrup_activada;

void iniciar_proceso(char* path){
    //mandar el pid y el path a memoria y recibir ok o error
    t_nuevo_proceso* nuevo_proceso = malloc(sizeof(t_nuevo_proceso));
    nuevo_proceso->pid = asignar_pid();
    nuevo_proceso->path = path;
    t_paquete* nuevo_proceso_empaquetado = malloc(sizeof(t_paquete));
    serializar_nuevo_proceso(nuevo_proceso, nuevo_proceso_empaquetado); //crea el paquete y lo guarda en la direc apuntada por nuevo_proceso_empaquetado
    send_paquete(&fd_memoria, nuevo_proceso_empaquetado); //esta funcion tiene que mandar bien el paquete y (liberar la memoria del paquete)????

    bool resultado_creacion;
    recv(fd_memoria, &resultado_creacion, sizeof(bool), MSG_WAITALL); //la memoria me manda true si salio bien o false si salio mal
    
    //si resultado_creacion es ok creo que el pcb y lo pusheo a new
    if(resultado_creacion == true){
        t_pcb* pcb_nuevo = crear_pcb(nuevo_proceso->pid); //crea el pcb y lo iniciliza
        pcb_nuevo->estado = NUEVO;
        sem_wait(&mx_cola_new);
        queue_push(cola_new, pcb_nuevo);
        sem_post(&mx_cola_new);
        log_info(logger, "Se crea el proceso %d en NEW", nuevo_proceso->pid); 

        sem_wait(&mx_dicc_de_procesos);
        t_proceso_admin* nodo_nuevo = malloc(sizeof(t_proceso_admin));
        nodo_nuevo->estado = NUEVO;
        nodo_nuevo->cola_bloqueante = NULL;
        nodo_nuevo->mx_cola = NULL;
        dictionary_put(dicc_de_procesos_activos, string_itoa(pcb_nuevo->pid), nodo_nuevo);
        sem_post(&mx_dicc_de_procesos);
        
    }
    else{ log_info(logger, "No se pudo crear el proceso con path %s", path); }
    //Si resultado_creacion  no es ok??

    free_nuevo_proceso(nuevo_proceso);
}


int asignar_pid(){
    int pid = proximo_pid; //VAR GLOBAL
    proximo_pid++; //VAR GLOBAL
    return pid;
}

t_pcb* crear_pcb(int pid){
    t_pcb* nuevo_pcb = malloc(sizeof(t_pcb));
    nuevo_pcb->pid = pid;
    nuevo_pcb->quantum = !strcmp(algoritmo_planificacion, "FIFO")? -1 : quantum;
    nuevo_pcb->registros_cpu = crear_registros_en_cero();
    return nuevo_pcb;

} 

void listar_procesos_por_estado(){
    t_estado estados[5] = {NUEVO, LISTO, EJECUTANDO, BLOQUEADO, FINALIZADO};
    sem_wait(&mx_dicc_de_procesos);
    for(int i = 0; i < 5; i++){
        printf("\nEstado: %s\n", estado_to_str(estados[i]));
        imprimir_procesos_del_estado(estados[i]);
    }
    printf("\n");
    sem_post(&mx_dicc_de_procesos);
}

void imprimir_procesos_del_estado(t_estado estado){
    t_list* keys = dictionary_keys(dicc_de_procesos_activos);
    for(int i = 0; i < list_size(keys); i++){
        char* key = list_get(keys, i);
        if( ((t_proceso_admin*) dictionary_get(dicc_de_procesos_activos, key))->estado == estado){
            printf("---PID: %s\n", key);
        }
    }
}

void iniciar_planificacion(){
    sem_wait(&mx_planificacion_activada);
    planificacion_activada = true;
    sem_post(&mx_planificacion_activada);

}

void detener_planificacion(){

    sem_wait(&mx_planificacion_activada);
    planificacion_activada = false;
    sem_post(&mx_planificacion_activada);
}

void* planificador_largo_plazo(){
    while(1){
        if(planificacion_activada){
            if(!queue_is_empty(cola_new) && procesos_activos < grado_multiprogramacion){
                
                sem_wait(&mx_cola_new);
                t_pcb* proceso = queue_pop(cola_new);
                sem_post(&mx_cola_new);

                sem_wait(&mx_procesos_activos);
                procesos_activos++;
                sem_post(&mx_procesos_activos);

                sem_wait(&mx_cola_ready);
                queue_push(cola_ready, proceso);

                
                proceso->estado = LISTO;
                sem_post(&mx_cola_ready);
                
                log_info(logger, "PID: %d - Estado Anterior: NUEVO - Estado Actual: LISTO", proceso->pid);
                
                logear_cola_ready();

                sem_wait(&mx_dicc_de_procesos);
                ((t_proceso_admin*) dictionary_get(dicc_de_procesos_activos, string_itoa(proceso->pid)))->estado  = LISTO;
                sem_post(&mx_dicc_de_procesos);
            }
        }
    }
}

void* planificador_corto_plazo( void* args){
    int* fds = args;
    bool cpu_ejecutando = false;
    bool devolver_ejecucion = false;
    t_pcb* pcb_ejecucion;
    while(1){
        bool hay_procesos_ready = !queue_is_empty(cola_ready) || !queue_is_empty(cola_ready_prioritaria);
        if(planificacion_activada && !cpu_ejecutando && (hay_procesos_ready || devolver_ejecucion)){
            if(devolver_ejecucion){
                devolver_ejecucion = false;
            }
            else{
                if(!queue_is_empty(cola_ready_prioritaria)){
                sem_wait(&mx_cola_ready_prioritaria);
                pcb_ejecucion = queue_pop(cola_ready_prioritaria);
                sem_post(&mx_cola_ready_prioritaria);
                }
                else{
                    sem_wait(&mx_cola_ready);
                    pcb_ejecucion = queue_pop(cola_ready);
                    sem_post(&mx_cola_ready);
                }
                pcb_ejecucion->estado = EJECUTANDO;
                log_info(logger, "PID: %d - Estado Anterior: LISTO - Estado Actual: EJECUTANDO", pcb_ejecucion->pid);
            }

            sem_wait(&mx_dicc_de_procesos);
            ((t_proceso_admin*) dictionary_get(dicc_de_procesos_activos, string_itoa(pcb_ejecucion->pid)))->estado = EJECUTANDO;
            sem_post(&mx_dicc_de_procesos);

            int quantum_restante = pcb_ejecucion->quantum;

            t_paquete* pcb_empaquetado = malloc(sizeof(t_paquete));
            serializar_pcb(pcb_ejecucion, pcb_empaquetado);
            cpu_ejecutando = true;
            send_paquete(&fds[0], pcb_empaquetado);
            
            clock_rr = temporal_create();
            pthread_t tid_interruptor;
            int args_interrupt[2] = {fds[1], quantum_restante};
            interrup_activada = true;
            pthread_create(&tid_interruptor, NULL, (void*) interruptor, (void*) args_interrupt);
            t_paquete* desalojo_empaquetado = malloc(sizeof(t_paquete));
            recv_paquete(&fds[0], desalojo_empaquetado);
            interrup_activada = false;
            temporal_stop(clock_rr);
            
            pthread_join(tid_interruptor, NULL);

            cpu_ejecutando = false;

            t_desalojo* desalojo_recibido = malloc(sizeof(t_desalojo));
            deserializar_desalojo(desalojo_recibido, desalojo_empaquetado);
            log_debug(debugger, "Desalojo deserializado con motivo %d y pid %d", (int) desalojo_recibido->motivo, desalojo_recibido->pcb->pid);
            quantum_restante -= temporal_gettime(clock_rr);
            temporal_destroy(clock_rr);
            log_debug(debugger, "Reloj destruido");

            if(!strcmp(algoritmo_planificacion, "VRR")){
                desalojo_recibido->pcb->quantum = desalojo_recibido->motivo == FIN_DE_QUANTUM ? quantum : quantum_restante;
            }
            
            while(!planificacion_activada){ /*nada */}
            //TRATAR DESALOJO
            switch (desalojo_recibido->motivo){
                case FINALIZACION_NORMAL:
                    log_info(logger, "PID: %d - Estado Anterior: EJECUTANDO - Estado Actual: FINALIZADO", desalojo_recibido->pcb->pid);
                    log_info(logger, "Finaliza el proceso %d - Motivo: SUCCESS", desalojo_recibido->pcb->pid);
                    pasar_a_exit(desalojo_recibido->pcb);
                    break;
                case FIN_DE_QUANTUM:
                    log_info(logger, "PID: %d - Estado Anterior: EJECUTANDO - Estado Actual: LISTO", desalojo_recibido->pcb->pid);
                    log_info(logger, "PID: %d - Desalojado por fin de Quantum", desalojo_recibido->pcb->pid);

                    sem_wait(&mx_dicc_de_procesos);
                    ((t_proceso_admin*) dictionary_get(dicc_de_procesos_activos, string_itoa(desalojo_recibido->pcb->pid)))->estado = LISTO;
                    sem_post(&mx_dicc_de_procesos);

                    mover_a_ready(desalojo_recibido->pcb);
                    logear_cola_ready();
                    break;
                case DESALOJO_POR_IO:
                    char* dispositivo_pedido = desalojo_recibido->datos_adicionales[0];
                    char* operacion_pedida = desalojo_recibido->datos_adicionales[1];
                    if(!dictionary_has_key(dicc_de_ios, dispositivo_pedido)){
                        log_info(logger, "PID: %d - Estado Anterior: EJECUTANDO - Estado Actual: FINALIZADO", desalojo_recibido->pcb->pid);
                        log_info(logger, "Finaliza el proceso %d - Motivo: INVALID_INTERFACE", desalojo_recibido->pcb->pid);

                        pasar_a_exit(desalojo_recibido->pcb);
                    }
                    else{
                        t_io_data* interfaz = dictionary_get(dicc_de_ios, dispositivo_pedido);
                        if(!puede_realizar_io(interfaz->io_tipo, operacion_pedida)){
                            log_info(logger, "PID: %d - Estado Anterior: EJECUTANDO - Estado Actual: FINALIZADO", desalojo_recibido->pcb->pid);
                            log_info(logger, "Finaliza el proceso %d - Motivo: INVALID_INTERFACE", desalojo_recibido->pcb->pid);
                            pasar_a_exit(desalojo_recibido->pcb);
                        }
                        else{
                            t_pcb_bloqueado* pcb_a_bloquear = malloc(sizeof(t_pcb_bloqueado));
                            pcb_a_bloquear->pcb = desalojo_recibido->pcb;
                            pcb_a_bloquear->cant_datos = desalojo_recibido->cant_datos;
                            pcb_a_bloquear->datos_peticion = malloc(sizeof(char*)*pcb_a_bloquear->cant_datos);
                            for(int i = 0; i < pcb_a_bloquear->cant_datos; i++){
                                pcb_a_bloquear->datos_peticion[i] = malloc(strlen(desalojo_recibido->datos_adicionales[i]) + 1);
                                strcpy(pcb_a_bloquear->datos_peticion[i], desalojo_recibido->datos_adicionales[i]);
                            }
                            
                            sem_wait(interfaz->mx_cola_block);
                            queue_push(interfaz->cola_block, pcb_a_bloquear);
                            log_info(logger, "PID: %d - Estado Anterior: EJECUTANDO - Estado Actual: BLOQUEADO", pcb_a_bloquear->pcb->pid);
                            log_info(logger, "PID: %d - Bloqueado por: %s", pcb_a_bloquear->pcb->pid, interfaz->io_nombre);
                            sem_post(interfaz->mx_cola_block);

                            sem_wait(&mx_dicc_de_procesos);
                            t_proceso_admin* nodo_a_bloquear = dictionary_get(dicc_de_procesos_activos, string_itoa(desalojo_recibido->pcb->pid));
                            nodo_a_bloquear->estado = BLOQUEADO;
                            nodo_a_bloquear->cola_bloqueante = interfaz->cola_block;
                            nodo_a_bloquear->mx_cola = interfaz->mx_cola_block;
                            sem_post(&mx_dicc_de_procesos); 

                        }
                    }
                    break;
                case SOLICITUD_WAIT:
                    t_recurso* recurso_solicitado = buscar_recurso(desalojo_recibido->datos_adicionales[0]);
                    if(recurso_solicitado == NULL){
                        log_info(logger, "PID: %d - Estado Anterior: EJECUTANDO - Estado Actual: FINALIZADO", desalojo_recibido->pcb->pid);
                        log_info(logger, "Finaliza el proceso %d - Motivo: INVALID_RESOURCE", desalojo_recibido->pcb->pid);
                        pasar_a_exit(desalojo_recibido->pcb);
                    }
                    else if(recurso_solicitado->semaforo <= 0){
                        sem_wait(&(recurso_solicitado->mx_modificacion));
                        recurso_solicitado->semaforo--;
                        queue_push(recurso_solicitado->pcbs_bloqueados, desalojo_recibido->pcb);
                        sem_post(&(recurso_solicitado->mx_modificacion));
                        desalojo_recibido->pcb->estado = BLOQUEADO;
                        log_info(logger, "PID: %d - Estado Anterior: EJECUTANDO - Estado Actual: BLOQUEADO", desalojo_recibido->pcb->pid);
                        log_info(logger, "PID: %d - Bloqueado por: %s", desalojo_recibido->pcb->pid, recurso_solicitado->nombre);

                        sem_wait(&mx_dicc_de_procesos);
                        t_proceso_admin* nodo_a_bloquear = dictionary_get(dicc_de_procesos_activos, string_itoa(desalojo_recibido->pcb->pid));
                        nodo_a_bloquear->estado = BLOQUEADO;
                        nodo_a_bloquear->cola_bloqueante = recurso_solicitado->pcbs_bloqueados;
                        nodo_a_bloquear->mx_cola = &(recurso_solicitado->mx_modificacion);
                        sem_post(&mx_dicc_de_procesos);
                    }
                    else{
                        sem_wait(&(recurso_solicitado->mx_modificacion));
                        recurso_solicitado->semaforo--;
                        int* ptr_pid = malloc(sizeof(int32_t));
                        *ptr_pid = desalojo_recibido->pcb->pid;
                        list_add(recurso_solicitado->pids_asignados, ptr_pid);
                        sem_post(&(recurso_solicitado->mx_modificacion));
                        pcb_ejecucion = desalojo_recibido->pcb;
                        devolver_ejecucion = true;
                    }
                    break;
                case SOLICITUD_SIGNAL:
                    t_recurso* recurso_signal= buscar_recurso(desalojo_recibido->datos_adicionales[0]);
                    if(recurso_signal == NULL){
                        log_info(logger, "PID: %d - Estado Anterior: EJECUTANDO - Estado Actual: FINALIZADO", desalojo_recibido->pcb->pid);
                        log_info(logger, "Finaliza el proceso %d - Motivo: INVALID_RESOURCE", desalojo_recibido->pcb->pid);

                        pasar_a_exit(desalojo_recibido->pcb);
                    }
                    else{
                        sem_wait(&(recurso_signal->mx_modificacion));
                        recurso_signal->semaforo++;
                        buscar_y_borrar(recurso_signal->pids_asignados, desalojo_recibido->pcb->pid);
                        sem_post(&(recurso_signal->mx_modificacion));

                        pcb_ejecucion = desalojo_recibido->pcb;
                        devolver_ejecucion = true;

                        desbloquear_elemento_del_recurso(recurso_signal);
                    }
                    break;  
            }
        }

    }
}

bool buscar_y_borrar(t_list* lista, int pid){
    for(int i = 0; i < list_size(lista); i++){
        int* ptr_pid = list_get(lista, i);
        if(*ptr_pid == pid){
            list_remove_and_destroy_element(lista, i, free);
            return true;
        }
    }
    return false;
}

bool buscar_y_borrar_pcb_por_pid(t_list* lista, int32_t pid, int32_t (*obtener_pid)(void*)){
    for(int i = 0; i < list_size(lista); i++){
        void* elemento = list_get(lista, i);
        if(obtener_pid(elemento) == pid){
            list_remove(lista, i);
            return true;
        }
    }
    return false;
}

int32_t obtener_pid_de_pcb(void* pcb){
    return ((t_pcb*) pcb)->pid;
}

int32_t obtener_pid_de_pcb_bloqueado(void* pcb_bloq){
    return ((t_pcb_bloqueado*) pcb_bloq)->pcb->pid;
}

void* interruptor(void* void_args){
    if(!strcmp(algoritmo_planificacion, "FIFO")){
        return EXIT_SUCCESS;
    }
    int* args = (int*) void_args;
    int fd_cpu_interrupt = args[0];
    int quantum_rest = args[1];
    while(temporal_gettime(clock_rr) < quantum_rest && interrup_activada){
        /*nada*/
    }
    if(interrup_activada){
        int interrupt_signal = 1;
        send(fd_cpu_interrupt, &interrupt_signal, sizeof(int), 0);
    }
    return EXIT_SUCCESS;
}

void pasar_a_exit(t_pcb* pcb){ //chequear todos los lugares en los que lo uso!!
    
    //sacarlo de la cola que este
    bool pcb_sacado = false;
    while(!pcb_sacado){
        switch(pcb->estado){
            case NUEVO:
                sem_wait(&mx_cola_new);
                pcb_sacado = buscar_y_borrar_pcb_por_pid(cola_new->elements, pcb->pid);
                sem_post(&mx_cola_new);
                break;
            case LISTO:
                sem_wait(&mx_cola_ready);
                pcb_sacado = pcb_sacado = buscar_y_borrar_pcb_por_pid(cola_ready->elements, pcb->pid);
                sem_post(&mx_cola_ready);
                if(!strcmp(algoritmo_planificacion, "VRR") && !pcb_sacado){
                    sem_wait(&mx_cola_ready_prioritaria);
                    pcb_sacado = pcb_sacado = buscar_y_borrar_pcb_por_pid(cola_ready_prioritaria->elements, pcb->pid);
                    sem_post(&mx_cola_ready_prioritaria);
                }
                break;
            case EJECUTANDO:
                break;
            case BLOQUEADO:
                t_proceso_admin* nodo_proceso = dictionary_get(dicc_de_ios, string_itoa(pcb->pid));
                sem_wait(nodo_proceso->mx_cola);
                pcb_sacado = 
                sem_post(nodo_proceso->mx_cola);
                break;


        }
    }

    //liberar recursos en uso
    for(int i = 0; i < cant_de_recursos; i++){
        t_recurso* recurso = array_recursos[i];
        t_list* pids_asignados = recurso->pids_asignados;
        log_debug(debugger, "Intentando borrar pid");
        while(buscar_y_borrar(pids_asignados, pcb->pid)){
            recurso->semaforo++;
            log_debug(debugger, "Intentando desbloquear elemento");
            desbloquear_elemento_del_recurso(recurso);
            log_debug(debugger, "Intentando borrar pid");
        }   
    }
    

    pcb->estado = FINALIZADO;
    sem_wait(&mx_procesos_activos);
    procesos_activos--;
    sem_post(&mx_procesos_activos);

    //hay que solicitar a memoria que libere lo que tiene que liberar
}

bool desbloquear_elemento_del_recurso(t_recurso* recurso){
    if(recurso->semaforo >= 0 && !queue_is_empty(recurso->pcbs_bloqueados)){
        sem_wait(&(recurso->mx_modificacion));
        log_debug(debugger, "Intentando popear recurso");
        t_pcb* pcb_a_desbloquear = queue_pop(recurso->pcbs_bloqueados);
        int* ptr_pid = malloc(sizeof(int32_t));
        *ptr_pid = pcb_a_desbloquear->pid;
        list_add(recurso->pids_asignados, ptr_pid);
        sem_post(&(recurso->mx_modificacion));
        if(!strcmp(algoritmo_planificacion, "VRR")){
            log_info(logger, "PID: %d - Estado Anterior: BLOQUEADO - Estado Actual: LISTO", pcb_a_desbloquear->pid);
            mover_a_ready_prioritaria(pcb_a_desbloquear);
            logear_cola_ready_prioritaria();
        }
        else{
            log_info(logger, "PID: %d - Estado Anterior: BLOQUEADO - Estado Actual: LISTO", pcb_a_desbloquear->pid);
            mover_a_ready(pcb_a_desbloquear);
            logear_cola_ready();
        }
        return true;
    }
    return false;
}

void mover_a_ready(t_pcb* pcb){
    sem_wait(&mx_cola_ready);
    pcb->estado = LISTO;
    queue_push(cola_ready, pcb);
    sem_post(&mx_cola_ready);
}

void mover_a_ready_prioritaria(t_pcb* pcb){
    sem_wait(&mx_cola_ready_prioritaria);
    pcb->estado = LISTO;
    queue_push(cola_ready_prioritaria, pcb);
    sem_post(&mx_cola_ready_prioritaria);
}

void cambiar_grado_multip(char* nuevo_grado){
    grado_multiprogramacion = atoi(nuevo_grado);
}

void logear_cola_ready(void){
    char* string_pids_ready = string_new();
    for(int i = 0; i < queue_size(cola_ready); i++){
        string_append_with_format(&string_pids_ready, " %d ", ((t_pcb*) list_get(cola_ready->elements, i))->pid);
    }
    log_info(logger, "Cola Ready: [%s]", string_pids_ready);
}


void logear_cola_ready_prioritaria(void){
    char* string_pids_ready = string_new();
    for(int i = 0; i < queue_size(cola_ready_prioritaria); i++){
        string_append_with_format(&string_pids_ready, " %d ", ((t_pcb*) list_get(cola_ready_prioritaria->elements, i))->pid);
    }
    log_info(logger, "Cola Ready Prioridad: [%s]", string_pids_ready);
}


bool puede_realizar_io(char* tipo, char* operacion){
    if(!strcmp(tipo, "GENERICA") && !strcmp(operacion, "IO_GEN_SLEEP")){
        return true;
    }
    else if(!strcmp(tipo, "STDIN") && !strcmp(operacion, "IO_STDIN_READ")){
        return true;
    }
    else if(!strcmp(tipo, "STDOUT") && !strcmp(operacion, "IO_STDOUT_WRITE")){
        return true;
    }
    else if(!strcmp(tipo, "DIALFS") && (!strcmp(operacion, "IO_FS_CREATE") || !strcmp(operacion, "IO_FS_DELETE") || !strcmp(operacion, "IO_FS_TRUNCATE")
                                                                    || !strcmp(operacion, "IO_FS_WRITE") || !strcmp(operacion, "IO_FS_READ"))){
        return true;
    }
    else{
        return false;
    }
}

char* estado_to_str(t_estado estado){
    switch (estado){
        case NUEVO:
            return "NUEVO";
            break;
        case LISTO:
            return "LISTO";
            break;
        case EJECUTANDO:
            return "EJECUTANDO";
            break;
        case BLOQUEADO:
            return "BLOQUEADO";
            break;
        case FINALIZADO:
            return "FINALIZADO";
            break;
    }
    return NULL;
}

t_recurso* buscar_recurso(char* nombre){
    for(int i = 0; i < cant_de_recursos; i++){
        t_recurso* recurso = array_recursos[i];
        if(!strcmp(recurso->nombre, nombre)){
            return recurso;
        }
    }
    return NULL;
}
