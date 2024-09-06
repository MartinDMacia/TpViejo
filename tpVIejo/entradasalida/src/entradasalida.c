#include "entradasalida.h"

t_log* logger;

int main(void) {
    t_config* config;
    printf("Bienvenido al I/O creator!\n");
    char* nombre = readline("Ingrese el nombre de la I/O a iniciar:\n> ");
    char* path_config = readline("Ingrese el path del config:\n> ");

	config = config_create(path_config);
	if(config == NULL){
		printf("\nConfiguracion no encontrada.\n");
		return 1;
    }

	printf("\nConfiguracion creada.\n\n");
    
    logger = log_create("./entradasalida.log", "E/S", false, LOG_LEVEL_INFO);

    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");

    /*
    switch(tipo_interfaz){

    }
    */

    if(!strcmp(tipo_interfaz, "GENERICA")){
        int64_t tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO"); 
        //conectarse con kernel
        int fd_kernel = crear_conexion(ip_kernel, puerto_kernel);
        bool handshake_salio_bien = enviar_handshake(fd_kernel, IO_KE);
        if(handshake_salio_bien){
            printf("\nConectado con Kernel!\n");
        }
        //pasarle nombre y tipo (identificacion) a kernel
        t_identificacion_io* identificacion = malloc(sizeof(t_identificacion_io));
        identificacion->nombre = nombre;
        identificacion->tipo = tipo_interfaz;
        t_paquete* id_empaquetada = malloc(sizeof(t_paquete));
        
        serializar_identificacion_io(identificacion, id_empaquetada);
        
        send_paquete(&fd_kernel, id_empaquetada);
        
        //aca podria esperar un ok de kernel quizas

        while(1){
            //esperar a recibir una peticion de io desde kernel
            t_paquete* peticion_empaquetada = malloc(sizeof(t_paquete));
            recv_paquete(&fd_kernel, peticion_empaquetada);
            
            t_peticion_gen_sleep* peticion_recibida = malloc(sizeof(t_peticion_gen_sleep));
            deserializar_peticion_gen_sleep(peticion_recibida, peticion_empaquetada);
            log_info(logger, "PID: %d - Operacion: IO_GEN_SLEEP", peticion_recibida->pid);
            //procesarla
            t_temporal* reloj = temporal_create();
            while( temporal_gettime(reloj) < tiempo_unidad_trabajo * peticion_recibida->unidades_de_sleep ){
                printf("%ld\n", temporal_gettime(reloj));
            }
            temporal_destroy(reloj);
            //enviarla
            t_respuesta_gen_sleep respuesta = 1; //da lo mismo lo que mande
            t_paquete* paquete_a_enviar = malloc(sizeof(t_paquete));
            serializar_respuesta_gen_sleep(&respuesta, paquete_a_enviar);
            send_paquete(&fd_kernel, paquete_a_enviar);
            free(peticion_recibida);
        }
    }

}