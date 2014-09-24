/*
 * planificador.h
 *
 * Created on: 24/05/2013
 * Author: utnso
 */

#include "planificador.h"

#define distanciaEstimada 10

//TODO rodricadaval - Agrego quatumGlobal y algoritmo a la estructura del planificador.



void* mainPlanificador(void * referenciaNivel) {

	char * recursoAux;
	terminoJuego = 0;
	t_referenciaNivel * estructuraNivel = (t_referenciaNivel*) referenciaNivel;
	estructuraNivel->personajeAPlanificar = malloc(sizeof(t_referenciaPersonaje));
	int personajeCerroConexion;
	t_tipoEstructura tipo;
	void * estructura;
	estructuraNivel->quantum=1;
	char* dirLog = string_from_format("planificador-%s.log", estructuraNivel->nombre); //Ruta donde se gurada!
	estructuraNivel->logger = log_create(dirLog, dirLog, false, LOG_LEVEL_TRACE); //Inicializo. Va a mostrar tambien en pantalla lo que guarde en los archivos
	free(dirLog);
	estructuraNivel->personajesInterbloqueados = list_create();

	estructuraNivel->personajeAPlanificar = NULL;

	while(estructuraNivel->personajesListos->elements_count ==0){

	}

	while(1) {
		printf("HOla! 7 \n");

		pthread_mutex_lock(&estructuraNivel->semaforoListos);
		printf("HOla! 7 \n");
		if (estructuraNivel->personajeAPlanificar == NULL && !list_is_empty(estructuraNivel->personajesListos)) {
			if(string_equals_ignore_case ((char*)estructuraNivel->algoritmo,(char*)"SRDF")){
				estructuraNivel = acomodarListaDeListos(estructuraNivel);
			}
			estructuraNivel->personajeAPlanificar = (t_referenciaPersonaje *) list_remove(estructuraNivel->personajesListos, 0);
			log_info(estructuraNivel->logger,"Empiezo a planificar al personaje: %c",estructuraNivel->personajeAPlanificar->simbolo);

		}
		printf("HOla! 8 \n");
		pthread_mutex_unlock(&estructuraNivel->semaforoListos);
		printf("HOla! 9 \n");

		if(estructuraNivel->personajeAPlanificar != NULL ){

			printf("Estoy planificando al personaje %c\n",estructuraNivel->personajeAPlanificar->simbolo);
			usleep(estructuraNivel->retardo);

			enviarSignalEsTuTurnoPersonaje(estructuraNivel->personajeAPlanificar,estructuraNivel);
			personajeCerroConexion = gestionarConexiones(estructuraNivel);



			if(!personajeCerroConexion){

				int recibi = socket_recibir(estructuraNivel->personajeAPlanificar->fd, &tipo, &estructura);
				if(!recibi){
					log_error(estructuraNivel->logger, "Error al recibir mensaje de nivel o de personaje");
				}

				t_struct_posxposy * posxposy = malloc(sizeof(t_struct_posxposy));
				t_struct_posxposy_simbolo * posxposySimbolo = malloc(sizeof(t_struct_posxposy_simbolo));
				t_signal signal;
				t_struct_simboloPersonaje* SimboloAMandar = malloc(sizeof (t_struct_simboloPersonaje));
				t_struct_pedir * personajeYSimbolo = malloc(sizeof(t_struct_pedir));
				t_struct_asignar * pys = malloc(sizeof(t_struct_asignar));

				switch(tipo)        {

				case D_STRUCT_SIGNAL:
					signal = ((t_struct_signal*) estructura)->signal;
					if(signal == S_Personaje_Orquestador_TermineJuego){
						printf("ME MATARRONNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN\n");
						//if(estructuraNivel->personajesListos->elements_count > 0){

						printf("Estoy matando al personaje %c \n", estructuraNivel->personajeAPlanificar->simbolo);
						socket_cerrarConexion(estructuraNivel->personajeAPlanificar->fd);
						estructuraNivel->quantum = 1;
						SimboloAMandar->simbolo = estructuraNivel->personajeAPlanificar->simbolo;

						printf("Avisando al nivel que el personaje %c termino el de jugar el nivel\n",estructuraNivel->personajeAPlanificar->simbolo);
						log_info(estructuraNivel->logger,"Avisando al nivel que el personaje %c termino el de jugar el nivel",estructuraNivel->personajeAPlanificar->simbolo);
						int seEnvioRecursoANivel = socket_enviar(estructuraNivel->fd, D_STRUCT_SIMBOLOPERSONAJE, SimboloAMandar);
						if(!seEnvioRecursoANivel){
							log_error(estructuraNivel->logger, "Fallo el pedido de posicion de recurso enviada al nivel");
							//}

						}

						liberarAlgunPersonajeBloqueado(estructuraNivel->personajeAPlanificar->recursosAsignados, estructuraNivel);
						estructuraNivel->personajeAPlanificar = NULL;
					}

					if(signal == S_Termine_Por_Control_C) {

						printf("Estoy matando al personaje %c \n", estructuraNivel->personajeAPlanificar->simbolo);
						socket_cerrarConexion(estructuraNivel->personajeAPlanificar->fd);
						estructuraNivel->quantum = 1;
						SimboloAMandar->simbolo = estructuraNivel->personajeAPlanificar->simbolo;

						printf("Avisando al nivel que el personaje %c termino el de jugar el nivel\n",estructuraNivel->personajeAPlanificar->simbolo);
						log_info(estructuraNivel->logger,"Avisando al nivel que el personaje %c termino el de jugar el nivel",estructuraNivel->personajeAPlanificar->simbolo);
						int seEnvioRecursoANivel = socket_enviar(estructuraNivel->fd, D_STRUCT_SIMBOLOPERSONAJE, SimboloAMandar);
						if(!seEnvioRecursoANivel){
							log_error(estructuraNivel->logger, "Fallo el pedido de posicion de recurso enviada al nivel");
						}
						estructuraNivel->personajeAPlanificar = NULL;



					}
					break;

				case D_STRUCT_PEDIRPOSICIONRECURSO:
					log_info(estructuraNivel->logger,"Me estan pidiendo recursos");
					personajeYSimbolo->recurso = ((t_struct_pedirRecurso*)estructura)->letra;
					personajeYSimbolo->personaje = estructuraNivel->personajeAPlanificar->simbolo;
					printf("El personaje %c me esta pidiendo la posicion de un recurso\n", estructuraNivel->personajeAPlanificar->simbolo);
					int seEnvioRecursoANivel = socket_enviar(estructuraNivel->fd, D_STRUCT_PEDIRPOS_RECURSO, personajeYSimbolo);

					if(!seEnvioRecursoANivel){
						log_error(estructuraNivel->logger, "Fallo el pedido de posicion de recurso enviada al nivel");
					}
					int recibiPosicionDeRecurso = socket_recibir(estructuraNivel->fd, &tipo, &estructura);
					if(!recibiPosicionDeRecurso){
						log_error(estructuraNivel->logger, "Error al recibir la posicion del recurso pedido");
					}
					log_info(estructuraNivel->logger,"Recibi la respuesta de nivel!!!!");

					memcpy(&estructuraNivel->personajeAPlanificar->recursoActual,&personajeYSimbolo->recurso,sizeof(char));


					if(tipo == D_STRUCT_POSXPOSY){
						log_info(estructuraNivel->logger,"Recibi la posicion del recurso, me la mando el nivel");
						posxposy = (t_struct_posxposy*)estructura;
						printf("El nivel me mando la posicion del recurso %c \n", personajeYSimbolo->recurso);
						int seEnvioPjPosRecurso = socket_enviar(estructuraNivel->personajeAPlanificar->fd, D_STRUCT_POSXPOSY, posxposy);
						if(!seEnvioPjPosRecurso){
							log_error(estructuraNivel->logger, "Fallo el envio de posicion de recurso al personaje");
						}
						printf("Le envio al personaje %c la posicion del recurso %c \n", personajeYSimbolo->personaje, personajeYSimbolo->recurso);
						memcpy(&estructuraNivel->personajeAPlanificar->posRecX,&posxposy->posX,sizeof(int));
						memcpy(&estructuraNivel->personajeAPlanificar->posRecY,&posxposy->posY,sizeof(int));

					}
					else{

						if(tipo == D_STRUCT_SIMBOLO_DIBUJAME){
							log_info(estructuraNivel->logger,"Recibi matate porque enemigo");

							printf("Cuando estaba pidiendole recurso al nivel, un enemigo mato al personaje %c \n",personajeYSimbolo->personaje);
							// La señal no tiene nada que ver,
							//pero se reutiliza para matar al personaje que fue alcanzado por el enemigo.
							int seEnvioSignalMatateAlPersonaje = socket_enviarSignal(estructuraNivel->personajeAPlanificar->fd, S_Planificador_Personaje_Matate);
							log_info(estructuraNivel->logger,"Mandando signal a personaje para matar a: %c\n",estructuraNivel->personajeAPlanificar->simbolo);
							if(!seEnvioSignalMatateAlPersonaje){
								log_error(estructuraNivel->logger ,"Fallo el envio de la señal MATATE al personaje...\n");
							}
							socket_cerrarConexion(estructuraNivel->personajeAPlanificar->fd);
							estructuraNivel->personajeAPlanificar = NULL;
							estructuraNivel->quantum=1;

							printf("Mate al personaje! \n");
						}else{
							if (tipo == D_STRUCT_SIGNAL){
								t_signal signal = ((t_struct_signal*) estructura)->signal;
								log_info(estructuraNivel->logger, "Recibi una signal %d ERROR!", signal);
							}else{
								printf("Deberia haber recibido posicion recurso o que maten al personaje y recibi: %d\n",tipo);
							}
						}
					}
					free(personajeYSimbolo);
					free(posxposy);

					break;

				case D_STRUCT_POSXPOSY:

					posxposy = (t_struct_posxposy*)estructura;
					posxposySimbolo->posX = posxposy->posX;
					posxposySimbolo->posY = posxposy->posY;
					posxposySimbolo->simbolo = estructuraNivel->personajeAPlanificar->simbolo;


					int seEnvioPjAMover = socket_enviar(estructuraNivel->fd, D_STRUCT_POSXPOSY_SIMBOLO, posxposySimbolo);
					if(!seEnvioPjAMover){
						log_error(estructuraNivel->logger, "Fallo el envio de simbolo de personaje a mover al nivel");
					}
					int recibioBienLoMovio = socket_recibirSignal(estructuraNivel->fd, &signal);
					if(!recibioBienLoMovio){
						printf("No recibi la confirmacion del nivel que movio al personaje\n");
					}

					if(signal == S_Nivel_Personaje_ConfirmaQueSeMovio)
						printf("Planif: El nivel me confirmo que movio al personaje \n");

					avisarPersonajeQueLoMovi(estructuraNivel->personajeAPlanificar, estructuraNivel);
					estructuraNivel->personajeAPlanificar->posPersX = posxposySimbolo->posX;
					estructuraNivel->personajeAPlanificar->posPersY = posxposySimbolo->posY;

					memcpy(&estructuraNivel->personajeAPlanificar->posPersX,&posxposySimbolo->posX,sizeof(int));
					memcpy(&estructuraNivel->personajeAPlanificar->posPersY,&posxposySimbolo->posY,sizeof(int));



					if(signal == S_Enemigo_Mato_Personaje){

						printf("Planif: El nivel me dijo que mato al personaje \n");

						socket_cerrarConexion(estructuraNivel->personajeAPlanificar->fd);
						estructuraNivel->quantum = 1;
						estructuraNivel->personajeAPlanificar = NULL;



					}


					if(string_equals_ignore_case((char *)estructuraNivel->algoritmo,(char *)"RR")){

						(estructuraNivel->quantum)++;
					}



					free(posxposy);
					free(posxposySimbolo);
					break;

					// armar estructura para enviar nivel

					// enviar mensaje a nivel


				case D_STRUCT_PEDIRRECURSO:

					pys->recurso = ((t_struct_pedirRecurso*)estructura)->letra;
					pys->personaje = estructuraNivel->personajeAPlanificar->simbolo;
					printf("El personaje %c  me pide que le asigne el recurso %c \n", pys->personaje, pys->recurso);
					int seEnvioRecursoPedido = socket_enviar(estructuraNivel->fd, D_STRUCT_ASIGNAR_RECURSO , pys); // Le envio el char correspondiente al recurso que pide el personaje al nivel con el mensaje D_STRUCT_CHAR
					if(!seEnvioRecursoPedido) {
						log_error(estructuraNivel->logger,"No se pudo enviar recurso pedido a nivel");
					}


					int recibioSignalHayRecurso = socket_recibirSignal(estructuraNivel->fd, &signal);
					if(!recibioSignalHayRecurso) {
						log_error(estructuraNivel->logger, "No pude recibir signal si habia recurso de nivel");

					}

					if (signal == S_Nivel_Personaje_TeAsigneRecurso ) {

						printf("El nivel me confirmo que hay una instancia recurso para asignarle al personaje \n");
						int seEnvioSignalAPersonaje = socket_enviarSignal(estructuraNivel->personajeAPlanificar->fd, S_Nivel_Personaje_TeAsigneRecurso);
						if(!seEnvioSignalAPersonaje){
							log_error(estructuraNivel->logger,"No se envio el aviso al personaje que le asigne el recurso...");
						}else{
							log_info(estructuraNivel->logger, "Avise a personaje %c que le asigne el recurso", estructuraNivel->personajeAPlanificar->simbolo );
							recursoAux = string_from_format("%c", estructuraNivel->personajeAPlanificar->recursoActual);
							string_append(&estructuraNivel->personajeAPlanificar->recursosAsignados,recursoAux);
							printf ("LA LISTA DE RECURSOS ASIGNADOS ES %s \n\n\n\n",estructuraNivel->personajeAPlanificar->recursosAsignados);
						}
						t_signal signal;
						int recibio = socket_recibirSignal(estructuraNivel->personajeAPlanificar->fd, &signal);
						if(!recibio){
							log_error(estructuraNivel->logger, "No se recibio correctamente la señal del planificador");

						}
						if(signal==Termine_Juego){

							//TODO: ARI, NO DEBERIA HACERSE SOCKETCERRARCONEXION ACA?

							SimboloAMandar->simbolo = estructuraNivel->personajeAPlanificar->simbolo;
							printf("Avisando al nivel que el personaje %c termino el de jugar el nivel\n",estructuraNivel->personajeAPlanificar->simbolo);
							log_info(estructuraNivel->logger,"Avisando al nivel que el personaje %c termino el de jugar el nivel",estructuraNivel->personajeAPlanificar->simbolo);
							int seEnvioRecursoANivel = socket_enviar(estructuraNivel->fd, D_STRUCT_SIMBOLOPERSONAJE, SimboloAMandar);
							if(!seEnvioRecursoANivel){
								log_error(estructuraNivel->logger, "Fallo el pedido de posicion de recurso enviada al nivel");
							}
							liberarAlgunPersonajeBloqueado(estructuraNivel->personajeAPlanificar->recursosAsignados,estructuraNivel);


						}else if(signal==No_Termine_Juego){

							estructuraNivel->personajeAPlanificar->posRecX = 0;
							estructuraNivel->personajeAPlanificar->posRecY = 0;
							estructuraNivel->personajeAPlanificar->recursoActual = '0';

							pthread_mutex_lock(&estructuraNivel->semaforoListos);

							list_add(estructuraNivel->personajesListos,(void *)estructuraNivel->personajeAPlanificar);

							pthread_mutex_unlock(&estructuraNivel->semaforoListos);

						}

					}


					if (signal == S_Nivel_Personaje_NoTeAsigneRecurso ) {

						printf("El nivel me confirmo que no hay una instancia recurso para asignarle al personaje \n");
						int seEnvioSignalAPersonaje = socket_enviarSignal(estructuraNivel->personajeAPlanificar->fd, S_Nivel_Personaje_NoTeAsigneRecurso);
						if(!seEnvioSignalAPersonaje){
							log_error(estructuraNivel->logger,"No se envio el aviso al personaje que no le asigne el recurso...");
						}
						else
							log_info(estructuraNivel->logger, "Avise a personaje %c que no le asigne el recurso", estructuraNivel->personajeAPlanificar->simbolo );
						log_info(estructuraNivel->logger, "Personaje %c quedo bloqueado", estructuraNivel->personajeAPlanificar->simbolo );

						pthread_mutex_lock(&estructuraNivel->semaforoBloqueados);

						list_add(estructuraNivel->personajesBloqueados,(void *)estructuraNivel->personajeAPlanificar);

						pthread_mutex_unlock(&estructuraNivel->semaforoBloqueados);

						printf("ENTRE ACA2 \n\n\n\n\n\n\n\n\n\n\n");

					}


					estructuraNivel->quantum = 1;
					estructuraNivel->personajeAPlanificar = NULL;

					free(personajeYSimbolo);

					break;

					free(personajeYSimbolo);

					break;


				} // cierro switch

			} // cierro if !personajeCerroConexion
		} // cierro if personajeAPlanificar != NULL
		else {
			printf("HOla! 10 \n");
			if (estructuraNivel->personajesListos == 0 && estructuraNivel->personajesBloqueados > 0 && estructuraNivel->personajeAPlanificar == NULL){
				printf("HOla! 11 \n");
				// Alternativa! Se puede hacer un socket recibir o poner gestionarConexionesConTimeOut aca......
			}
			printf("HOla! 12 \n");
			gestionarConexionesConTimeOut(estructuraNivel);

		}
		printf("HOla! 2 \n");


		if(string_equals_ignore_case ((char*)estructuraNivel->algoritmo,(char*)"RR")){
			if(estructuraNivel->quantum > estructuraNivel->quantumGlobal && estructuraNivel->personajeAPlanificar != NULL ){
				cambiarQuantumYCambiarPersonaje(estructuraNivel);
				log_info(estructuraNivel->logger,"Personaje cumplio quantum, cambio a otro personaje");
			}

		}
		printf("HOla! 3 \n");
		matarPersonajesInterbloqueados(estructuraNivel);
		printf("HOla! 6 \n");
	}// cierro while

	return NULL;

}

