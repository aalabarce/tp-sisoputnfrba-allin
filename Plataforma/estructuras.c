/*
 * utilidades.c
 *
 *  Created on: 05/05/2013
 *      Author: utnso
 */

#include "estructuras.h"

//t_referenciaNivel * crearReferenciaNivel(char* nombre, char* ipOrquestador, int fd, char * recursos){

t_referenciaNivel * crearReferenciaNivel(char* nombre, char* ipOrquestador, int fd, char * recursos, char * algoritmoRecibido, int quantumGlobalRecibido, int retardoRecibido){

        t_referenciaNivel *nuevoNivel = malloc(sizeof(t_referenciaNivel));

        nuevoNivel->nombre = string_duplicate(nombre);
        nuevoNivel->direccionPlanificador = string_duplicate(ipOrquestador);
        nuevoNivel->fd = fd;
        nuevoNivel->quantum = 1;
        nuevoNivel->ordenLlegada = 1;   // ¿PARA QUé ES ESTO?
        nuevoNivel->algoritmo = string_duplicate(algoritmoRecibido);
        nuevoNivel->quantumGlobal = quantumGlobalRecibido;
        nuevoNivel->retardo = retardoRecibido;
        nuevoNivel->recursos = string_duplicate(recursos);

        nuevoNivel->personajesDibujados = list_create ();
        nuevoNivel->personajeActual = NULL;     // 89APUNTA A EL PROXIMO A SER EJECUTADO EN LA LISTA DE LISTOS. INICIALIZA EN NULL.
        pthread_mutex_lock(&sem1);
        nuevoNivel->personajesListos = list_create();
        pthread_mutex_unlock(&sem1);
        pthread_mutex_init(&(nuevoNivel->semaforoListos), NULL);
        nuevoNivel->personajesBloqueados = list_create();



        return nuevoNivel;
}

t_referenciaRecurso * crearReferenciaRecurso(char simboloRecurso){
        t_referenciaRecurso * referenciaRecurso = malloc(sizeof(t_referenciaRecurso));
        referenciaRecurso->simbolo = simboloRecurso;
        referenciaRecurso->personajesBloqueados = list_create();
        pthread_mutex_init(&referenciaRecurso->semaforoBloqueados, NULL);
        return referenciaRecurso;
}

t_referenciaPersonaje * crearReferenciaPersonaje(char simbolo, int fd, int * ordenLlegada){
        t_referenciaPersonaje * nuevoPersonaje = malloc(sizeof(t_referenciaPersonaje));

        nuevoPersonaje->simbolo = simbolo;
        nuevoPersonaje->fd = fd;
        nuevoPersonaje->ordenLlegada = *ordenLlegada;

        (*ordenLlegada)++;      //AUMENTA EL ORDEN DE LLEGADA!

        return nuevoPersonaje;
}


void destruirReferenciaNivel(t_referenciaNivel * nivel){
        free(nivel->nombre);
        free(nivel->direccionPlanificador);
        list_destroy_and_destroy_elements(nivel->personajesListos, (void*) destruirReferenciaPersonaje);
        list_destroy_and_destroy_elements(nivel->personajesBloqueados, (void*) destruirReferenciaRecurso);
        free(nivel);
}


void destruirReferenciaPersonaje(t_referenciaPersonaje * personaje){
        free(personaje);
}

void destruirReferenciaRecurso(t_referenciaRecurso * referenciaRecurso){
        list_destroy_and_destroy_elements(referenciaRecurso->personajesBloqueados, (void*) destruirReferenciaPersonaje);
        free(referenciaRecurso);
}

void agregarPersonajeAListos(t_referenciaNivel * nivel, t_referenciaPersonaje * personaje){

        //ESTA FUNCION LA HAGO ACA ADENTRO, PORQUE NECESITO EL PERSONAJE ACTUAL Y ESTO NO SE PUEDE MANDAR POR LA FUNCION DE GET INDEX.
        _Bool esPersonajeActual(void * element){
                t_referenciaPersonaje * personajeDeLista = (t_referenciaPersonaje *) element;
                return (personajeDeLista->simbolo == getPersonajeActual(nivel)->simbolo );
        }
        pthread_mutex_lock(&nivel->semaforoListos);
                // ACA EMPIEZA agregarPersonajeListos
                if(getPersonajeActual(nivel) != NULL) {
                        int index = list_get_index(nivel->personajesListos, esPersonajeActual);
                        if (index == -1){       //SI NO ENCUENTRA AL PERSONAJE ACTUAL (O PASE MAL POR ARGUMENTO O NO SE QUE PUDO PASAR) O LA LISTA ES VACIA
                                list_add(nivel->personajesListos, (void*) personaje);
                        }
                        else {
                                list_add_in_index(nivel->personajesListos, index,(void*) personaje);    // Lo agrega en la cola, una posicion antes que el que se va a ejecutar en el siguiente momento, esto hace que sea el ultimo a ejecutarse
                        }
                }
                else {
                        list_add(nivel->personajesListos, (void*) personaje);
                }
                if(nivel->personajeActual == NULL) {
                        nivel->personajeActual = nivel->personajesListos->head;         //Agrega automaticamente a el personaje si la lista de listos estaba vacia.
                }
        pthread_mutex_unlock(&nivel->semaforoListos);
}


