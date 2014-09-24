#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>

#include <commons/config.h>
#include <commons/socket.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/estructurasPackage.h>
#include <commons/collections/list.h>


/*
 * Estructura de t_objetivoNivel
 * Contiene:
 *                 - simbolo (char)
 * Comentarios: Ya se que es al pedo crear una estructura para una sola variable, pero me parece que va a ser mas ordenado...
 * Puede ser que en otro momento se agrege mas info y no solo el simbolo. (como el nombre del objetivo)->(me pa que eso es al pedo igual)
 * Se lo va a llamarde esta forma: objetivo->simbolo, y no objetivo directamente. Me parece que es mas claro.
 */
struct objetivoNivel {
        char simbolo;
};
typedef struct objetivoNivel t_objetivoNivel;
typedef struct nivelPersonaje {
        char* nombre;
        t_list *objetivosNivel;
        int fdPlanificadorNivel;
        int personajeSigueJugando;
        int tengoPendienteAgregarRecurso;
        int tengoQuePedirPosicionRecursos;
        int flag;
        t_objetivoNivel * primerObjetivo;
        int reinicio;
}t_nivelPersonaje;

/*int personajeSigueJugando = 1;
                int tengoPendienteAgregarRecurso = 0;
                int tengoQuePedirPosicionRecursos = 1;
                int flag = 1;
 */


typedef struct nivelActual {
        t_link_element *nivel;
        int fdNivel;
        int fdPlanificador;
}t_nivelActual;

struct objetivoActual {
        t_link_element *objetivo;
        unsigned int posX;
        unsigned int posY;
};

typedef struct objetivoActual t_objetivoActual;

struct personaje {
        char *nombre;
        char simbolo;
        int cantVidas;
        int cantVidasOriginales;
        unsigned int posX;
        unsigned int posY;
        t_list *planDeNiveles;
        t_nivelActual nivelActual;
        t_objetivoActual objetivoActual;
        char *orquestador;
        int fdPersonaje;
        char* path;
        t_list * personajes;
};
typedef struct personaje t_personaje;


typedef struct personajeAux {
        char *nombre;
        char simbolo;
        int cantVidas;
        int cantVidasOriginales;
        unsigned int posX;
        unsigned int posY;
        t_nivelPersonaje* nivelActual;
        t_objetivoActual objetivoActual;
        char *orquestador;
        int fdPersonaje;
        char* path;
        int ultimoMovimiento;
}t_personajeAux;

/* ESTRUCTURAS AUXILIARES */
typedef struct {
        unsigned int posX;
        unsigned int posY;
} t_posicion;
t_personaje* personaje;


/* VARIABLES GLOBALES Y SEMAFOROS*/
pthread_mutex_t mPersonaje = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mVidas = PTHREAD_MUTEX_INITIALIZER;
t_log * logger;
int VERTICAL = 1;
int HORIZONTAL  = 0;
pthread_t hilo[100];
char * path;

/* LLEGO LA HORA DE LOS PROTOTIPOS! */

t_personaje* crearPersonaje(char * ruta);
t_list * crearPlanDeNiveles(t_config * config);
t_list * crearObjetivosNivel(t_config * config, char * nombreNivel);

void destruirPersonaje(void);
void destruirNivel(t_nivelPersonaje *);
void destruirDireccionesNivel(t_struct_direcciones *);


int validarConfigPersonaje(t_config *);
void* jugarNivel(void * estructura);
t_personajeAux* cambiarNivel(int fd);
void cambiarObjetivo(t_personajeAux* persaux);
void reiniciarNivel(void);
void reiniciarJuego(void);
int terminoNivel(t_personajeAux* persaux);
int terminoJuego(t_personajeAux* persaux);

void quitarVida(void);
void aumentarVida(void);
void manejarControlC();

int estaEnObjetivo(t_personajeAux* persaux);

void guardarPosObjetivoActual(t_posicion posicionXY);
void guardarFdNivelYPlanificador(int fdNivel, int fdPlanificador);

/* PROTOTIPOS AUXILIARES */

char* crear_stream(t_list* list);
char* crear_streamPersonaje();
t_link_element *getPrimerObjetivo(void);
t_nivelPersonaje *dataNivelActual(void);
t_objetivoNivel *dataObjetivoActual(t_personajeAux* persaux);

char* nombreNivelActual(void);
char simboloObjetivoActual(void);
t_list* listaObjetivosNivelActual(void);

/*FUNCIONES GROSAS*/

void logicaPlanificadorMoverPersonaje(int * punteroPersonajeSigueJugando, int* punteroTengoPendienteAgregarRecurso, t_personajeAux* persaux);
void logicaPlanificadorMatarPersonaje(void);

void avisarOrquestadorIniciaJuego(void);
void comunicarseConOrquestador(t_personajeAux* nivelEnJuego);
int PasarPlanNiveles(void);
void conectarseNivelYPlanificador(t_struct_direcciones * direcciones);
void conectarseOrquestadorNivelYPlanificador(void);

void avisarNivelReinicio(void);
void avisarPlanificadorMeMovi(void);
void avisarPlanificadorMeBloquie(void);
void avisarPlanificadorMeMoviYTomeRecurso(void);
void finalizacionInesperada(void);
void avisarOrquestadorTermineJuego(void);
void avisarOrquestadorTermineJuegoMal(void);
t_signal recibirPlanificadorSignal(t_personajeAux* nivelEnJuego);

int autoMover(t_personajeAux* nivelEnJuego);
void mandarPosicionAMoverAPlanificador(t_posicion * posxposy,t_personajeAux* nivelEnJuego);
void recibirSignalMoviCorrectamente(t_personajeAux* nivelEnJuego);

void desconectarseNivelActual(t_personajeAux* nivelEnJuego);
int pedirNivelRecurso(t_personajeAux* nivelEnJuego);
void pedirNivelPosicionRecurso(int fdDeNivel,t_personajeAux* nivelEnJuego);
void matarPersonaje(void);

void *matarPersonajePorSignal(void*);
void *aumentarVidaPorSignal(void*);

void crearThreadMatarPersonajePorSignal(void);
void crearThreadAumentarVidaPorSignal(void);

void accionMeMandaronAlgo(epoll_data_t epollData);
void accionSeDesconecto(epoll_data_t epollData);
t_nivelPersonaje * getNivel_Personaje(char* nombreNivel);
t_nivelPersonaje* getNivel_PersonajexFD(int fd);
t_link_element * getNivelPorFd2(int fd);
t_nivelActual * getNivel2(char* nombreNivel);
