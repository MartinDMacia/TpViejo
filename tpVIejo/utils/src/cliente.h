#ifndef CLIENTE_H_
#define CLIENTE_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<tipos.h>
#include<stdbool.h>
#include<paquetes.h>

int crear_conexion(char* ip, char* puerto);
bool enviar_handshake(int socket, t_handshake handshake);
void liberar_conexion(int socket_cliente);


#endif