void matarPersonajesInterbloqueados(t_referenciaNivel * nivel){
	printf("HOla! 4 \n");
	if (nivel->personajesInterbloqueados->elements_count > 0) {
		printf("LLego a la funcion para matar a interbloqueados\n\n\n");


		printf("TAMAÑO DE LA LISTA: %d \nPRIMER PERSONAJE: %c \n",nivel->personajesInterbloqueados->elements_count, ((t_struct_char *)nivel->personajesInterbloqueados->head->data)->letra);

		t_struct_char* personaje = list_remove(nivel->personajesInterbloqueados, 0);

		while (personaje != NULL){
			buscaEnListasYLoBorra(personaje->letra, nivel);
			personaje = list_remove(nivel->personajesInterbloqueados, 0);
		}

	}
	printf("HOla! 5 \n");

}


void buscaEnListasYLoBorra(char simbolo, t_referenciaNivel * estructuraNivel){

	printf("SIUMBOLOOOOOOOO %c\n",simbolo);

	t_referenciaPersonaje * personaje;
	if(estructuraNivel->personajesListos->elements_count > 0)
		personaje  = getPersonajePorSimbolo(estructuraNivel->personajesListos, simbolo);
	else
		personaje=NULL;
	int posicion;

	if (personaje != NULL){
		posicion = posicionPersonajeEnLaLista(estructuraNivel->personajesListos, personaje->simbolo);
		t_referenciaPersonaje * personajeAEliminar = (t_referenciaPersonaje *) list_remove(estructuraNivel->personajesListos, posicion);
		socket_cerrarConexion(personajeAEliminar->fd);
		liberarAlgunPersonajeBloqueado(personajeAEliminar->recursosAsignados, estructuraNivel);
		t_struct_simboloPersonaje* SimboloAMandar = malloc(sizeof (t_struct_simboloPersonaje));
		SimboloAMandar->simbolo = personajeAEliminar->simbolo;
		int seEnvioRecursoANivel = socket_enviar(estructuraNivel->fd, D_STRUCT_SIMBOLOPERSONAJE, SimboloAMandar);
		if(!seEnvioRecursoANivel){
			log_error(estructuraNivel->logger, "Fallo el pedido de posicion de recurso enviada al nivel");
		}
		printf("Lo encontre en listos %c\n",personaje->simbolo);

	}
	else {
		personaje  = getPersonajePorSimbolo(estructuraNivel->personajesBloqueados, simbolo);
		if (personaje != NULL) {
			posicion = posicionPersonajeEnLaLista (estructuraNivel->personajesBloqueados, personaje->simbolo);
			t_referenciaPersonaje * personajeAEliminar = (t_referenciaPersonaje *) list_remove(estructuraNivel->personajesBloqueados, posicion);
			socket_cerrarConexion(personajeAEliminar->fd);
			liberarAlgunPersonajeBloqueado(personajeAEliminar->recursosAsignados, estructuraNivel);
			t_struct_simboloPersonaje* SimboloAMandar = malloc(sizeof (t_struct_simboloPersonaje));
			SimboloAMandar->simbolo = personajeAEliminar->simbolo;
			int seEnvioRecursoANivel = socket_enviar(estructuraNivel->fd, D_STRUCT_SIMBOLOPERSONAJE, SimboloAMandar);
			if(!seEnvioRecursoANivel){
				log_error(estructuraNivel->logger, "Fallo el pedido de posicion de recurso enviada al nivel");
			}
			printf("Lo encontre en Bloqueados %c\n",personaje->simbolo);

		}		else {

			if(estructuraNivel->personajeAPlanificar != NULL) {
				if (simbolo == estructuraNivel->personajeAPlanificar->fd){
					log_info(estructuraNivel->logger,"Va a cerrar la conexion de personaje actual con simbolo %c",estructuraNivel->personajeAPlanificar->simbolo);
					estructuraNivel->quantum = 1;
					liberarAlgunPersonajeBloqueado(estructuraNivel->personajeAPlanificar->recursosAsignados, estructuraNivel);
					t_struct_simboloPersonaje* SimboloAMandar = malloc(sizeof (t_struct_simboloPersonaje));
					SimboloAMandar->simbolo = estructuraNivel->personajeAPlanificar->simbolo;
					int seEnvioRecursoANivel = socket_enviar(estructuraNivel->fd, D_STRUCT_SIMBOLOPERSONAJE, SimboloAMandar);
					if(!seEnvioRecursoANivel){
						log_error(estructuraNivel->logger, "Fallo el pedido de posicion de recurso enviada al nivel");
					}
					estructuraNivel->personajeAPlanificar = NULL;
					printf("Lo encontre en personaje a planificar %c\n",personaje->simbolo);


				}				else				{
					log_info(estructuraNivel->logger,"EL PJ A CERRAR NO ESTA EN LISTOS, NI BLOQUEADOS, NI EN ES EL personajeAPlanificar");
				}
			}
		}
	}
}


