#include "paquetes.h"


void send_paquete(int* socket, t_paquete* paquete_ptr){

    void* a_enviar = malloc(paquete_ptr->buffer->size + sizeof(op_code) + sizeof(int));
    int offset = 0;

    memcpy(a_enviar + offset, &(paquete_ptr->codigo_operacion), sizeof(op_code));
    offset += sizeof(op_code);

    memcpy(a_enviar + offset, &(paquete_ptr->buffer->size), sizeof(int));
    offset += sizeof(int);

    memcpy(a_enviar + offset, paquete_ptr->buffer->stream, paquete_ptr->buffer->size);

    send(*socket, a_enviar, paquete_ptr->buffer->size + sizeof(op_code) + sizeof(int), 0);

    // llamar para liberar paquete al final
    free(a_enviar);
    free_paquete (paquete_ptr);
} 

void recv_paquete(int* socket, t_paquete* paquete){
    paquete->buffer = malloc(sizeof(t_buffer));

    recv(*socket, &(paquete->codigo_operacion), sizeof(op_code), MSG_WAITALL);

    recv(*socket, &(paquete->buffer->size), sizeof(int), MSG_WAITALL);
    paquete->buffer->stream = malloc(paquete->buffer->size);
    recv(*socket, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);
}

void serializar_nuevo_proceso(t_nuevo_proceso* nuevo_proceso, t_paquete* paquete){
    int sizepath = ( strlen(nuevo_proceso->path)+1 ) * sizeof(char);

    t_buffer* buffer = malloc(sizeof(t_buffer));

    buffer->size = sizeof(int) * 2 //int pid *2 porque agrego el sizepath
            + sizepath; //path

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    // para el pid
    memcpy(buffer->stream + buffer->offset, &(nuevo_proceso->pid), sizeof(int));
    buffer->offset += sizeof(int);
    //para el path
    memcpy(buffer->stream + buffer->offset, &sizepath, sizeof(int)); 
    buffer->offset += sizeof(int);
    memcpy(buffer->stream + buffer->offset, nuevo_proceso->path, sizepath);
    buffer->offset += sizepath;


    // lleno el paquete con los valores del buffer
    paquete->codigo_operacion = NUEVO_PROCESO; 
    paquete->buffer = buffer;

}


void deserializar_nuevo_proceso(t_nuevo_proceso* nuevo_proceso, t_paquete* paquete){
    int sizepath;
    void* stream = paquete->buffer->stream;
    // Deserializamos los campos que tenemos en el buffer

    memcpy(&(nuevo_proceso->pid), stream, sizeof(int));
    stream += sizeof(int);
    memcpy(&sizepath, stream, sizeof(int));
    stream += sizeof(int);
    nuevo_proceso->path = malloc(sizeof(char)*sizepath);
    memcpy(nuevo_proceso->path, stream, sizepath);
    free_paquete(paquete);
}

void serializar_pcb(t_pcb* pcb, t_paquete* paquete){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size =  sizeof(int32_t) + //pid
                    sizeof(int) + //quantum
                    sizeof(t_estado) + //estado
                    4 * sizeof(int8_t) + // AX,BX,CX,DX
                    7 * sizeof(int32_t); //PC, EAX, EBX, ECX, EDX, SI, DI

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream + buffer->offset, &(pcb->pid), sizeof(int32_t));
    buffer->offset += sizeof(int32_t); 
    memcpy(buffer->stream + buffer->offset, &(pcb->quantum), sizeof(int));
    buffer->offset += sizeof(int);
    memcpy(buffer->stream + buffer->offset, &(pcb->estado), sizeof(t_estado));
    buffer->offset += sizeof(t_estado);
    memcpy(buffer->stream + buffer->offset, &(pcb->registros_cpu->PC), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);
    memcpy(buffer->stream + buffer->offset, &(pcb->registros_cpu->AX), sizeof(int8_t));
    buffer->offset += sizeof(int8_t);
    memcpy(buffer->stream + buffer->offset, &(pcb->registros_cpu->BX), sizeof(int8_t));
    buffer->offset += sizeof(int8_t);
    memcpy(buffer->stream + buffer->offset, &(pcb->registros_cpu->CX), sizeof(int8_t));
    buffer->offset += sizeof(int8_t);
    memcpy(buffer->stream + buffer->offset, &(pcb->registros_cpu->DX), sizeof(int8_t));
    buffer->offset += sizeof(int8_t);
    memcpy(buffer->stream + buffer->offset, &(pcb->registros_cpu->EAX), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);
    memcpy(buffer->stream + buffer->offset, &(pcb->registros_cpu->EBX), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);
    memcpy(buffer->stream + buffer->offset, &(pcb->registros_cpu->ECX), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);
    memcpy(buffer->stream + buffer->offset, &(pcb->registros_cpu->EDX), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);
    memcpy(buffer->stream + buffer->offset, &(pcb->registros_cpu->SI), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);
    memcpy(buffer->stream + buffer->offset, &(pcb->registros_cpu->DI), sizeof(int32_t));
    
    paquete->codigo_operacion = PCB; 
    paquete->buffer = buffer;
}

