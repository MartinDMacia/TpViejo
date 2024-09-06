#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include<tipos.h>
#include<stdbool.h>
#include<paquetes.h>
#include<commons/log.h>


bool esperar_handshake(int* socket, t_handshake hanshake_esperado);
int esperar_cliente(int socket_servidor);
int iniciar_servidor(char* puerto);


#endif
