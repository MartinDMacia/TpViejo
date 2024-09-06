#ifndef TIPOS_H_
#define TIPOS_H_

#include<stdint.h>
#include<stdlib.h>
#include<commons/collections/dictionary.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<commons/string.h>
#include<semaphore.h>

typedef enum
{
	NUEVO_PROCESO, //Cuando kernel le envia a memoria el path con el pid
    PEDIR_INSTRUCCION, // Cuando CPU le pide a memoria una instruccion
    INSTRUCCION, //Cuando memoria le manda la instruccion a CPU
    PCB,
    PROCESO_DESALOJO,
    IDENTIFICACION_IO,
    PETICION_GEN_SLEEP,
    RESPUESTA_GEN_SLEEP,
}op_code;
//Por cada uno de estos op_code hay que codear las funciones para serializar y deserializar
typedef struct
{
	int size;
	int offset;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef enum{
    KE_ME,
    KE_CD,
    KE_CI,
	CP_ME,
    IO_KE,
}t_handshake;


 typedef enum
{
    NUEVO,
	LISTO,
	EJECUTANDO,
    BLOQUEADO,
    FINALIZADO
}t_estado;



typedef struct{
	int pid;
	char* path;
} t_nuevo_proceso;

typedef struct{
    int pid;
    int pc;
}t_pedir_instruccion;


typedef struct{
    int32_t PC;
    int8_t AX;
    int8_t BX;
    int8_t CX;
    int8_t DX;
    int32_t EAX;
    int32_t EBX;
    int32_t ECX;
    int32_t EDX;
    int32_t SI;
    int32_t DI;
}t_registros;


typedef struct{
    int32_t pid;
    int quantum;
    t_estado estado;
    t_registros* registros_cpu;
}t_pcb; 


typedef enum{
    SET,
    MOV_IN, //NO IMPLEMENTADA
    MOV_OUT, //NO IMPLEMENTADA
    SUM,
    SUB,
    JNZ,
    RESIZE, //NO IMPLEMENTADA
    COPY_STRING, //NO IMPLEMENTADA
    WAIT, //NO IMPLEMENTADA
    SIGNAL, //NO IMPLEMENTADA
    IO_GEN_SLEEP,
    IO_STDIN_READ, //NO IMPLEMENTADA
    IO_STDOUT_WRITE, //NO IMPLEMENTADA
    IO_FS_DELETE, //NO IMPLEMENTADA
    IO_FS_CREATE, //NO IMPLEMENTADA
    IO_FS_TRUNCATE, //NO IMPLEMENTADA
    IO_FS_WRITE, //NO IMPLEMENTADA
    IO_FS_READ, //NO IMPLEMENTADA
    EXIT,
    INST_ERROR, //POR LAS DUDAS, SI ES QUE LA INSTRUCCION ESTA MAL ESCRITA
}inst_code;

typedef struct{
    inst_code codigo_instruccion;
    t_list* argumentos;
}t_instruccion;

typedef struct{
    char* nombre;
    char* tipo;
}t_identificacion_io;

typedef enum{
    GEN_SLEEP,
}peticion_code;

typedef struct{
    t_pcb* pcb;
    int cant_datos;
    char** datos_peticion;
}t_pcb_bloqueado;


typedef struct{
    int unidades_de_sleep;
    int32_t pid;
} t_peticion_gen_sleep;

typedef int t_respuesta_gen_sleep;

typedef enum{
    FINALIZACION_NORMAL, // INSTRUCCION EXIT
    FIN_DE_QUANTUM,
    DESALOJO_POR_IO,
    SOLICITUD_WAIT, 
    SOLICITUD_SIGNAL,
}t_motivo_cpu;

typedef struct{
    t_motivo_cpu motivo;
    t_pcb* pcb;
    int cant_datos; 
    char** datos_adicionales; 
}t_desalojo;

typedef struct{
    char* io_nombre;
    char* io_tipo;
    sem_t* mx_cola_block;
    t_queue* cola_block;
} t_io_data;


typedef struct{
    char* nombre;
    int instancias;
    int semaforo;
    t_list* pids_asignados;
    t_queue* pcbs_bloqueados;
    sem_t mx_modificacion;
} t_recurso;

typedef struct{
    t_estado estado;
    t_queue* cola_bloqueante;
    sem_t* mx_cola;
} t_proceso_admin;

#endif