void deserializar_pcb(t_pcb* pcb, t_paquete* paquete){

    void* stream = paquete->buffer->stream;

    memcpy(&(pcb->pid), stream, sizeof(int32_t));
    stream += sizeof(int32_t);
    memcpy(&(pcb->quantum), stream, sizeof(int));
    stream += sizeof(int);
    memcpy(&(pcb->estado), stream , sizeof(t_estado));
    stream += sizeof(t_estado);

    pcb->registros_cpu = malloc(sizeof(t_registros));
    memcpy(&(pcb->registros_cpu->PC), stream, sizeof(int32_t));
    stream += sizeof(int32_t);

    memcpy(&(pcb->registros_cpu->AX), stream, sizeof(int8_t));
    stream += sizeof(int8_t);
    memcpy(&(pcb->registros_cpu->BX), stream, sizeof(int8_t));
    stream += sizeof(int8_t);
    memcpy(&(pcb->registros_cpu->CX), stream, sizeof(int8_t));
    stream += sizeof(int8_t);
    memcpy(&(pcb->registros_cpu->DX), stream, sizeof(int8_t));
    stream += sizeof(int8_t);
    
    memcpy(&(pcb->registros_cpu->EAX), stream, sizeof(int32_t));
    stream += sizeof(int32_t);
    memcpy(&(pcb->registros_cpu->EBX), stream, sizeof(int32_t));
    stream += sizeof(int32_t);
    memcpy(&(pcb->registros_cpu->ECX), stream, sizeof(int32_t));
    stream += sizeof(int32_t);
    memcpy(&(pcb->registros_cpu->EDX), stream, sizeof(int32_t));
    stream += sizeof(int32_t);
    memcpy(&(pcb->registros_cpu->SI), stream, sizeof(int32_t));
    stream += sizeof(int32_t);
    memcpy(&(pcb->registros_cpu->DI), stream, sizeof(int32_t));
    stream += sizeof(int32_t);
    
    free_paquete(paquete);

}



void serializar_pedido(t_pedir_instruccion* pedido, t_paquete* paquete){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size =  2 * sizeof(int); //pid + pc

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream + buffer->offset, &(pedido->pid), sizeof(int));
    buffer->offset += sizeof(int); 
    memcpy(buffer->stream + buffer->offset, &(pedido->pc), sizeof(int));
    buffer->offset += sizeof(int);

    paquete->codigo_operacion = PEDIR_INSTRUCCION; 
    paquete->buffer = buffer;
}

void deserializar_pedido(t_pedir_instruccion* pedido, t_paquete* paquete){
    void* stream = paquete->buffer->stream;

    memcpy(&(pedido->pid), stream, sizeof(int));
    stream += sizeof(int);
    memcpy(&(pedido->pc), stream, sizeof(int));
    stream += sizeof(int);

    free_paquete(paquete);
}


