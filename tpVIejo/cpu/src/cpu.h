#ifndef CPU_H_
#define CPU_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <../../utils/src/servidor.h>
#include <../../utils/src/cliente.h>
#include<pthread.h>
#include<semaphore.h>
#include<instrucciones.h>
#include<stdbool.h>

void* receptor_de_interrupciones(void* arg);

#endif