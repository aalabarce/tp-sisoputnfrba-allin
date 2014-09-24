/*
 * Cliente.c
 *
 *  Created on: 21/04/2013
 *      Author: utnso
 */



#include "cliente.h"

int main(){

	int socketFd, seEnvio;
	char* mensaje = malloc(BUFF_SIZE); // ACÁ SE VAN A GUARDAR LOS CARACTERES A ENVIAR AL SERVER (HASTA 1024)
	char* nombre = malloc(30);
	pthread_t thrRecibirMensaje;
	char *direccionIP = malloc(sizeof(DIRECCION));
	*direccionIP = DIRECCION;
	t_estructura0 *mensajeEnviar = malloc(sizeof(t_estructura0));

	printf("Su nombre: ");
	gets(nombre);

	mensajeEnviar->nombre = string_duplicate(nombre);
	free(nombre);

	printf("Conectando por el puerto %d, espere...\n",PUERTO);

	socketFd = socket_crearYConectarCliente(direccionIP, PUERTO);
	if (socketFd == -1)
		exit(1);

	//Crea un hilo para recibir mensajes del servidor
	pthread_create(&thrRecibirMensaje, NULL, (void*) RecibirMensajesServidor, (void *) &socketFd);

	mensajeEnviar->mensaje = string_duplicate("Hola, soy nuevo aca!");

	seEnvio = socket_enviar(socketFd, 0, mensajeEnviar);
	if(!seEnvio){
		perror("Server no encontrado\n");
	}

	free(mensajeEnviar->mensaje);


	printf("Conexión establecida!\n"); //A PARTIR DE ACÁ, SE VAN A ESPERAR LOS CARACTERES A ENVIAR


	while (1){
		printf("\nYO: ");
		gets(mensaje);

		mensajeEnviar->mensaje = string_duplicate(mensaje);

		seEnvio = socket_enviar(socketFd, 0, mensajeEnviar);
		if(!seEnvio){
			perror("Server no encontrado\n");
		}

		free(mensajeEnviar->mensaje);
	}

	pthread_join(thrRecibirMensaje,NULL);
	close(socketFd);
	free(direccionIP);
	return 0;
}

//Funcion recibir mensajes de servidor


void * RecibirMensajesServidor(void *socketfd) {
	int bytesRecibidos;
	t_tipoEstructura tipoRecibido;
	void* estructuraRecibido;
	t_estructura0 *mensaje;

	while (1) {
		bytesRecibidos = socket_recibir(*(int *)socketfd, &tipoRecibido, &estructuraRecibido);
		if(!bytesRecibidos){
			perror("\nErrorEE al recibir datos\n");
			break;
		}


		if (tipoRecibido == 0) {
			mensaje = (t_estructura0 *) estructuraRecibido;
		}
		else {
			perror("\nRecibi otra cosa que no era estructura 0 en un mensaje");
			exit(1);
		}

		printf("\n%s: %s", mensaje->nombre, mensaje->mensaje);

		printf("\nYO: ");
		fflush(stdout);//DESCARTO EL STREAM QUE GUARDE EN MEMORIA
	}

	return NULL;
}