void avisarPersonajeQueLoMovi(t_referenciaPersonaje* personaje, t_referenciaNivel * estructuraNivel){
	int fdPersonaje = personaje->fd;

	int seEnvio = socket_enviarSignal(fdPersonaje, S_Nivel_Personaje_ConfirmaQueSeMovio);
	if(!seEnvio){
		log_error(estructuraNivel->logger   ,"No se envio el aviso al personaje que lo movi...\n");
		avisarPersonajeQueLoMovi(personaje,estructuraNivel);        //Como fallo el envio de mensajes, lo mando de nuevo
		return;
	}else{
		log_info(estructuraNivel->logger, "Avise a personaje %c que se lo movi\n", personaje->simbolo );
	}
	printf("Se envio valor: %d\n", seEnvio);
}

void cerramosTodo(t_referenciaNivel * estructuraNivel) {
	log_error(estructuraNivel->logger, "Se cierra el planificador correctamente. (NOT AN ERROR)"); //EN este caso es error para que si o si muestre que termino
	log_destroy(estructuraNivel->logger);
}



void accionPersonajeSeBloqueo(t_referenciaNivel* estructuraNivel, char recurso){

	cambiarPersonajeListosABloqueado(estructuraNivel, estructuraNivel->personajeAPlanificar->simbolo, recurso);
}

