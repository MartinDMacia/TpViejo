#ifndef PAQUETES_H_
#define PAQUETES_H_

#include<tipos.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>


void serializar_nuevo_proceso(t_nuevo_proceso* nuevo_proceso, t_paquete* paquete_a_guardar);
void deserializar_nuevo_proceso(t_nuevo_proceso* nuevo_proceso, t_paquete* paquete);


void serializar_pcb(t_pcb* pcb, t_paquete* paquete);
void deserializar_pcb(t_pcb* pcb, t_paquete* paquete);

void serializar_pedido(t_pedir_instruccion* pedido, t_paquete* paquete);
void deserializar_pedido(t_pedir_instruccion* pedido, t_paquete* paquete);

void serializar_instruccion(t_instruccion* instruccion, t_paquete* paquete);
void deserializar_instruccion(t_instruccion* instruccion, t_paquete* paquete);

void serializar_identificacion_io(t_identificacion_io* id_io, t_paquete* paquete);
void deserializar_identificacion_io(t_identificacion_io* id_io, t_paquete* paquete);

void serializar_peticion_gen_sleep(t_peticion_gen_sleep* peticion, t_paquete* paquete);
void deserializar_peticion_gen_sleep(t_peticion_gen_sleep* peticion, t_paquete* paquete);

void serializar_respuesta_gen_sleep(t_respuesta_gen_sleep* respuesta, t_paquete* paquete);
void deserializar_respuesta_gen_sleep(t_respuesta_gen_sleep* respuesta, t_paquete* paquete);

void serializar_desalojo(t_desalojo* desalojo , t_paquete* paquete);
void deserializar_desalojo(t_desalojo* desalojo , t_paquete* paquete);



void send_paquete(int* socket, t_paquete* paquete_ptr); 
void recv_paquete(int* socket, t_paquete* paquete_ptr); 

void free_paquete(t_paquete* paquete);
void free_nuevo_proceso(t_nuevo_proceso* np);
void free_pcb(t_pcb* pcb);
void free_desalojo(t_desalojo* desalojo);

t_registros* crear_registros_en_cero(void);

#endif