void agregarPersonajeABloqueado(t_referenciaNivel * nivel, t_referenciaPersonaje * personaje, char recursoBloqueante){
        t_referenciaRecurso * referenciaRecurso = getReferenciaRecurso(nivel, recursoBloqueante);
        pthread_mutex_lock(&referenciaRecurso->semaforoBloqueados);
                list_add(referenciaRecurso->personajesBloqueados, (void*)personaje);
        pthread_mutex_unlock(&referenciaRecurso->semaforoBloqueados);
}

t_referenciaPersonaje * getPersonajeActual(t_referenciaNivel *nivel){
        if (nivel->personajeActual != NULL){
                return (t_referenciaPersonaje*) nivel->personajeActual->data;
        }
        else {
                return NULL;
        }
}

t_referenciaRecurso * getReferenciaRecurso(t_referenciaNivel* nivel, char recursoBloqueante){
        _Bool simboloEsIgual(void * element){
                t_referenciaRecurso * referenciaRecurso = (t_referenciaRecurso*) element;
                return (referenciaRecurso->simbolo == recursoBloqueante);
        }

        t_referenciaRecurso * referenciaRecursoBloqueante = list_find(nivel->personajesBloqueados, simboloEsIgual);

        return referenciaRecursoBloqueante;
}

t_referenciaPersonaje * sacarPersonajeDeLista(t_list * lista, char simboloPersonaje){
        //ESTA FUNCION LA HAGO ACA ADENTRO, PORQUE NECESITO EL PERSONAJE  Y ESTO NO SE PUEDE MANDAR POR LA FUNCION DE remove by condition.
        _Bool esPersonajeIgual(void * element){
                t_referenciaPersonaje* personajeDeLista = (t_referenciaPersonaje*) element;
                return ( personajeDeLista->simbolo == simboloPersonaje);
        }

        // ACA EMPIEZA sacarPersonajeDeLista
        return list_remove_by_condition(lista, esPersonajeIgual); //Lo saca de la lista y devuelve el data.
}

t_referenciaPersonaje * sacarPersonajeDeListaListos(t_referenciaNivel * nivel, char simboloPersonaje){
        if( getPersonajeActual(nivel)->simbolo == simboloPersonaje ) {
                siguientePersonajeActual(nivel);
        }

        pthread_mutex_lock(&nivel->semaforoListos);

                t_referenciaPersonaje * personaje = sacarPersonajeDeLista(nivel->personajesListos, simboloPersonaje);

                if(nivel->personajesListos->elements_count == 0) {
                        nivel->personajeActual = NULL;
                }
        pthread_mutex_unlock(&nivel->semaforoListos);
        return personaje;
}


void cambiarPersonajeListosABloqueado(t_referenciaNivel * nivel, char simboloPersonaje, char recursoBloqueante){
        t_referenciaPersonaje * personaje = sacarPersonajeDeListaListos(nivel, simboloPersonaje);

        agregarPersonajeABloqueado(nivel, personaje, recursoBloqueante);
}


void cambiarPersonajeBloqueadoAListo(t_referenciaNivel * nivel, char simboloPersonaje, char recursoBloqueante){

                t_referenciaRecurso * referenciaRecurso = getReferenciaRecurso(nivel, recursoBloqueante);
                pthread_mutex_lock(&referenciaRecurso->semaforoBloqueados);
                        t_referenciaPersonaje * personaje = sacarPersonajeDeLista(referenciaRecurso->personajesBloqueados, simboloPersonaje);
                pthread_mutex_unlock(&referenciaRecurso->semaforoBloqueados);
                agregarPersonajeAListos(nivel, personaje);
}


t_referenciaNivel * getNivel(char* nombreNivel){
        //ESTA FUNCION LA HAGO ACA ADENTRO, PORQUE NECESITO EL NOMBRE DEL NIVEL  Y ESTO NO SE PUEDE MANDAR POR LA FUNCION DE LIST FIND.
        _Bool esNivelIgual(void * element){
                t_referenciaNivel* nivelDeLista = (t_referenciaNivel*) element;
                return ( strcmp(nombreNivel, nivelDeLista->nombre) == 0);
        }

        return (t_referenciaNivel*) list_find(orquestador->listaDeNiveles, esNivelIgual);
}