void serializar_instruccion(t_instruccion* instruccion, t_paquete* paquete){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    int tamanio_lista_argumentos = sizeof(int); //elements_count
    //y ahora sumo el tamano de cada argumento
    int cantidad_de_argumentos = list_size(instruccion->argumentos);
    for( int i = 0; i < cantidad_de_argumentos; i++){
        char* argumento = list_get(instruccion->argumentos, i);
        int tamanio_argumento = (strlen( argumento ) + 1) * sizeof(char);
        tamanio_lista_argumentos += sizeof(int);
        tamanio_lista_argumentos += tamanio_argumento;
    }
                      
    buffer->size =  sizeof(inst_code) + //codigo_instruccion
                    tamanio_lista_argumentos; //argumentos

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    //paso el codigo de instruccion
    memcpy(buffer->stream + buffer->offset, &(instruccion->codigo_instruccion), sizeof(inst_code));
    buffer->offset += sizeof(inst_code); 
    //ahora paso toda la lista (elements_count y cada elemento con su tamanio (int))
    memcpy(buffer->stream + buffer->offset, &(cantidad_de_argumentos), sizeof(int));
    buffer->offset += sizeof(int);
    for(int i = 0; i < cantidad_de_argumentos; i++){
        char* argumento = (char*) list_get(instruccion->argumentos, i);
        int tamanio_argumento = (strlen( argumento ) + 1) * sizeof(char);
        memcpy(buffer->stream + buffer->offset, &(tamanio_argumento), sizeof(int));
        buffer->offset += sizeof(int);
        memcpy(buffer->stream + buffer->offset, argumento, tamanio_argumento);
        buffer->offset += tamanio_argumento;
    }

    paquete->codigo_operacion = INSTRUCCION; 
    paquete->buffer = buffer;

}

void deserializar_instruccion(t_instruccion* instruccion, t_paquete* paquete){
    void* stream = paquete->buffer->stream;
    t_list* argumentos = list_create();
    int cantidad_de_argumentos;

    memcpy(&(instruccion->codigo_instruccion), stream, sizeof(inst_code));
    stream += sizeof(inst_code);

    memcpy(&(cantidad_de_argumentos), stream, sizeof(int));
    stream += sizeof(int);

    for(int i = 0; i < cantidad_de_argumentos; i++){
        int tamanio_argumento;
        char* argumento;
        memcpy(&(tamanio_argumento), stream, sizeof(int));
        stream += sizeof(int);
        argumento = malloc(sizeof(char)* tamanio_argumento);
        memcpy(argumento, stream, tamanio_argumento);
        stream += tamanio_argumento;
        list_add(argumentos, argumento);
    }


    instruccion->argumentos = argumentos;

    free_paquete(paquete);
}


void free_paquete(t_paquete* paquete){
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}


void free_nuevo_proceso(t_nuevo_proceso* np){
    //free(np->path);
    //free(np);
}


t_registros* crear_registros_en_cero(){
    t_registros* nuevo_registros = malloc(sizeof(t_registros));
    nuevo_registros->PC=0;
    nuevo_registros->AX=0;
    nuevo_registros->BX=0;
    nuevo_registros->CX=0;
    nuevo_registros->DX=0;
    nuevo_registros->EAX=0;
    nuevo_registros->EBX=0;
    nuevo_registros->ECX=0;
    nuevo_registros->EDX=0;
    nuevo_registros->SI=0;
    nuevo_registros->DI=0;
    return nuevo_registros;
}

void serializar_identificacion_io(t_identificacion_io* id_io, t_paquete* paquete){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    int tamanio_nombre = sizeof(char) * (strlen(id_io->nombre) + 1);
    int tamanio_tipo = sizeof(char) * (strlen(id_io->tipo) + 1);
    buffer->size =  sizeof(int) + //tamano del nombre
                    tamanio_nombre + //nombre
                    sizeof(int) + //tamanio del tipo
                    tamanio_tipo; //tipo

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream + buffer->offset, &(tamanio_nombre), sizeof(int));
    buffer->offset += sizeof(int); 
    memcpy(buffer->stream + buffer->offset, id_io->nombre, tamanio_nombre);
    buffer->offset += tamanio_nombre; 
    memcpy(buffer->stream + buffer->offset, &(tamanio_tipo), sizeof(int));
    buffer->offset += sizeof(int); 
    memcpy(buffer->stream + buffer->offset, id_io->tipo, tamanio_tipo);
    buffer->offset += tamanio_tipo;

    paquete->codigo_operacion = IDENTIFICACION_IO; 
    paquete->buffer = buffer;
}

