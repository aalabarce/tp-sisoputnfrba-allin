/*
 * planificador.h
 *
 *  Created on: 24/05/2013
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "estructuras.h"
#include <stdio.h>
#include <commons/socket.h>
#include <commons/estructurasPackage.h>
#include <commons/string.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>

int terminoJuego;


/* FUNCIONES ! */
void * mainPlanificador(void * referenciaNivel);

void crearServidorPlanif(t_referenciaNivel* nivel);
void cerramosTodo(t_referenciaNivel * nivel);
char* eliminarUnaPosicionDeUnString (char* stringRecursosLiberados, int posicionAQuitar);
char* quitarRecursoDeRecusosLiberados (char* stringRecursosLiberados ,char RecursoAEliminar );

void buscaEnListasYLoBorra(char simbolo, t_referenciaNivel * estructuraNivel);
void gestionarConexionesConTimeOut(t_referenciaNivel  * nivel);
void matarPersonajesInterbloqueados(t_referenciaNivel * nivel);
void accionNuevaConexionPlanificador(epoll_data_t epollData, t_referenciaNivel* estructuraNivel);
void accionCerroConexionPlanificador(epoll_data_t epollData, t_referenciaNivel* estructuraNivel);void accionReiniciarPersonaje(int fdPersonaje, t_referenciaNivel* estructuraNivel);

void enviarSignalEsTuTurnoPersonaje(t_referenciaPersonaje * fdPersonaje, t_referenciaNivel* estructuraNivel);
void enviarSignalMovetePersonaje(t_referenciaPersonaje * personajeActual, t_referenciaNivel* estructuraNivel);

void chequearAlgoritmo(t_referenciaNivel * estructuraNivel);

void avisarPersonajeQueLoMovi(t_referenciaPersonaje* personaje, t_referenciaNivel * estructuraNivel);
void accionPersonajeSeBloqueo(t_referenciaNivel* nivel, char recursoBloqueante);
void accionPersonajeSeMovio(t_referenciaNivel* nivel);
void accionPersonajeSeMovioPosta(t_referenciaNivel* estructuraNivel,t_signal signal);
void accionNivelUnPersonajeSeMovio(t_referenciaNivel* estructuraNivel,t_referenciaPersonaje * personajeActual, t_signal signal);
void accionPersonajeSeMovioYTomoRecurso(t_referenciaNivel* estructuraNivel);

void cambiarQuantumYCambiarPersonaje(t_referenciaNivel* estructuraNivel);
int estaDibujadoElPersonajeAPlanificar(t_referenciaNivel * estructuraNivel);
t_referenciaNivel * acomodarListaDeListos(t_referenciaNivel * nivel);
int gestionarConexiones(t_referenciaNivel * estructuraNivel);
void gestionarNuevasConexiones(t_referenciaNivel * estructuraNivel);
int posicionPersonajeEnLaLista (t_list *self,char  simboloPersonaje);
void pasarARoundRobin(t_referenciaNivel * nivel);
void liberarAlgunPersonajeBloqueado(char * recursosLiberados,t_referenciaNivel* nivel);

t_referenciaPersonaje* getPersonajePorSimbolo2(char simboloPersonaje,t_referenciaNivel * estructuraNivel);


void logearListas(t_referenciaNivel * estructuraNivel);

#endif /* PLANIFICADOR_H_ */
