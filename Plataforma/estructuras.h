/*
 * utilidades.h
 *
 *  Created on: 05/05/2013
 *      Author: utnso
 */

#ifndef UTILIDADES_H_
#define UTILIDADES_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <pthread.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )
#define BUF_LEN     ( 1024 * EVENT_SIZE )

/*DEFINO ESTRUCTURAS GLOBALES*/

typedef struct {
	char * direccionIp;
	t_list * listaDeNiveles;
	int cantSegundosEspera;
	//int quantumGlobal;
	int fdOrquestadorServer;
	int fdOrquestadorEpoll;
	int cantPersonajesJugando;
	pthread_t hilosPlanificadores[100];
	char * rutaConfig;
} t_orquestador;

typedef struct {
	char simbolo;
	int fd;
	int ordenLlegada;
	int posPersX;
	int posPersY;
	char recursoActual;
	int posRecX;
	int posRecY;
	char * recursosAsignados;
} t_referenciaPersonaje;

typedef struct {
	char * nombre;
	char * direccionPlanificador;
	int ordenLlegada;
	int quantum;
	char * algoritmo;
	int quantumGlobal;
	unsigned int retardo;
	char * recursos;
	int fd;
	pthread_t threadPlanificador;

	int fdPlanificadorEpoll;
	int fdPlanificadorServer;
	t_list * personajesDibujados;
	t_list * personajesListos; // EL DATO VA ES DEL TIPO t_referenciaPersonaje
	pthread_mutex_t semaforoListos;
	pthread_mutex_t semaforoBloqueados;
	t_list * personajesBloqueados; //EL DATO VA A SER DEL TIPO t_referenciaRecurso
	t_link_element * personajeActual; //Este lo teniamos como suelto, lo agrego aca porque lo van a utilizar el planificador y orquestador.
	t_log * logger;
	int * cerrarPlanificador;
	t_referenciaPersonaje * personajeAPlanificar;
	t_list * personajesInterbloqueados;
} t_referenciaNivel;
pthread_mutex_t sem1;

typedef struct {
	char simbolo;
	t_list * personajesBloqueados; // EL DATO VA A SER DEL TIPO t_referenciaPersonaje
	pthread_mutex_t semaforoBloqueados;
} t_referenciaRecurso;

typedef struct {
	char simbolo;
	int fdPersonaje;
	int distanciaTotal;
}t_comparaPersonajes;

typedef struct{
	t_list* lista;
	pthread_mutex_t mutex;
}t_lista;





//Estructuras globales

t_orquestador * orquestador; //Estructura de orquestador global.


/*PROTOTIPOS DE FUNCIONES*/

//t_referenciaNivel * crearReferenciaNivel(char* nombre, char* ipOrquestador, int fd, char * recursos);
void lista2_add(t_lista* lista, void* personaje);
t_lista* lista2_create();
t_referenciaNivel * crearReferenciaNivel(char* nombre, char* ipOrquestador, int fd, char * recursos, char * algoritmoRecibido, int quantumGlobalRecibido, int retardoRecibido);
t_referenciaPersonaje * crearReferenciaPersonaje(char simbolo, int fd, int * ordenLlegada);
t_referenciaRecurso * crearReferenciaRecurso(char simboloRecurso);

void destruirReferenciaNivel(t_referenciaNivel * nivel);
void destruirReferenciaPersonaje(t_referenciaPersonaje * personaje);
void destruirReferenciaRecurso(t_referenciaRecurso * referenciaRecurso);

void agregarPersonajeAListos(t_referenciaNivel * nivel, t_referenciaPersonaje * personaje);
void agregarPersonajeABloqueado(t_referenciaNivel * nivel, t_referenciaPersonaje * personaje, char recursoBloqueante);

t_referenciaRecurso * getReferenciaRecurso(t_referenciaNivel* nivel, char recursoBloqueante);
t_referenciaPersonaje * getPersonajeActual(t_referenciaNivel *nivel);
t_referenciaNivel * getNivel(char* nombreNivel);        // SIRVE PARA LISTA DE t_referenciaNivel

t_referenciaPersonaje * getPersonajePorFd(t_list * lista, int fdPersonaje);
t_referenciaPersonaje * getPersonajePorSimbolo(t_list * lista, char simbolo);
t_list * buscarUnPersonajeEnTodasLasListas(t_referenciaNivel * referenciaNivel, int fdPersonaje, int * index);

t_referenciaPersonaje * sacarPersonajeDeLista(t_list * lista, char simboloPersonaje);
t_referenciaPersonaje * sacarPersonajeDeListaListos(t_referenciaNivel * nivel, char simboloPersonaje);

void cambiarPersonajeListosABloqueado(t_referenciaNivel * nivel, char simboloPersonaje, char recursoBloqueante);
void cambiarPersonajeBloqueadoAListo(t_referenciaNivel * nivel, char simboloPersonaje, char recursoBloqueante);

void siguientePersonajeActual(t_referenciaNivel * nivel);

#endif /* UTILIDADES_H_ */
