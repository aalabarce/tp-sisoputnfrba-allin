/*
 * Header.h
 *
 *  Created on: 21/04/2013
 *      Author: utnso
 */

#ifndef HEADER_H_
#define HEADER_H_

#include <stdlib.h>
#include <stdio.h>
#include "../commons/socket.h"
#include "../commons/package.h"
#include <pthread.h>

typedef struct {
	char * nombre;
	char * mensaje;
} t_estructura0;

#define PUERTO 5451 //ESTE ES EL PUERTO AL QUE NOS VAMOS A CONECTAR
#define DIRECCION INADDR_ANY //ESTA CONSTANTE ES 0, SIGNIFICA QUE EL S.O. ME VA A DAR MI DIRECCIÓN IP
#define BUFF_SIZE 1024 // TAMAÑO MÁXIMO DE BYTES QUE ALMACENO EN EL BUFFER DEL SOCKET

void * RecibirMensajesServidor(void *socketfd) ;

#endif /* HEADER_H_ */
