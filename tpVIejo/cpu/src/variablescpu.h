#ifndef VARIABLESCPU_H_
#define VARIABLESCPU_H_

#include<stdbool.h>
#include<semaphore.h>
#include<tipos.h>
#include<commons/log.h>

extern bool hay_interrupcion;
extern sem_t mx_hay_interrupcion;

extern t_registros* registros_cpu;

extern t_log* logger;
extern t_log* debugger;


#endif