void enviarSignalMovetePersonaje(t_referenciaPersonaje* personajeActual,t_referenciaNivel* estructuraNivel){

	int fdPersonajeAMoverse = personajeActual->fd;
	printf("Avisando al personaje que se mueva\n");
	int seEnvio = socket_enviarSignal(fdPersonajeAMoverse, S_Planificador_Personaje_HabilitarMover);
	if(!seEnvio){
		log_error(estructuraNivel->logger,"No se envio correctamente la señal de moverse al personaje con simbolo %c",personajeActual->simbolo);
		return;
	}
}

void accionReiniciarPersonaje(int fdPersonaje, t_referenciaNivel * estructuraNivel){
	int index;
	t_list * listaDondeEstaElPersonaje = buscarUnPersonajeEnTodasLasListas(estructuraNivel, fdPersonaje, &index);
	t_referenciaPersonaje * personaje = list_get(listaDondeEstaElPersonaje, index);

	if(listaDondeEstaElPersonaje == estructuraNivel->personajesListos) {
		personaje = sacarPersonajeDeListaListos(estructuraNivel, personaje->simbolo);
	}
	else {
		personaje = sacarPersonajeDeLista(listaDondeEstaElPersonaje, personaje->simbolo);
	}

	agregarPersonajeAListos(estructuraNivel, personaje);
	personaje->ordenLlegada = estructuraNivel->ordenLlegada;
	estructuraNivel->ordenLlegada = estructuraNivel->ordenLlegada + 1;
	log_info(estructuraNivel->logger, "Reinicio al personaje %c con orden de llegada %d", personaje->simbolo, personaje->ordenLlegada);
}

void enviarSignalEsTuTurnoPersonaje(t_referenciaPersonaje * personajeAPlanificar, t_referenciaNivel* estructuraNivel){
	printf("Envie es tu turno a %c \n",personajeAPlanificar->simbolo);
	if(!socket_enviarSignal(personajeAPlanificar->fd, S_Planificador_Personaje_HabilitarMover)){
		printf("No recibi correctamente el envio del personaje o nivel\n");
		//estructuraNivel->personajeAPlanificar = NULL;
	}

	//log_error(estructuraNivel->logger,"No recibi correctamente el envio del personaje o nivel");
}

void cambiarQuantumYCambiarPersonaje(t_referenciaNivel* estructuraNivel) {
	estructuraNivel->quantum = 1;
	//pthread_mutex_lock(&estructuraNivel->semaforoListos);
	printf("\n");
	printf("Entre en cambiar Quantum y cambiar personaje\n");
	printf("\n");
	printf("CANTIDAD DE ELEMENTOS %d\n",(list_add(estructuraNivel->personajesListos, estructuraNivel->personajeAPlanificar)));
	estructuraNivel->personajeAPlanificar = NULL;
	//pthread_mutex_lock(&estructuraNivel->semaforoListos);
}



