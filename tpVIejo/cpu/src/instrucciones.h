#ifndef INSTRUCCIONESH
#define INSTRUCCIONESH

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <../../utils/src/servidor.h>
#include <../../utils/src/cliente.h>
#include<variablescpu.h>


    // MANDA EL CONTEXTO DE EJECUCION A LA CPU CON EL MOTIVO DE DESALOJO Y DATOS ADICIONALES SI LO NECESITA
    void ejecutar_exit(int* fd, t_pcb* pcb);

    // MANDA EL CONTEXTO DE EJECUCION A LA CPU CON EL MOTIVO DE DESALOJO Y DATOS ADICIONALES SI LO NECESITA
    void ejecutar_io_gen_sleep(int* fd, t_pcb* pcb, t_instruccion* instruccion);

    //EJECUTAR WAIT
    void ejecutar_wait(int* fd, t_pcb* pcb, t_list* argumentos);

    //EJECUTAR SIGNAL
    void ejecutar_signal(int* fd, t_pcb* pcb, t_list* argumentos);

    // DEVUELVE EL valor del REGISTRO QUE LE INDIQUE
    int obtener_valor_registro(char* nombre_registro, t_registros* registros_cpu);

    //cambia el valor del registro
    void cambiar_valor_registro(char* nombre_registro, t_registros* registros_cpu, int valor);

    // SUMA EL REGISTRO DESTINO con el REGISTRO ORIGEN Y LUEGO GUARDA EL RESULTADO EN EL REGISTRO DESTINO
    void ejecutar_sum(t_registros* registros_cpu, t_list* argumentos);

    // ASIGNA EN EL REGISTRO UN VALOR
    void ejecutar_set(t_registros* registros_cpu, t_list* argumentos);

    // RESTA EL REGISTRO DESTINO con el REGISTRO ORIGEN Y LUEGO GUARDA EL RESULTADO EN EL REGISTRO DESTINO
    void ejecutar_sub(t_registros* registros_cpu, t_list* argumentos);

    // SI EL VALOR DEL REGISTRO ES DIFERENTE A CERO, ACTUALIZA EL PC AL NUMEROD DE LA INSTRUCCION PASADA
    void ejecutar_jnz(t_registros* registros_cpu, t_list* argumentos);

#endif