void deserializar_identificacion_io(t_identificacion_io* id_io, t_paquete* paquete){
    void* stream = paquete->buffer->stream;
    int tamanio_nombre, tamanio_tipo;
    
    memcpy(&(tamanio_nombre), stream, sizeof(int));
    stream += sizeof(int);
    id_io->nombre = malloc(sizeof(char) * tamanio_nombre);
    memcpy(id_io->nombre, stream, tamanio_nombre);
    stream += tamanio_nombre;

    
    memcpy(&(tamanio_tipo), stream, sizeof(int));
    stream += sizeof(int);
    id_io->tipo = malloc(sizeof(char) * tamanio_tipo);
    memcpy(id_io->tipo, stream, tamanio_tipo);
    stream += tamanio_tipo;

    free_paquete(paquete);
}

void serializar_peticion_gen_sleep(t_peticion_gen_sleep* peticion, t_paquete* paquete){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(int) + sizeof(int32_t);
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream + buffer->offset, &(peticion->pid), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);

    memcpy(buffer->stream + buffer->offset, &(peticion->unidades_de_sleep), sizeof(int));

    paquete->codigo_operacion = PETICION_GEN_SLEEP; 
    paquete->buffer = buffer;
}

void deserializar_peticion_gen_sleep(t_peticion_gen_sleep* peticion, t_paquete* paquete){
    void* stream = paquete->buffer->stream;

    memcpy(&(peticion->pid), stream, sizeof(int32_t));
    stream += sizeof(int32_t);

    memcpy(&(peticion->unidades_de_sleep), stream, sizeof(int));

    free_paquete(paquete);
}

void serializar_respuesta_gen_sleep(t_respuesta_gen_sleep* respuesta, t_paquete* paquete){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(t_respuesta_gen_sleep);
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream + buffer->offset, respuesta, sizeof(t_respuesta_gen_sleep));

    paquete->codigo_operacion = RESPUESTA_GEN_SLEEP; 
    paquete->buffer = buffer;
}

void deserializar_respuesta_gen_sleep(t_respuesta_gen_sleep* respuesta, t_paquete* paquete){
    void* stream = paquete->buffer->stream;
    memcpy(respuesta, stream, sizeof(t_respuesta_gen_sleep));
    free_paquete(paquete);
}

void serializar_desalojo(t_desalojo* desalojo, t_paquete* paquete){
    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer->size = sizeof(t_motivo_cpu)
                + sizeof(int32_t) + sizeof(int) + sizeof(t_estado) //pcb
                + 7 * sizeof(int32_t) + 4 * sizeof(int8_t) //registros del pcb
                + sizeof(int); //cant_datos
    for (int i = 0; i < desalojo->cant_datos; i++){
        char* cadena = desalojo->datos_adicionales[i];
        buffer->size += sizeof(int) + strlen(cadena) + 1; //entero para el tamanio de cadena + tamanio de cadena
    }

    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream + buffer->offset, &(desalojo->motivo), sizeof(t_motivo_cpu));
    buffer->offset += sizeof(t_motivo_cpu);

    memcpy(buffer->stream + buffer->offset, &(desalojo->pcb->pid), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);
    memcpy(buffer->stream + buffer->offset, &(desalojo->pcb->quantum), sizeof(int));
    buffer->offset += sizeof(int);
    memcpy(buffer->stream + buffer->offset, &(desalojo->pcb->estado), sizeof(t_estado));
    buffer->offset += sizeof(t_estado);
    memcpy(buffer->stream + buffer->offset, &(desalojo->pcb->registros_cpu->PC), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);
    memcpy(buffer->stream + buffer->offset, &(desalojo->pcb->registros_cpu->AX), sizeof(int8_t));
    buffer->offset += sizeof(int8_t);
    memcpy(buffer->stream + buffer->offset, &(desalojo->pcb->registros_cpu->BX), sizeof(int8_t));
    buffer->offset += sizeof(int8_t);
    memcpy(buffer->stream + buffer->offset, &(desalojo->pcb->registros_cpu->CX), sizeof(int8_t));
    buffer->offset += sizeof(int8_t);
    memcpy(buffer->stream + buffer->offset, &(desalojo->pcb->registros_cpu->DX), sizeof(int8_t));
    buffer->offset += sizeof(int8_t);
    memcpy(buffer->stream + buffer->offset, &(desalojo->pcb->registros_cpu->EAX), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);
    memcpy(buffer->stream + buffer->offset, &(desalojo->pcb->registros_cpu->EBX), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);
    memcpy(buffer->stream + buffer->offset, &(desalojo->pcb->registros_cpu->ECX), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);
    memcpy(buffer->stream + buffer->offset, &(desalojo->pcb->registros_cpu->EDX), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);
    memcpy(buffer->stream + buffer->offset, &(desalojo->pcb->registros_cpu->SI), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);
    memcpy(buffer->stream + buffer->offset, &(desalojo->pcb->registros_cpu->DI), sizeof(int32_t));
    buffer->offset += sizeof(int32_t);

    memcpy(buffer->stream + buffer->offset, &(desalojo->cant_datos), sizeof(int));
    buffer->offset += sizeof(int);

    for (int i = 0; i < desalojo->cant_datos; i++){
        char* cadena = desalojo->datos_adicionales[i];
        int tamanio_cadena = strlen(cadena) + 1;
        memcpy(buffer->stream + buffer->offset, &(tamanio_cadena), sizeof(int));
        buffer->offset += sizeof(int);
        memcpy(buffer->stream + buffer->offset, cadena, tamanio_cadena);
        buffer->offset += tamanio_cadena;
    }

    paquete->codigo_operacion = PROCESO_DESALOJO;
    paquete->buffer = buffer;
}

