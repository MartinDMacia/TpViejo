#ifndef VARIABLESKERNEL_H_
#define VARIABLESKERNEL_H_

#include<commons/collections/queue.h>
#include<commons/collections/dictionary.h>
#include<commons/log.h>
#include<pthread.h>
#include<semaphore.h>
#include<commons/temporal.h>
#include<tipos.h>

extern t_queue* cola_new;
extern t_queue* cola_ready;
extern t_queue* cola_ready_prioritaria;

extern t_dictionary* dicc_de_ios;

extern t_dictionary* dicc_de_procesos_activos;

extern int cant_de_recursos;
extern t_recurso** array_recursos;

extern int proximo_pid; 
extern int fd_memoria;
extern int fd_cpu_dispatch;
extern int fd_cpu_interrupt;
extern int quantum;

extern int grado_multiprogramacion;

extern int procesos_activos;
extern sem_t mx_procesos_activos;

extern char* algoritmo_planificacion;

extern bool planificacion_activada;
extern bool planificacion_pausada;
extern sem_t mx_planificacion_activada;

extern sem_t mx_cola_new;
extern sem_t mx_cola_ready;
extern sem_t mx_cola_ready_prioritaria;
extern sem_t mx_dicc_de_ios;
extern sem_t mx_dicc_de_procesos;

extern t_log* logger;
extern t_log* debugger;

extern t_temporal* clock_rr;

#endif