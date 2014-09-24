/*
 * orquestador.h

 *  Created on: 05/10/2013
 *      Author: utnso
 */

#ifndef ORQUESTADOR_H_
#define ORQUESTADOR_H_

#include <stdio.h>
#include <commons/socket.h>
#include <commons/estructurasPackage.h>
#include <commons/string.h>
#include <commons/config.h>
#include <pthread.h>
#include <commons/log.h>
#include <sys/inotify.h>
#include <sys/stat.h>

#include "estructuras.h"

//#include "planificador.h"

typedef struct s_koopa {
	char * pathKoopa;
	char * archivoLeeKoopa;
} t_koopa;

t_log * loggerOrquestador;
int cerrarPlanificadores = 0;
t_koopa koopa;


/*
#define ORQ_DIRECCION "127.0.0.1"  //NO se usa mas
#define ORQ_PUERTO 5014 //No se usa mas.
#define ORQ_RUTACONFIG */
#define ORQ_RUTASINARCHIVO "../config/"
#define LD_LIBRARY "LD_LIBRARY_PATH=/home/utnso/Escritorio/tp-20131c-los-ultrapuertos/Proyectos/sharedLibraries/so-commons-library/Debug:/home/utnso/Escritorio/tp-20131c-los-ultrapuertos/Proyectos/sharedLibraries/so-nivel-library/Debug:/home/utnso/Escritorio/tp-20131c-los-ultrapuertos/Proyectos/tp/koopa/memoria/Debug"

//FUNCIONES PRINCIPALES
t_orquestador * crearOrquestador(char *ruta);
void actualizarQuantumGlobal(char *ruta);
void crearServidor(char * direccionIP, int puerto);
void accionNuevaConexion(epoll_data_t epollData);
void accionMeMandaronAlgo(epoll_data_t epollData);
void accionSeDesconecto(epoll_data_t epollData);
int existenRecursosEnNivel (t_list* self, char* nombreNivel, char* recursos );
void recibiMensajeDeNivel (t_referenciaNivel * nivel);
void obtenerYEnviar_PersonajeQueMuere(char* personajesInterbloqueados, t_referenciaNivel* nivel);
void obtenerYEnviar_RecursosAsignadosYSobrantes(char* recursosLiberados, t_referenciaNivel* nivel);
int esteNivelExiste (t_list *self,char * nombreNivel);
void crearHiloPlanificador(t_referenciaNivel * referenciaNivel);
t_referenciaPersonaje * getSimboloPersonajeMuere(char * personajesInterbloqueados, t_list * personajesBloqueados);
int estaDibujadoElPersonajeAPlanificar(t_referenciaNivel * estructuraNivel);

//FUNCIONES DE MANEJO DE ESTRUCTURAS
void agregarNivelALista(t_referenciaNivel * referenciaNivel);
void eliminarNivelDeLista(t_referenciaNivel * referenciaNivel);
t_struct_direcciones * armarStructDirecciones(char * dirPlanificador);
t_referenciaNivel * getNivelPorFd(int fd);
t_list * getPersonajesBloqueadosPorRecurso(t_list * listReferenciaRecurso, char simboloRecurso);
t_list * getPersonajesPorNivel(t_referenciaNivel * nivel);

//KO0PA
void hacerAccionesDeFinalDeJuego(void);
void mandarMensajeANivelParaCerrarlo(t_referenciaNivel * nivel);

//TEST!
void testObtenerPersonajeMuere();
void testObtenerRecursosAsignadosYSobrantes();

#endif /* ORQUESTADOR_H_ */
