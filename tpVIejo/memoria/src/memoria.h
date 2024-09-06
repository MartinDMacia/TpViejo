#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/config.h>
#include <string.h>
#include <commons/log.h>
#include <../../utils/src/servidor.h>
#include<pthread.h>
#include<variablesmemoria.h>


void* atender_cpu(void* arg);
void* atender_kernel(void* fd_keargrnel);
int guardar_nuevo_proceso(t_paquete* nuevo_proceso_empaquetado); //debe desempaquetar, deserializar, guardar el proceso y devolver 0 si salio bien
t_instruccion* str_to_instruccion(char* string);
inst_code str_to_inst_code(char* string);

#endif