t_referenciaPersonaje* getPersonajePorSimbolo2(char simboloPersonaje,t_referenciaNivel * estructuraNivel) {
	_Bool esSimboloPersonajeIgual(void * element){
		t_referenciaPersonaje* personajeActual = (t_referenciaPersonaje*) element;
		return (personajeActual->simbolo == simboloPersonaje);
	}

	return list_find(estructuraNivel->personajesListos, esSimboloPersonajeIgual);

}


int gestionarConexiones(t_referenciaNivel * estructuraNivel){
	struct epoll_event events[MAX_EVENTS_EPOLL];
	t_tipoEstructura tipo;
	void * estructura;
	int nfds = 0, n;
	t_signal signal;
	int meLlegoDePersonajeActual = 0, cerroConexion;
	nfds = epoll_escucharBloqueante(estructuraNivel->fdPlanificadorEpoll, events);
	if (nfds == -1) {
		if (errno == EINTR ){
			return gestionarConexiones(estructuraNivel);
		}
	}
	for (n = 0; n < nfds; ++n) {
		printf("DATA: %d\n",events[n].data.fd);
		if (events[n].data.fd == estructuraNivel->fdPlanificadorServer) { //Abrio conexion
			printf("Llego una conexion Nueva\n");
			//log_info(estructuraNivel->logger, "Llego una conexion Nueva");
			accionNuevaConexionPlanificador(events[n].data, estructuraNivel);
		}
		else {
			if(estructuraNivel->personajeAPlanificar != NULL){
				if(events[n].data.fd == estructuraNivel->personajeAPlanificar->fd && !(events[n].events & EPOLLRDHUP) ){ //Me hablo el personaje actual y no cerro su conexion
					printf("Llego mensaje que esperaba del personaje actual: %c\n", estructuraNivel->personajeAPlanificar->simbolo);
					//log_info(estructuraNivel->logger, "Llego mensaje que esperaba del personaje actual: %c", personajeAPlanificar->simbolo);
					meLlegoDePersonajeActual=1;
					cerroConexion = 0;
				}
				else if (events[n].data.fd == estructuraNivel->personajeAPlanificar->fd){ //Me cerro la conexion el personaje actual

					//liberarAlgunPersonajeBloqueado(estructuraNivel->personajeAPlanificar->recursosAsignados,estructuraNivel);
					printf("Llego cierre personaje actual: %c\n",estructuraNivel->personajeAPlanificar->simbolo);
					//log_info(estructuraNivel->logger, "Llego cierre personaje actual: %c",personajeAPlanificar->simbolo);
					meLlegoDePersonajeActual=1;
					cerroConexion = 1;
					accionCerroConexionPlanificador(events[n].data, estructuraNivel);
				}
				else if(events[n].events & EPOLLRDHUP) { //Cerro conexion
					//					liberarAlgunPersonajeBloqueado(estructuraNivel->personajeAPlanificar->recursosAsignados,estructuraNivel);
					log_info(estructuraNivel->logger,"Llego cierre de un personaje\n");
					//log_info(estructuraNivel->logger, "Llego cierre de un personaje");
					accionCerroConexionPlanificador(events[n].data, estructuraNivel);

				}
				else if (events[n].data.fd == estructuraNivel->fd){ // Me mandó algo el nivel

					printf("Me hablo %s\n",estructuraNivel->nombre);
					log_info(estructuraNivel->logger,"Me hablo %s",estructuraNivel->nombre);


					int recibio = socket_recibir(estructuraNivel->fd, &tipo, &estructura);
					if(!recibio){
						log_error(estructuraNivel->logger, "Error al recibir mensaje de nivel");
					}

					printf(" RECIBI UN SIGNAL TIPO : %d \n", tipo);


					switch(tipo) {

					char simboloAMatar;
					t_struct_algoritmo * nuevosDatos  = malloc(sizeof(t_struct_algoritmo));
					t_struct_char * pInter = malloc(sizeof(t_struct_char));

					case D_STRUCT_ACTUALIZARALGORITMO:

						nuevosDatos = ((t_struct_algoritmo *)estructura);

						estructuraNivel->algoritmo = string_duplicate(nuevosDatos->algoritmo);
						estructuraNivel->quantumGlobal = nuevosDatos->quantum;
						estructuraNivel->retardo = nuevosDatos->retardo;
						printf("-----------------------------------------------------------------------------------------\n");
						printf("Los cambios son. \n Algoritmo: %s \n Quantum: %d \n Retardo: %d \n",estructuraNivel->algoritmo, estructuraNivel->quantum, estructuraNivel->retardo);

						log_info(estructuraNivel->logger, "El algoritmo, retardo y quantum fueron actualizados a los nuevos valores");

						if(string_equals_ignore_case ((char*)estructuraNivel->algoritmo,(char*)"RR")){
							pasarARoundRobin(estructuraNivel);
						}

						break;


					case D_STRUCT_SIMBOLO_DIBUJAME:
						printf("El nivel me pidio que mate a un personaje por haber sido alcanzado por un enemigo! \n");
						// La señal no tiene nada que ver,
						//pero se reutiliza para matar al personaje que fue alcanzado por el enemigo.

						simboloAMatar = ((t_struct_simbolo_dibujame*)estructura)->letra;
						t_referenciaPersonaje* personajeAMatar;

						if (simboloAMatar != estructuraNivel->personajeAPlanificar->simbolo){
							cerroConexion = 0;
							personajeAMatar = getPersonajePorSimbolo2(simboloAMatar,estructuraNivel);
						} else {
							personajeAMatar = estructuraNivel->personajeAPlanificar;
							cerroConexion = 1;

						}
						if(personajeAMatar !=NULL){
							int seEnvioSignalMatateAlPersonaje = 1;//socket_enviarSignal(personajeAMatar->fd, S_Planificador_Personaje_Matate);
							log_info(estructuraNivel->logger,"Mandando signal a personaje para matar a: %c\n",personajeAMatar->simbolo);
							if(!seEnvioSignalMatateAlPersonaje){
								log_error(estructuraNivel->logger ,"Fallo el envio de la señal MATATE al personaje...\n");
							} else {

								if (personajeAMatar->simbolo == estructuraNivel->personajeAPlanificar->simbolo){
									estructuraNivel->personajeAPlanificar = NULL;
								} else {

									int i=0, loEncontre=0;
									int cantidadPersonajes = list_size(estructuraNivel->personajesListos);
									while (i<cantidadPersonajes)
									{

										t_referenciaPersonaje *personajeEncontrado = list_get(estructuraNivel->personajesListos,i);
										if(personajeAMatar->simbolo == personajeEncontrado->simbolo){
											loEncontre = i;

										}
										i++;
									}
									list_remove(estructuraNivel->personajesListos,loEncontre);
								}
							}

							meLlegoDePersonajeActual=1;
							estructuraNivel->quantum=1;
							socket_enviarSignal(personajeAMatar->fd, S_Planificador_Personaje_Matate);
							socket_cerrarConexion(personajeAMatar->fd);
						}
						break;


					case D_STRUCT_CHAR:

						pInter = (t_struct_char *)estructura;
						list_add(estructuraNivel->personajesInterbloqueados, pInter);

						break;
					}

				}
				else {
					socket_recibirSignal(events[n].data.fd, &signal);
					if(signal == S_Personaje_Planificador_Reiniciar) {
						printf("Llego un mensaje de personaje para que lo reinicie.\n");
						//log_info(estructuraNivel->logger, "Llego un mensaje de personaje para que lo reinicie.");
						accionReiniciarPersonaje(events[n].data.fd, estructuraNivel);
					}
					else {
						printf("Llego signal %d y esperaba %d\n", signal, S_Personaje_Planificador_Reiniciar);
						//log_error(estructuraNivel->logger, "Llego signal %d y esperaba %d", signal, S_Personaje_Planificador_Reiniciar);
					}
				}
			}
		}
	}


	if(meLlegoDePersonajeActual) {
		return cerroConexion;

	}

	else {
		return gestionarConexiones(estructuraNivel);
	}
}

