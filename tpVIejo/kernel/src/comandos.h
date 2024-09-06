#ifndef COMANDOS_H_
#define COMANDOS_H_

#include<stdio.h>
#include<variableskernel.h>
#include <../../utils/src/cliente.h>
#include <commons/log.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdbool.h>




void iniciar_proceso(char* path);
int asignar_pid(void);

void iniciar_planificacion(void);
void detener_planificacion(void);

void listar_procesos_por_estado();

void cambiar_grado_multip(char* nuevo_grado);

void* planificador_largo_plazo();
void* planificador_corto_plazo( void* args);
void* interruptor(void* args);

t_pcb* crear_pcb(int pid); 

void pasar_a_exit(t_pcb* pcb);
void mover_a_ready(t_pcb* pcb);
void mover_a_ready_prioritaria(t_pcb* pcb);

char* estado_to_str(t_estado estado);

void logear_cola_ready(void);
void logear_cola_ready_prioritaria(void);

bool puede_realizar_io(char* tipo, char* operacion);

void imprimir_procesos_del_estado(t_estado estado);

bool buscar_y_borrar(t_list* lista, int pid);
bool buscar_y_borrar_pcb_por_pid(t_list* lista, int32_t pid, int32_t (*obtener_pid)(void*));

int32_t obtener_pid_de_pcb(void* pcb);

int32_t obtener_pid_de_pcb_bloqueado(void* pcb_bloq);

bool desbloquear_elemento_del_recurso(t_recurso* recurso);

t_recurso* buscar_recurso(char* nombre);

#endif