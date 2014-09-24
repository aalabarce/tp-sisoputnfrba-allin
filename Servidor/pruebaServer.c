/*
 * pruebaServer.c
 *
 *  Created on: Sep 29, 2013
 *      Author: utnso
 */


#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <commons/socket.h>
#include <commons/estructurasPackage.h>
#include <commons/string.h>
#include <commons/config.h>
#include <pthread.h>
#include <commons/log.h>
#include <sys/inotify.h>
#include <sys/stat.h>


#define DIRECCION INADDR_ANY   //INADDR_ANY representa la direccion de cualquier
							   //interfaz conectada con la computadora
#define PUERTO 5000
#define BUFF_SIZE 1024


typedef struct {
	char * nombre;
	// char * direccionNivel;
	char * direccionPlanificador;
	int ordenLlegada;
	int quantum;
	int fd;
	pthread_t threadPlanificador;
	int fdPlanificadorEpoll;
	int fdPlanificadorServer;
	t_list * personajesListos;	// EL DATO VA ES DEL TIPO t_referenciaPersonaje
	pthread_mutex_t semaforoListos;
	t_list * personajesBloqueados;	//EL DATO VA A SER DEL TIPO t_referenciaRecurso
	t_link_element * personajeActual; //Este lo teniamos como suelto, lo agrego aca porque lo van a utilizar el planificador y orquestador.
	t_log * logger;
	int * cerrarPlanificador;
} t_referenciaNivel;


int main() {

	int socketEscucha, socketNuevaConexion;
	//int nbytesRecibidos;

	struct sockaddr_in socketInfo;
	// char buffer[BUFF_SIZE];
	int optval = 1;

	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	if ((socketEscucha = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &optval,
			sizeof(optval));

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = DIRECCION; //Notar que aca no se usa inet_addr()
	socketInfo.sin_port = htons(PUERTO);

// Vincular el socket con una direccion de red almacenada en 'socketInfo'.
	if (bind(socketEscucha, (struct sockaddr*) &socketInfo, sizeof(socketInfo))
			!= 0) {

		perror("Error al bindear socket escucha");
		return EXIT_FAILURE;
	}

// Escuchar nuevas conexiones entrantes.
	if (listen(socketEscucha, 10) != 0) {

		perror("Error al poner a escuchar socket");
		return EXIT_FAILURE;

	}

	printf("Escuchando conexiones entrantes.\n");

// Aceptar una nueva conexion entrante. Se genera un nuevo socket con la nueva conexion.
	if ((socketNuevaConexion = accept(socketEscucha, NULL, 0)) < 0) {

		perror("Error al aceptar conexion entrante");
		return EXIT_FAILURE;

	} else { printf("Aparezco cuando te conectas nivel!\n"); }

	while (1) {
/*

		// Recibir hasta BUFF_SIZE datos y almacenarlos en 'buffer'.
		if ((nbytesRecibidos = recv(socketNuevaConexion, buffer, BUFF_SIZE, 0))
				> 0) {

			printf("Mensaje recibido: ");
			fwrite(buffer, 1, nbytesRecibidos, stdout);
			printf("\n");
			printf("Tamanio del buffer %d bytes!\n", nbytesRecibidos);
			fflush(stdout);

			if (memcmp(buffer, "fin", nbytesRecibidos) == 0) {

				printf("Server cerrado correctamente.\n");
				break;

			}

		} else {
			perror("Error al recibir datos");
			break;
		}
		*/

		//int fdNuevaConexion=0;
		t_tipoEstructura tipo;
			void * estructura;

			int recibio = socket_recibir(socketNuevaConexion, &tipo, &estructura);
			if (!recibio) {
				if (estructura != NULL)
					free(estructura);
				printf("No recibi correctamente el envio");
			//	log_error(loggerOrquestador, "No recibi correctamente el envio del personaje o nivel");
							}

			t_struct_datosNivel * estructuraDatosNivel;

			// t_referenciaNivel * referenciaNivel;
			// t_signal signal;
			//char * nombreNivel;
			// int cerrarPlanificadores = 0;
			estructuraDatosNivel = (t_struct_datosNivel *) estructura;
			printf("Nombre: %s", estructuraDatosNivel->nombre);
			printf("Recursos: %s", estructuraDatosNivel->recursos);
						// referenciaNivel = crearReferenciaNivel(estructuraDatosNivel->nombre, estructuraDatosNivel->direccion, orquestador->direccionIp, fdNuevaConexion, estructuraDatosNivel->recursos);
						// log_info(loggerOrquestador, "Recibi del nivel %s, direccion: %s y recursos: %s", estructuraDatosNivel->nombre, estructuraDatosNivel->direccion, estructuraDatosNivel->recursos);

						// referenciaNivel->cerrarPlanificador = &cerrarPlanificadores;

						//agregarNivelALista(referenciaNivel);

						//crearHiloPlanificador(referenciaNivel);

						// log_info(loggerOrquestador, "Acabo de conectar al nivel %s", referenciaNivel->nombre);

						free(estructuraDatosNivel->nombre);
						free(estructuraDatosNivel->recursos);


	}


	close(socketEscucha);
	close(socketNuevaConexion);

	return EXIT_SUCCESS;
}