int posicionPersonajeEnLaLista (t_list *self,char  simboloPersonaje){

	_Bool personajePorSimbolo(void * element){
		t_referenciaPersonaje* personaje = (t_referenciaPersonaje*) element;
		return (personaje->simbolo == simboloPersonaje);
	}


	int posicion = list_get_index(self, personajePorSimbolo);

	return posicion;
}

int posicionPersonajeEnLaListaPorFd (t_list *self,int fd){

	_Bool personajePorSimbolo(void * element){
		t_referenciaPersonaje* personaje = (t_referenciaPersonaje*) element;
		return (personaje->fd == fd);
	}


	int posicion = list_get_index(self, personajePorSimbolo);

	return posicion;
}


void gestionarNuevasConexiones(t_referenciaNivel * estructuraNivel){
	struct epoll_event events[MAX_EVENTS_EPOLL];
	int nfds, n;
	t_signal signal;
	nfds = epoll_escucharTimeOut(estructuraNivel->fdPlanificadorEpoll, events, 10000); //Para que no se quede aca siempre y vea si hay que cerrar el thread o no...
	if (nfds == -1) {
		if (errno == EINTR ){
			gestionarNuevasConexiones(estructuraNivel);
			return;
		}
	}
	for (n = 0; n < nfds; ++n) {
		if (events[n].data.fd == estructuraNivel->fdPlanificadorServer) { //Abrio conexion
			accionNuevaConexionPlanificador(events[n].data, estructuraNivel);
		}
		else if(events[n].events & EPOLLRDHUP) { //Cerro conexion
			log_info(estructuraNivel->logger, "Llego cierre de un personaje");
			accionCerroConexionPlanificador(events[n].data, estructuraNivel);
		}
		else {
			socket_recibirSignal(events[n].data.fd, &signal);
			if(signal == S_Personaje_Planificador_Reiniciar) {
				log_info(estructuraNivel->logger, "Llego un mensaje de algun personaje. Seguramente es para que lo reinicie.");
				accionReiniciarPersonaje(events[n].data.fd, estructuraNivel);
			}
			else {
				log_error(estructuraNivel->logger, "Llego signal %d y esperaba %d", signal, S_Personaje_Planificador_Reiniciar);
			}
		}
	}
}

void accionNuevaConexionPlanificador(epoll_data_t epollData, t_referenciaNivel* estructuraNivel){
	int fdNuevaConexion;
	printf("Ha llegado un nuevo personaje\n");
	log_trace(estructuraNivel->logger,"Ha llegado un nuevo personaje");

	fdNuevaConexion = socket_aceptarCliente(estructuraNivel->fdPlanificadorServer);

	if (epoll_agregarSocketCliente(estructuraNivel->fdPlanificadorEpoll, fdNuevaConexion)== -1){
		log_error(estructuraNivel->logger,"error al agregar el personaje al epoll");
		return ;
	}

	t_tipoEstructura tipo;
	void * estructura;
	t_struct_simboloPersonaje * simboloPersonaje;

	int recibio = socket_recibir(fdNuevaConexion, &tipo, &estructura);
	if (!recibio) {
		if (estructura != NULL)
			free(estructura);
		socket_cerrarConexion(fdNuevaConexion);
		log_error(estructuraNivel->logger,"No recibi correctamente el envio del personaje o nivel");
		return ;
	}


	if (tipo == D_STRUCT_SIMBOLOPERSONAJE) { //cuando se conecta un personaje.
		simboloPersonaje = (t_struct_simboloPersonaje *) estructura;
		t_referenciaPersonaje * personajeNuevo = crearReferenciaPersonaje(simboloPersonaje->simbolo, fdNuevaConexion, &(estructuraNivel->ordenLlegada));
		agregarPersonajeAListos(estructuraNivel, personajeNuevo);
		printf("Agrego al personaje con simbolo %c con orden llegada %d a Listos\n",personajeNuevo->simbolo, estructuraNivel->ordenLlegada);
		log_info(estructuraNivel->logger,"Agrego al personaje con simbolo %c con orden llegada %d a Listos\n",personajeNuevo->simbolo, estructuraNivel->ordenLlegada);
	}
	else {
		socket_cerrarConexion(fdNuevaConexion);
		log_error(estructuraNivel->logger,"No recibi un simbolo de personaje");
	}

	if (estructura != NULL)
		free(estructura);
	return;
}

