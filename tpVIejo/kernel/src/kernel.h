#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <../../utils/src/cliente.h>
#include <../../utils/src/servidor.h>
#include<pthread.h>
#include<regex.h>
#include<readline/readline.h>
#include<consola.h>

/*
typedef struct{
    pid;
    pc;
    quantum;
    registro_cpu

}t_pcb;
*/


void* io_manager(void* arg);
void* io_handler(void* arg);




#endif /* KERNEL_H_ */