t_referenciaPersonaje * getPersonajePorFd(t_list * lista, int fdPersonaje) {
        _Bool esFdPersonajeIgual(void * element){
                t_referenciaPersonaje* personajeActual = (t_referenciaPersonaje *) element;

                return (personajeActual->fd == fdPersonaje);
        }

        return list_find(lista, esFdPersonajeIgual);
}

t_referenciaPersonaje * getPersonajePorSimbolo(t_list * lista, char simboloPersonaje) {
        _Bool esSimboloPersonajeIgual(void * element){
                t_referenciaPersonaje * personajeActual = (t_referenciaPersonaje *) element;
                return (personajeActual->simbolo == simboloPersonaje);
        }

        return list_find(lista, esSimboloPersonajeIgual);
}

t_list * buscarUnPersonajeEnTodasLasListas(t_referenciaNivel * referenciaNivel, int fdPersonaje, int * index){
        _Bool esFdPersonajeIgual(void * element){
                t_referenciaPersonaje* personajeActual = (t_referenciaPersonaje *) element;
                return (personajeActual->fd == fdPersonaje);
        }

        pthread_mutex_lock(&referenciaNivel->semaforoListos);
                t_list * lista = referenciaNivel->personajesListos;

                log_debug(referenciaNivel->logger, "Voy a buscar el personaje a eliminar en la lista de listos");

                t_referenciaPersonaje * personaje = getPersonajePorFd(lista, fdPersonaje);
                if (personaje != NULL) { //Si lo encontro en listos
                        (*index) = list_get_index(lista, esFdPersonajeIgual);
                        pthread_mutex_unlock(&referenciaNivel->semaforoListos);
                        log_debug(referenciaNivel->logger, "Estaba en la lista de listos!");
                        return lista;
                }
        pthread_mutex_unlock(&referenciaNivel->semaforoListos);

        log_debug(referenciaNivel->logger, "No estaba en la lista de listos. Busco en bloqueados!");
        t_link_element * nodoReferenciaRecurso = referenciaNivel->personajesBloqueados->head;
        while (nodoReferenciaRecurso != NULL) {
                pthread_mutex_lock(&((t_referenciaRecurso *) nodoReferenciaRecurso->data)->semaforoBloqueados);
                        lista = ((t_referenciaRecurso *) nodoReferenciaRecurso->data)->personajesBloqueados;
                        log_debug(referenciaNivel->logger, "Buscar el personaje a eliminar en lista bloquedo por: %c", ((t_referenciaRecurso *) nodoReferenciaRecurso->data)->simbolo);
                        personaje = getPersonajePorFd(lista, fdPersonaje);
                        log_debug(referenciaNivel->logger, "eliminar a %c", ((t_referenciaRecurso *) nodoReferenciaRecurso->data)->simbolo);
                        if(personaje != NULL) { //Si lo encontro en la lista

                                (*index) = list_get_index(lista, esFdPersonajeIgual);
                                pthread_mutex_unlock(&((t_referenciaRecurso *) nodoReferenciaRecurso->data)->semaforoBloqueados);
                                log_debug(referenciaNivel->logger, "Lo encontre!");
                                return lista;
                        }
                pthread_mutex_unlock(&((t_referenciaRecurso *) nodoReferenciaRecurso->data)->semaforoBloqueados);
                nodoReferenciaRecurso = nodoReferenciaRecurso->next;
        }

        log_debug(referenciaNivel->logger, "No lo encontre.. Algo raro pasa aquii!");

        (*index) = -1;
        return NULL;
}

void siguientePersonajeActual(t_referenciaNivel * nivel){

        pthread_mutex_lock(&(nivel->semaforoListos));
                if(nivel->personajeActual != NULL) {    //Si personajeActual apunta a alguien
                        if(nivel->personajeActual->next != NULL){       //Si NO estoy parado en el final de la lista
                                nivel->personajeActual = nivel->personajeActual->next;  //Le paso el siguiente
                        }
                        else {
                                nivel->personajeActual = nivel->personajesListos->head; //Le paso el primero de la lista de listos
                        }
                }
                else{
                        nivel->personajeActual = nivel->personajesListos->head; //Como no apunta a nadie, que apunte al primero de la lista (si es que hay alguien, sino NULL)
                }
        pthread_mutex_unlock(&(nivel->semaforoListos));
}


void lista2_add(t_lista* lista, void* personaje){
	pthread_mutex_lock(&lista->mutex);
	list_add(lista->lista,personaje);
	pthread_mutex_unlock(&lista->mutex);
}

t_lista* lista2_create(){
	t_lista* lista=malloc(sizeof(t_lista));
	lista->lista = list_create();
	pthread_mutex_init(&lista->mutex, NULL);

	return lista;
}