void accionCerroConexionPlanificador(epoll_data_t epollData, t_referenciaNivel * estructuraNivel){

	int fd = epollData.fd;
	printf("cerre1 \n");
	t_referenciaPersonaje * personaje;
	personaje  = getPersonajePorFd(estructuraNivel->personajesListos, fd);
	int posicion;
	printf("cerre2 \n");
	if (personaje != NULL){
		printf("cerre3 \n");
		posicion = posicionPersonajeEnLaListaPorFd (estructuraNivel->personajesListos, personaje->fd);
		t_referenciaPersonaje * personajeAEliminar = (t_referenciaPersonaje *) list_remove(estructuraNivel->personajesListos, posicion);
		socket_cerrarConexion(personajeAEliminar->fd);
		t_struct_simboloPersonaje* SimboloAMandar = malloc(sizeof (t_struct_simboloPersonaje));
		SimboloAMandar->simbolo = personajeAEliminar->simbolo;
		int seEnvioRecursoANivel = socket_enviar(estructuraNivel->fd, D_STRUCT_SIMBOLOPERSONAJE, SimboloAMandar);
		if(!seEnvioRecursoANivel){
			log_error(estructuraNivel->logger, "Fallo el pedido de posicion de recurso enviada al nivel");
		}

	}
	else {
		printf("cerre4 \n");
		if(estructuraNivel->personajesBloqueados->elements_count>0){
			printf("cerre14 %d,%d\n",estructuraNivel->personajesBloqueados->elements_count,fd);
		personaje  = getPersonajePorFd(estructuraNivel->personajesBloqueados, fd);
		printf("cerre1114 \n");
		if (personaje != NULL) {
			printf("cerre222224 \n");
			posicion = posicionPersonajeEnLaListaPorFd (estructuraNivel->personajesBloqueados, personaje->fd);
			t_referenciaPersonaje * personajeAEliminar = (t_referenciaPersonaje *) list_remove(estructuraNivel->personajesBloqueados, posicion);
			socket_cerrarConexion(personajeAEliminar->fd);
			printf("cerre333333333334 \n");
			t_struct_simboloPersonaje* SimboloAMandar = malloc(sizeof (t_struct_simboloPersonaje));
			SimboloAMandar->simbolo = personajeAEliminar->simbolo;
			int seEnvioRecursoANivel = socket_enviar(estructuraNivel->fd, D_STRUCT_SIMBOLOPERSONAJE, SimboloAMandar);
			if(!seEnvioRecursoANivel){
				log_error(estructuraNivel->logger, "Fallo el pedido de posicion de recurso enviada al nivel");
			}

		}		else {
			printf("cerre5 \n");
			if(estructuraNivel->personajeAPlanificar != NULL) {
				if (fd == estructuraNivel->personajeAPlanificar->fd){
					log_info(estructuraNivel->logger,"Va a cerrar la conexion de personaje actual con simbolo %c",estructuraNivel->personajeAPlanificar->simbolo);
					estructuraNivel->quantum = 1;
					t_struct_simboloPersonaje* SimboloAMandar = malloc(sizeof (t_struct_simboloPersonaje));
					SimboloAMandar->simbolo = estructuraNivel->personajeAPlanificar->simbolo;
					int seEnvioRecursoANivel = socket_enviar(estructuraNivel->fd, D_STRUCT_SIMBOLOPERSONAJE, SimboloAMandar);
					if(!seEnvioRecursoANivel){
						log_error(estructuraNivel->logger, "Fallo el pedido de posicion de recurso enviada al nivel");
					}
					estructuraNivel->personajeAPlanificar = NULL;

					printf("cerre6\n");

				}				else				{
					log_info(estructuraNivel->logger,"EL PJ A CERRAR NO ESTA EN LISTOS, NI BLOQUEADOS, NI EN ES EL personajeAPlanificar");
				}
			}
			printf("cerre7\n");
		}
		printf("cerre8\n");
	}
		printf("cerre9\n");
	}


	//	estructuraNivel->personajeAPlanificar = NULL;

}






t_referenciaNivel * acomodarListaDeListos(t_referenciaNivel * nivel){

	printf ("cantidad de elementos en pjs listos es %d \n\n\n",nivel->personajesListos->elements_count);

	_Bool criterioOrdenamiento(t_referenciaPersonaje * personaje1, t_referenciaPersonaje * personaje2 ){

		int distanciaPersonaje1;
		int distanciaPersonaje2;
		printf("LA POSICION DEL PERSONAJE %c ES (%d,%d)\n", personaje1->simbolo, personaje1->posPersX, personaje1->posPersY);
		printf("LA POSICION DEL RECURSO %c ES (%d,%d)\n", personaje1->recursoActual, personaje1->posRecX, personaje1->posRecY);


		printf("LA POSICION DEL PERSONAJE %c ES (%d,%d)\n", personaje2->simbolo, personaje2->posPersX, personaje2->posPersY);
		printf("LA POSICION DEL RECURSO %c ES (%d,%d)\n", personaje2->recursoActual, personaje2->posRecX, personaje2->posRecY);


		if(personaje1->posRecX==0 && personaje1->posRecY==0){
			distanciaPersonaje1=distanciaEstimada;
		}else{
			int distanciaEnX=abs(personaje1->posPersX - personaje1->posRecX);
			int distanciaEnY=abs(personaje1->posPersY - personaje1->posRecY);
			distanciaPersonaje1=distanciaEnX+distanciaEnY;
		}
		printf ( "ENTRE ACA2 distanciaPersonaje1 %d\n\n\n\n", distanciaPersonaje1);

		printf ( "pj2 simbolo %c \n\n\n\n",personaje2->simbolo);
		printf ( "pj2 posPersX %d posRecX %d \n\n\n\n",personaje2->posPersX, personaje2->posRecX);
		printf ( "pj2 posPersY %d posRecY %d \n\n\n\n",personaje2->posPersY, personaje2->posRecY);
		if(personaje2->posRecX==0 && personaje2->posRecY==0){
			distanciaPersonaje2=distanciaEstimada;
		}else{
			int distanciaEnX=abs(personaje2->posPersX - personaje2->posRecX);
			int distanciaEnY=abs(personaje2->posPersY - personaje2->posRecY);
			distanciaPersonaje2=distanciaEnX+distanciaEnY;
		}
		printf ( "ENTRE ACA3 distanciaPersonaje2 %d\n\n\n\n", distanciaPersonaje2);
		return(distanciaPersonaje1>=distanciaPersonaje2);

	}



	list_sort(nivel->personajesListos,(void*)criterioOrdenamiento);

	return nivel;


}

