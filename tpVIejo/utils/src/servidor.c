#include "servidor.h"

int esperar_cliente(int socket_servidor)
{
	int socket_cliente = accept(socket_servidor, NULL, NULL);

	return socket_cliente;
}

int iniciar_servidor(char* puerto){
    
    int socket_servidor;

	struct addrinfo hints;
    struct addrinfo *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);


	socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);

	return socket_servidor;
}


bool esperar_handshake(int* socket, t_handshake hanshake_esperado){
	int bytes = sizeof(t_handshake);
	t_handshake handshake_recibido;
	bool resultOK = true;
	bool resultERR = false;
	recv(*socket, &handshake_recibido, bytes, MSG_WAITALL);
	if(handshake_recibido == hanshake_esperado){
		send(*socket, &resultOK, sizeof(bool), 0);
		return resultOK;
	}
	else{
		send(*socket, &resultERR, sizeof(bool), 0);
		return resultERR;
	}
}



