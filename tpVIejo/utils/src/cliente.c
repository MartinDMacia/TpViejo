#include "cliente.h"

int crear_conexion(char* ip, char* puerto){
    struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return socket_cliente;
}



bool enviar_handshake(int socket, t_handshake handshake){
	int bytes =  sizeof(t_handshake);
	bool result;

	send(socket, &handshake, bytes, 0);
	recv(socket, &result, sizeof(bool), MSG_WAITALL);

	return result; //true->OK false->ERROR
}

void liberar_conexion(int socket_cliente){
	close(socket_cliente);
}