void liberarAlgunPersonajeBloqueado(char * recursosLiberados,t_referenciaNivel* nivel){

    printf("SE EJECUTA LA FUNCION liberarAlgunPersonajeBloqueado \n\n\n\n\n\n\n\n\n\n");

    printf("CANTIDAD DE PJ'S BLOQUEADOS %d \n\n\n\n\n\n\n\n",nivel->personajesBloqueados->elements_count);

    //    printf ("LA LISTA DE RECURSOS ASIGNADOS ES %s \n\n\n\n",nivel->personajeAPlanificar->recursosAsignados);

    if(nivel->personajesBloqueados->elements_count>0){
        printf("ME METO ACA Y HAGO CAGADAS \n\n\n\n\n\n");


        int c = 0;
        char * recursoAux;
        int posicion = 0;
        int b;
        t_referenciaPersonaje * personaje;
        int cant = list_size(nivel->personajesBloqueados);

        int removedor=0;
        for (c = 0;c < cant;c++){
        	personaje = list_get(nivel->personajesBloqueados, removedor);
            b = strlen (recursosLiberados);

            int i = 0;

            while(i < b){
            	printf(" %c \n",recursosLiberados[i]);
            	if(personaje==NULL){
            		printf("Es nnull!!!!\n");
            	}
            	printf("Recurso actual %c == ",personaje->recursoActual);


                if ( personaje->recursoActual == recursosLiberados[i]){


                    //enviarSignalEsTuTurnoPersonaje((t_referenciaPersonaje *)(nivel->personajesBloqueados->head->data),nivel);
                    posicion = posicionPersonajeEnLaLista (nivel->personajesBloqueados,personaje->simbolo);

                    printf("La Posicion es %d\n\n\n\n\n\n",posicion);

                    recursoAux = string_from_format("%c", recursosLiberados[i] );
                    string_append(&personaje->recursosAsignados,recursoAux);

                    printf("--------Estoy por agregar a la lista de listos a :%c ----------------------------------------\n",personaje->simbolo);
                    pthread_mutex_lock(&nivel->semaforoListos);
                    list_add(nivel->personajesListos,personaje);
                    pthread_mutex_unlock(&nivel->semaforoListos);

                    t_struct_asignar * pys = malloc(sizeof(t_struct_asignar));
                    pys->recurso = recursosLiberados[i];
                    pys->personaje = personaje->simbolo;
                    printf("El personaje %c me pide que le asigne el recurso %c porque fue liberado!!!! \n\n\n", pys->personaje, pys->recurso);
                    int seEnvioRecursoPedido = socket_enviar(nivel->fd, D_STRUCT_ASIGNAR_RECURSO , pys); // Le envio el char correspondiente al recurso que pide el personaje al nivel con el mensaje D_STRUCT_CHAR
                    if(!seEnvioRecursoPedido) {
                        printf("No se pudo enviar recurso pedido a nivel --------------------------------------- \n");
                    }
                    free(pys);
                    t_signal signal;

                    int recibioSignalHayRecurso = socket_recibirSignal(nivel->fd, &signal);
                    if(!recibioSignalHayRecurso) {
                        log_error(nivel->logger, "No pude recibir signal si habia recurso de nivel");

                    }
                    printf("RECIBI %d  \n\n",signal);
                    printf("SE ROMPE ACA22222 \n\n\n\n\n\n");
                    pthread_mutex_lock(&nivel->semaforoBloqueados);
                    if(posicion >=0){
                    list_remove(nivel->personajesBloqueados,posicion);


                    printf("SE ROMPE ACA333333333 \n\n\n\n\n\n");

                    // nivel->personajeAPlanificar = NULL;
                    char* cadena;
                    printf("CADENAAAAAAAA121213121 %s\n",recursosLiberados);
                    cadena = quitarRecursoDeRecusosLiberados ( recursosLiberados, recursosLiberados[i] );
                    printf("CADENAAAAAAAA %s\n",cadena);
                    printf("STRLEEEENMNNN %d\n",strlen(cadena));
                    recursosLiberados = string_duplicate(cadena);
                    printf("CADENAAAAAAAA22222 %s\n",recursosLiberados);
                    printf("sfsdfsdfsdfsfsdfsdfsd! 2 \n");
                    i = 10000;
                    }
                    pthread_mutex_unlock(&nivel->semaforoBloqueados);
                }
                i++;
                printf("SE ROMPE 4444444444444 \n\n\n\n\n\n");
            }
            if(i<10000){
            	removedor++;
            }
            printf("SE ROMPE 55555555555 \n\n\n\n\n\n");
        }
        printf("SE ROMPE 66666666666 \n\n\n\n\n\n");
    }
    printf("SE ROMPE 777777777777 \n\n\n\n\n\n");

}


char* quitarRecursoDeRecusosLiberados (char* stringRecursosLiberados ,char RecursoAEliminar ){

    int cantidadDeCaracteres = strlen (stringRecursosLiberados);
    int c;
    int acaEsta;
    char* devuelvo;
    for (c = 0;c < cantidadDeCaracteres;c++){
        if (stringRecursosLiberados[c]==RecursoAEliminar){
            acaEsta = c;
            c = cantidadDeCaracteres;
        }
    }
    printf("POSICION DE QUITAR RECURSOS : %d\n",acaEsta);

    devuelvo = eliminarUnaPosicionDeUnString (stringRecursosLiberados,acaEsta);
    return devuelvo;

}


char* eliminarUnaPosicionDeUnString (char* stringRecursosLiberados, int posicionAQuitar ){
	printf("arrancando! 1 \n");
    int cantidadDeCaracteres = strlen(stringRecursosLiberados);
    int desdeAca = posicionAQuitar;
    char * loQueQuieras = string_duplicate("");
    int c;
	printf("strlan cantidadDeCaracteres! %d \n",cantidadDeCaracteres);

	char * recursoAux;
    for (c = 0 ;c < desdeAca;c++){
    	printf("arrancando! en el for \n");
    		recursoAux = string_from_format("%c", stringRecursosLiberados[c] );
    		string_append(&loQueQuieras,recursoAux);


    }for (c = (desdeAca+1) ;c < cantidadDeCaracteres;c++){
    	printf("arrancando! en el for \n");
    	    		recursoAux = string_from_format("%c", stringRecursosLiberados[c] );
    	    		string_append(&loQueQuieras,recursoAux);

    }


	printf("arrancando! 3 \n");

    return loQueQuieras;
}



void pasarARoundRobin(t_referenciaNivel * nivel) {

	_Bool ordenLlegadaMenor(t_referenciaPersonaje * element, t_referenciaPersonaje* elementSiguiente){
		return element->ordenLlegada < elementSiguiente->ordenLlegada;
	}

	list_sort(nivel->personajesListos, (void*) ordenLlegadaMenor);

	//nivel->personajeAPlanificar = NULL;


}


void escuchaBloqueante (t_referenciaNivel * nivel){

	void *estructura;

	t_tipoEstructura tipo;
	socket_recibir(nivel->fd,&tipo,&estructura);
	printf("RECIBI ALGO DE NIVELLLL IUJUUU \n");

	if (tipo==D_STRUCT_CHAR){
		t_struct_char * pInter = malloc(sizeof(t_struct_char));
		pInter = (t_struct_char *)estructura;
		printf("Me llego un personaje interbloqueado con simbolo %c", pInter->letra);
		list_add(nivel->personajesInterbloqueados, pInter);

	} else {

		printf("Recibi un tipo de datos: %d", tipo);
	}

	printf("LLEGO HASTA ACA? \n");


}

void gestionarConexionesConTimeOut(t_referenciaNivel  * nivel){

	struct epoll_event events[MAX_EVENTS_EPOLL];
	int nfds, n;
	nfds = epoll_escucharTimeOut(nivel->fdPlanificadorEpoll, events, 10000);

	if (nfds == -1) {

		if (errno == EINTR ){

			gestionarConexionesConTimeOut(nivel);

			return;
		}

	}

	for (n = 0; n < nfds; ++n) {

		if(events[n].events & EPOLLRDHUP) { //Cerro conexion

			log_info(nivel->logger, "Llego cierre de un personaje");

			accionCerroConexionPlanificador(events[n].data,nivel);

		}
		else if (events[n].data.fd == nivel->fd &&  !(events[n].events & EPOLLRDHUP )) { // me llega msj del nivel que se murio personaje
			escuchaBloqueante(nivel);

		}
	}
}