void deserializar_desalojo(t_desalojo* desalojo, t_paquete* paquete){
    void* stream = paquete->buffer->stream;
    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->registros_cpu = malloc(sizeof(t_registros));
    desalojo->pcb = pcb;

    memcpy(&(desalojo->motivo), stream, sizeof(t_motivo_cpu));
    stream += sizeof(t_motivo_cpu);

    memcpy(&(pcb->pid), stream, sizeof(int32_t));
    stream += sizeof(int32_t);
    memcpy(&(pcb->quantum), stream, sizeof(int));
    stream += sizeof(int);
    memcpy(&(pcb->estado), stream, sizeof(t_estado));
    stream += sizeof(t_estado);

    memcpy(&(pcb->registros_cpu->PC), stream, sizeof(int32_t));
    stream += sizeof(int32_t);
    memcpy(&(pcb->registros_cpu->AX), stream, sizeof(int8_t));
    stream += sizeof(int8_t);
    memcpy(&(pcb->registros_cpu->BX), stream, sizeof(int8_t));
    stream += sizeof(int8_t);
    memcpy(&(pcb->registros_cpu->CX), stream, sizeof(int8_t));
    stream += sizeof(int8_t);
    memcpy(&(pcb->registros_cpu->DX), stream, sizeof(int8_t));
    stream += sizeof(int8_t);
    memcpy(&(pcb->registros_cpu->EAX), stream, sizeof(int32_t));
    stream += sizeof(int32_t);
    memcpy(&(pcb->registros_cpu->EBX), stream, sizeof(int32_t));
    stream += sizeof(int32_t);
    memcpy(&(pcb->registros_cpu->ECX), stream, sizeof(int32_t));
    stream += sizeof(int32_t);
    memcpy(&(pcb->registros_cpu->EDX), stream, sizeof(int32_t));
    stream += sizeof(int32_t);
    memcpy(&(pcb->registros_cpu->SI), stream, sizeof(int32_t));
    stream += sizeof(int32_t);
    memcpy(&(pcb->registros_cpu->DI), stream, sizeof(int32_t));
    stream += sizeof(int32_t);

    memcpy(&(desalojo->cant_datos), stream, sizeof(int));
    stream += sizeof(int);

    desalojo->datos_adicionales = desalojo->cant_datos == 0? NULL : malloc(sizeof(char*) * desalojo->cant_datos);
    for(int i = 0; i < desalojo->cant_datos; i++){
        int tamanio_cadena;
        memcpy(&(tamanio_cadena), stream, sizeof(int));
        stream += sizeof(int);
        desalojo->datos_adicionales[i] = malloc(tamanio_cadena);
        memcpy(desalojo->datos_adicionales[i], stream, tamanio_cadena);
        stream += tamanio_cadena;
    }

    free_paquete(paquete);
}

void free_desalojo(t_desalojo* desalojo){
    free_pcb(desalojo->pcb);
    for(int i = 0; i < desalojo->cant_datos; i++){
        free(desalojo->datos_adicionales[i]);
    }
    free(desalojo);
}

void free_pcb(t_pcb* pcb){
    free(pcb->registros_cpu);
    free(pcb);
}