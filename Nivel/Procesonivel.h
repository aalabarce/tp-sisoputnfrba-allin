/*
 * nivel.h
 *
 *  Created on: Oct 2, 2013
 *      Author: utnso
 */

#ifndef NIVEL_H_
#define NIVEL_H_



#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <curses.h>
#include <sys/inotify.h>




#include <commons/config.h>
#include <commons/string.h>
#include <commons/socket.h>
#include <commons/log.h>
#include <commons/estructurasPackage.h>
#include <commons/collections/list.h>
#include <nivel.h>
#include <tad_items.h>



/*HARDCODEOS*/
int VERTICAL = 1;
#define HORIZONTAL 0;
#define DIRECCION INADDR_ANY
#define PUERTO 5031
#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )
#define BUF_LEN     ( 1024 * EVENT_SIZE )
#define NIVEL_RUTASARCHIVO "../"

	typedef struct caja {
		char* nombre;
		char simbolo;
		unsigned int instanciasTotales; //instancias totales del nivel
		unsigned int instanciasActuales;//instancias disponibles al momento
		unsigned int posX;
		unsigned int posY;
	}  t_caja;

	typedef struct personaje {
		char simbolo;
		unsigned int posX;
		unsigned int posY;
		t_list *recursos;	//Lista de t_recursoPersonaje
		char recursoBuscadoActual; //me sirve para DeadLock
		int ordenDeLlegada;
	}  t_personaje;

	typedef struct recursoPersonaje {
		char simboloRecurso;
		unsigned int cantRecursos;
	}  t_recursoPersonaje;

	typedef struct nivel {
		char *nombre;
		t_list *cajasDisponibles; //Lista de t_caja
		unsigned int limiteX;
		unsigned int limiteY;
		char* orquestador;
		int fdOrquestador;
		unsigned int enemigos;
		unsigned int sleepEnemigos;
		int fdNivelServer; // fd del "servidor" que va a estar recibiendo pedidos del perosnaje
		int fdEpoll;
		unsigned int tiempoDeadLock;
		unsigned int recovery;
		char *algoritmo;
		unsigned int quantum;
		unsigned int retardo;
		int fdInotify;
		int inotifyWatch;
		char * rutaConfig;
	} t_nivel;

	typedef struct pos_enemigo
	{
	   int posicion;
	   int random_number;
	 } t_posicion_enemigo;

	typedef struct crear_enemigo
		{
		   int iteracion;
		   t_list* ListaItems;
		 } t_crear_enemigo;

	typedef struct mover_enemigo
		{

		 	int posicion_x;
		 	int posicion_y;
		 	char simbolo;
		 	int ultimoMovimiento;
		 } t_mover_enemigo;


	typedef struct posicion_caja
		{
			int posicion_x;
			int posicion_y;
		 } t_posicion_caja;

	typedef struct coords {
		int x;
		int y;
	} coords;

t_nivel* nivel;
t_list* personajesActuales;
t_list* ListaItems;
int max_filas, max_cols;
t_log * logger;


/*PROTOTIPOS*/


	t_nivel* crearNivel(char* ruta);
	t_list * crearCajasDisponibles(t_config *config, unsigned int cantidadCajas); //usado en crearNivel
	int validarConfigNivel(t_config *);
	void accionMeMandaronAlgo(epoll_data_t epollData);
	void accionSeDesconecto(epoll_data_t epollData);
	void* lanzarEPoll ();
	void conectarOrquestador(void);
	void crearEnemigo(void* enemigo);
	void agregarListaItemsCajas();
	void agregarListaEnemigos ();
	void* moverEnemigos(void* arg);
	t_posicion_enemigo moverseEnX(int max_filas, int pos_x);
	int moverseEnY(int valor_x, int max_cols, int pos_y);
	int cantidad_de_algo(char* conf, char* algo);
	void *algoritmoMueveEnemigos(void* arg);
	int hayCajaEnPosicion(int x, int y);
	void mandarAOrquestadorPersonajesInterbloqueados(char * personajesInterbloqueados);
	void imprimirVector(int* vec, int cant, char* desc);
	void * detectarInterbloqueo(void * loQueSea);
	void BorrarPersonajeActual(t_list* personajes, char simbolo);

#endif /* NIVEL_H_ */
