/*
 * orquestador.c
 *
 *  Created on: 05/10/2013
 *      Author: utnso
 */
#include "orquestador.h"
#include "planificador.h"
//#include "Personaje.h"

int main(int argc, char* argv[]){
        /*leo por parametro del programa el archivo config*/
        char * path = argv[1];
        if (argc != 4) {
                printf("plataforma -> Sintaxis correcta: plataforma rutaConfig rutaExeKoopa rutaConfigKoopa\n");
                exit(1);
        }

        struct stat buffer;
        if (stat(argv[1], &buffer) == -1) {
                printf("Archivo incorrecto.\n");
                exit(1);
        }

        koopa.pathKoopa = argv[2];
        koopa.archivoLeeKoopa = argv[3];

        /*----------------------     Empieza el main ---------------------------------------*/

        loggerOrquestador = log_create("orquestador.log", "ORQUESTADOR", true, LOG_LEVEL_TRACE);

        orquestador = crearOrquestador(path);

        log_info(loggerOrquestador, "Orquestador creado correctamente.");

        printf("Orquestador creado correctamente\n");

        crearServidor(socket_ip(orquestador->direccionIp), socket_puerto(orquestador->direccionIp));

        printf("Escuchando las conexiones entrantes...\n");

        // Escuchar nuevas conexiones
        while(!terminoJuego){
                epoll_escucharGeneral(orquestador->fdOrquestadorEpoll, orquestador->fdOrquestadorServer, accionNuevaConexion, accionMeMandaronAlgo, accionSeDesconecto);
        }
        return 1;
}
/*---------------------- Termina el main -------------------------------------------*/

t_orquestador * crearOrquestador(char *ruta){
        t_config *config = config_create(ruta);

        t_orquestador * newOrquestador = malloc(sizeof(t_orquestador));

        newOrquestador->listaDeNiveles = list_create();
        newOrquestador->direccionIp = string_duplicate(config_get_string_value(config, "direccionIp"));
        newOrquestador->cantPersonajesJugando = 0; //Contador que incrementa cuando entra a jugar un jugador nuevo, decrementa cuando el jugador termina el juego. Sirve para saber cuándo todos los personajes terminaron de jugar.
        newOrquestador->fdOrquestadorServer = 0;
        newOrquestador->rutaConfig = string_duplicate(ruta);

        config_destroy(config);
        return newOrquestador;
}

void crearServidor(char * ip, int puerto) {

        int socket = socket_crearServidor(ip, puerto);
        if (socket == -1) {
                //log_error(loggerOrquestador, "Error al crear socket");
                exit(1);
        }

        orquestador->fdOrquestadorServer = socket;

        int fdEpoll = epoll_crear();
        if (fdEpoll == -1) {
                log_error(loggerOrquestador, "Error al crear epoll");
                printf("Error al crear epoll");
                socket_cerrarConexion(orquestador->fdOrquestadorServer);
                exit(1);
        }

        orquestador->fdOrquestadorEpoll = fdEpoll;

        if ( epoll_agregarSocketServidor(orquestador->fdOrquestadorEpoll, orquestador->fdOrquestadorServer) == -1) {
                printf("Error al agregar socket servidor a epoll");
                log_error(loggerOrquestador, "Error al agregar socket servidor a epoll");
        }

}

/*--------------------------------- separador ----------------------------------*/

void accionNuevaConexion(epoll_data_t epollData){
        log_info(loggerOrquestador,"Se genero una nueva conexion");
        int fdNuevaConexion;
        fdNuevaConexion = socket_aceptarCliente(orquestador->fdOrquestadorServer);
        log_trace(loggerOrquestador, "Agrego la nueva conexion al epoll");

        if (epoll_agregarSocketCliente(orquestador->fdOrquestadorEpoll, fdNuevaConexion)== -1){
                log_error(loggerOrquestador, "Error al agregar personaje o nivel al epoll");
                printf("Error al agregar personaje o nivel al epoll");
                return ;
        }

        t_tipoEstructura tipo;
        void * estructura;

        int recibio = socket_recibir(fdNuevaConexion, &tipo, &estructura);
        if (!recibio) {
                if (estructura != NULL)
                        free(estructura);
                log_error(loggerOrquestador, "No recibi correctamente el envio del personaje o nivel");
                printf ("No recibi correctamente el envio del personaje o nivel");
                return ;
        }

        t_struct_datosNivel * estructuraDatosNivel;
        t_referenciaNivel * referenciaNivel;
        t_signal signal;
        t_struct_nombreNivel* respuesta = malloc(sizeof(t_struct_nombreNivel));
        t_struct_datosNivel* nivel= malloc(sizeof(t_struct_datosNivel));
        t_struct_nombreNivelYSimbolo* esElNivel = malloc(sizeof(t_struct_nombreNivelYSimbolo));

        t_referenciaNivel * nivelAConectar;
        t_referenciaPersonaje* Personaje = malloc(sizeof(t_referenciaPersonaje));

        int nivelExiste = 0;
        int recursoExiste = 0;

        switch(tipo){
        case D_STRUCT_DATOSNIVEL: // CUANDO SE CONECTA UN NIVEL
        log_info(loggerOrquestador,"Me llego un nivel");
        estructuraDatosNivel = (t_struct_datosNivel *) estructura;
        printf ("Se conectó el %s, sus recursos son: %s  \n", estructuraDatosNivel->nombre, estructuraDatosNivel->recursos);
        referenciaNivel = crearReferenciaNivel(estructuraDatosNivel->nombre, orquestador->direccionIp, fdNuevaConexion, estructuraDatosNivel->recursos,estructuraDatosNivel->algoritmo,estructuraDatosNivel->quantum,estructuraDatosNivel->retardo);
        referenciaNivel->cerrarPlanificador = &cerrarPlanificadores; // PARA QUE ES ESTO?

        //CHEQUEO SI EL NIVEL QUE SE ESTA CONECTANDO YA EXISTE.
        nivelExiste = esteNivelExiste (orquestador->listaDeNiveles,referenciaNivel->nombre);

        if (nivelExiste){
                log_info(loggerOrquestador, "El nivel creado ya existia, lo voy a matar");
                t_struct_signal* signalFinalizateNivel = malloc (sizeof(t_struct_signal));
                signalFinalizateNivel->signal = S_Orquestador_Nivel_Finalizate;
                int seEnvioSignalCerrarTodo = socket_enviar(fdNuevaConexion, D_STRUCT_SIGNAL, signalFinalizateNivel);
                while(!seEnvioSignalCerrarTodo) {
                        printf("No se pudo enviar la respuesta del nivel %s al personaje\n", respuesta->nombre);
                        log_error(loggerOrquestador, "No se pudo enviar las direcciones del nivel %s al personaje", referenciaNivel->nombre);
                }
        }

        if(!nivelExiste) {
                agregarNivelALista(referenciaNivel);
                log_info(loggerOrquestador, "El nivel fue agregado a la listaDeNiveles del orquestador. Cantidad de elementos en la listaDeNiveles: %d\n", orquestador->listaDeNiveles->elements_count);

                // CREO UN HILO PLANIFICADOR PARA EL NIVEL QUE SE CONECTÓ.
                crearHiloPlanificador(referenciaNivel);

                if (epoll_agregarSocketCliente(referenciaNivel->fdPlanificadorEpoll,
                                fdNuevaConexion) == -1) {
                        log_error(loggerOrquestador, "Error al agregar personaje o nivel al epoll");
                        return;
                }
                if (epoll_eliminarSocketCliente(orquestador->fdOrquestadorEpoll, fdNuevaConexion)== -1){
                        log_error(loggerOrquestador, "Error al agregar personaje o nivel al epoll");
                        return ;
                }
        }
        log_info(loggerOrquestador, "Acabo de conectar al nivel %s", referenciaNivel->nombre);
        break;

        case D_STRUCT_SIGNAL: // CUANDO SE CONECTA UN PERSONAJE POR PRIMERA VEZ.
                log_info(loggerOrquestador,"Me llego un signal");
                signal = ((t_struct_signal*) estructura)->signal;
                switch(signal){
                case S_Personaje_Orquestador_EmpeceJuego: // PERSONAJE AVISA QUE COMIENZA EL JUEGO.
                        orquestador->cantPersonajesJugando++;
                        log_info(loggerOrquestador, "Un personaje empezó el juego");
                        log_info(loggerOrquestador, "Cantidad de personajes jugando: %d\n",orquestador->cantPersonajesJugando);

                        break;
                case S_Personaje_Orquestador_TermineJuego: //PERSONAJE AVISA QUE TERMINO EL JUEGO.
                	orquestador->cantPersonajesJugando--;
                	log_info(loggerOrquestador, "Un personaje terminó el juego");
                	if(orquestador->cantPersonajesJugando == 0){
                		log_info(loggerOrquestador,"TERMINARON TODOS LOS PERSONAJES DE JUGAR! CERRAMOS TODO Y LLAMAMOS A KOOPA!");
                		hacerAccionesDeFinalDeJuego();
                	}
                	break;
                case Termine_Juego: //PERSONAJE AVISA QUE TERMINO EL JUEGO.
                	orquestador->cantPersonajesJugando--;
                	log_info(loggerOrquestador, "Un personaje terminó el juego MAL (nada grave)");
                	if(orquestador->cantPersonajesJugando == 0){
                		log_info(loggerOrquestador,"TERMINARON TODOS LOS PERSONAJES DE JUGAR! CERRAMOS TODO Y LLAMAMOS A KOOPA!");
                		hacerAccionesDeFinalDeJuego();
                	}
                	break;
                default:
                        printf("El personaje envió una señal incorrecta. La señal %c", signal);
                        //log_error(loggerOrquestador, "El personaje envió una señal incorrecta. La señal %c", signal);
                        free(estructura);
                        return ;
                }
                break;

                case D_STRUCT_DATONIVELYRECURSO:// PERSONAJE SE CONECTA CON EL PLAN DE NIVELES Y RECURSOS. (Orquestador verifica exitencia).
                        log_info(loggerOrquestador,"Me llego un personaje");
                        // En caso de tratarse de una conexión de un Personaje, validará que el Nivel que está solicitando exista y
                        // delegará la conexión al planificador correspondiente.
                        nivel->nombre = ((t_struct_datosNivel* )estructura)->nombre;
                        nivel->recursos = ((t_struct_datosNivel* )estructura)->recursos;

                        nivelExiste = esteNivelExiste (orquestador->listaDeNiveles,(char*)nivel->nombre);

                        if (!nivelExiste){
                                log_error (loggerOrquestador, "ERROR: El personaje quiere jugar a un nivel que no fue creado!!\n");
                                respuesta->nombre = "KO";
                        }
                        if (nivelExiste)
                        {
                                log_info (loggerOrquestador, "El %s existe!, el objetivo del personaje es conseguir estos recursos: %s, CHEQUEANDO SI EXISTEN\n",nivel->nombre,nivel->recursos);
                                recursoExiste = existenRecursosEnNivel(orquestador->listaDeNiveles,nivel->nombre,nivel->recursos);
                                log_info(loggerOrquestador, "Existe recurso: %d\n",recursoExiste);
                                if (recursoExiste)
                                {
                                        respuesta->nombre = "OK";
                                }
                                else
                                {
                                        respuesta->nombre = "KO";
                                }

                        }

                        int seEnvio = socket_enviar(fdNuevaConexion, D_STRUCT_NOMBRENIVEL , respuesta);
                        while(!seEnvio) {
                        //      log_error(loggerOrquestador, "No se pudo enviar las direcciones del nivel %s al personaje", referenciaNivel->nombre);
                        }
                        break;

                case D_STRUCT_CONECTAME: //Se conectar un personaje y derivo a planificador
                        log_info(loggerOrquestador,"Me llego un personaje y quiere conectarse a un nivel");
                        esElNivel->nombre = string_duplicate(((t_struct_nombreNivelYSimbolo*)estructura)->nombre);
                        esElNivel->simboloPersonaje = ((t_struct_nombreNivelYSimbolo*)estructura)->simboloPersonaje;
                        log_info(loggerOrquestador, "El Personaje de simbolo '%c' quiere conectarse al nivel '%s'\n",esElNivel->simboloPersonaje,esElNivel->nombre);

                        int nombreNivelEncaja(t_referenciaNivel *niv) {

                                return string_equals_ignore_case(niv->nombre,esElNivel->nombre);

                        }

                        nivelAConectar = list_find(orquestador->listaDeNiveles,(void*) nombreNivelEncaja);

                        if (epoll_agregarSocketCliente(nivelAConectar->fdPlanificadorEpoll, fdNuevaConexion)== -1){
                                //      log_error(loggerOrquestador, "Error al agregar personaje o nivel al epoll"); TODO
                                log_error(loggerOrquestador, "Error al agregar personaje o nivel al epoll\n");
                                return ;
                        }
                        printf ("Agregué el fd del personaje %d\n",fdNuevaConexion);
                        if (epoll_eliminarSocketCliente(orquestador->fdOrquestadorEpoll, fdNuevaConexion)== -1){
                                //      log_error(loggerOrquestador, "Error al agregar personaje o nivel al epoll"); TODO
                                log_error(loggerOrquestador, "Error al eliminar un personaje del epoll del orquestador\n");
                                return ;
                        }

                        memcpy(&Personaje->simbolo,&esElNivel->simboloPersonaje,sizeof(char));
                        printf("Personaje: %c y Nivel: %s\n",Personaje->simbolo,esElNivel->nombre);

                        Personaje->fd = fdNuevaConexion;
                        Personaje->ordenLlegada = orquestador->cantPersonajesJugando;
                        Personaje->posPersX = 1;
                        Personaje->posPersY = 1;
                        Personaje->posRecX = 0;
                        Personaje->posRecY = 0;
                        Personaje->recursosAsignados = string_new();
                        Personaje->recursoActual = '0';
                        pthread_mutex_lock(&nivelAConectar->semaforoListos);
                        list_add(nivelAConectar->personajesListos,Personaje);
                        t_struct_simbolo_dibujame* personajeADibujar = malloc(sizeof(t_struct_simbolo_dibujame));
                        personajeADibujar->letra = Personaje->simbolo;
                        int seEnviaDibujamANivel = socket_enviar(nivelAConectar->fd, D_STRUCT_SIMBOLO_DIBUJAME, personajeADibujar);
                        if(!seEnviaDibujamANivel){
                                log_error(nivelAConectar->logger, "Fallo el envio de dibujar por primera vez el simbolo de personaje al nivel");
                        }
                        pthread_mutex_unlock(&nivelAConectar->semaforoListos);

                        break;

                default:
                        log_info(loggerOrquestador,"Me llego cualquier cosa");
                        if (estructura != NULL){
                                free(estructura);
                        }
                        log_error(loggerOrquestador, "No recibi una estructura de esperada... recibi el tipo: %i", tipo);
                        return ;
                        break;
        }
        free(nivel);
        free(respuesta);
        free(estructura);

        return ;
}


/*--------------------------------- separador ----------------------------------*/
void accionSeDesconecto(epoll_data_t epollData){
        t_referenciaNivel * referenciaNivel = getNivelPorFd(epollData.fd);

        if(referenciaNivel != NULL) { //si es null, significa que el que se desconecto fue un personaje => el orq no tiene que hacer nada.
                printf("Cerró conexion el nivel %s\n",referenciaNivel->nombre);
                eliminarNivelDeLista(referenciaNivel);

        }
        else{
                printf("ORQ: Se desconectó un personaje\n");
        }
}

/*--------------------------------- separador ----------------------------------*/


void accionMeMandaronAlgo(epoll_data_t epollData){
        log_info(loggerOrquestador,"Me mandaron algo");
        int fdEmisor = epollData.fd;


        t_tipoEstructura tipo;
        void * estructura;
        int recursoExiste;
        int recibio = socket_recibir(fdEmisor, &tipo, &estructura);
        if (!recibio) {
                if (estructura != NULL)
                        free(estructura);
                log_error(loggerOrquestador, "No recibi correctamente el envio del personaje o nivel");
                printf ("No recibi correctamente el envio del personaje o nivel");
                return ;
        }

        t_struct_nombreNivel* respuesta = malloc(sizeof(t_struct_nombreNivel));
        t_struct_datosNivel* nivel = malloc(sizeof(t_struct_datosNivel));
        int nivelExiste = 0;

        switch(tipo){

        case D_STRUCT_DATONIVELYRECURSO:        //se conecta personaje con una lista de niveles, para que el orq le devuelva la direccione del planificador de el/los nivel/es en cuestion)

                // En caso de tratarse de una conexión de un Personaje, validará que el Nivel que está solicitando exista y
                // delegará la conexión al planificador correspondiente.
                nivel->nombre = ((t_struct_datosNivel* )estructura)->nombre;
                nivel->recursos = ((t_struct_datosNivel* )estructura)->recursos;
                printf("Nivel Recibido: %s\n",(char*)nivel->nombre);

                nivelExiste = esteNivelExiste (orquestador->listaDeNiveles,(char*)nivel->nombre);
                printf ("el valor de la variable nivelExiste es: %d\n",nivelExiste);
                printf ("Chequeando si existe %s con recursos %s\n",nivel->nombre,nivel->recursos);


                if (!nivelExiste){
                        log_error(loggerOrquestador,"ERROR: El personaje quiere jugar a un nivel que no fue creado!!\n");
                        respuesta->nombre = "KO";
                }
                if (nivelExiste)
                {
                        log_info (loggerOrquestador,"El %s existe!, el objetivo del personaje es conseguir estos recursos: %s, CHEQUEANDO SI EXISTEN\n",nivel->nombre,nivel->recursos);
                        recursoExiste = existenRecursosEnNivel(orquestador->listaDeNiveles,nivel->nombre,nivel->recursos);
                        log_info(loggerOrquestador,"Exite recurso: %d\n",recursoExiste);
                        if (recursoExiste)
                        {
                                respuesta->nombre = "OK";
                        }
                        else
                        {
                                respuesta->nombre = "KO";
                        }

                }                                               int seEnvio = socket_enviar(fdEmisor, D_STRUCT_NOMBRENIVEL , respuesta);
                while(!seEnvio) {
                        log_error(loggerOrquestador,"No se pudo enviar la respuesta del nivel %s al personaje\n", respuesta->nombre);
                }

                break;

        default:
                if (estructura != NULL){
                        free(estructura);
                        log_error(loggerOrquestador,"No recibi una estructura de esperada... recibi el tipo: %i\n", tipo);
                }
                log_error(loggerOrquestador,"No recibi una estructura de esperada... recibi el tipo: %i\n", tipo);
                return ;
                break;

                free(nivel);
                free(respuesta);
        }

}



/*--------------------------------- separador ----------------------------------*/


void agregarNivelALista(t_referenciaNivel * referenciaNivel){
        pthread_mutex_lock(&referenciaNivel->semaforoListos);
        list_add(orquestador->listaDeNiveles, referenciaNivel);
        //log_trace(loggerOrquestador, "Se agregó al nivel %s a la lista de niveles.", referenciaNivel->nombre);
        pthread_mutex_unlock(&referenciaNivel->semaforoListos);
}


/*--------------------------------- separador ----------------------------------*/

void crearHiloPlanificador(t_referenciaNivel * referenciaNivel){
        if (pthread_create(&orquestador->hilosPlanificadores[orquestador->listaDeNiveles->elements_count],NULL,mainPlanificador,(void *)referenciaNivel) ==-1)
        {
                log_error(loggerOrquestador,"Error al crear el hilo\n");
                exit(0);
        }
        log_info(loggerOrquestador, "Hilo planificador del nivel %s creado", referenciaNivel->nombre);
        int fdEpoll = epoll_crear();
        if (fdEpoll == -1) {
                //log_error(estructuraNivel->logger,"Error de creacion de epoll. A continuacion se cerrara la conexion");
                referenciaNivel->fdPlanificadorServer = 0;

        }

        referenciaNivel->fdPlanificadorEpoll = fdEpoll;
}

/*--------------------------------- separador ----------------------------------*/


t_struct_direcciones * armarStructDirecciones(char * dirPlanificador){
        t_struct_direcciones * structDirecciones = malloc(strlen(dirPlanificador)+1);

        structDirecciones->direccionPlanificador = string_duplicate(dirPlanificador);

        return structDirecciones;
}

/*--------------------------------- separador ----------------------------------*/


t_referenciaNivel * getNivelPorFd(int fd){
        _Bool esNivelIgual(void * element){
                t_referenciaNivel* nivelDeLista = (t_referenciaNivel*) element;
                return (nivelDeLista->fd == fd);
        }

        return (t_referenciaNivel*) list_find(orquestador->listaDeNiveles, esNivelIgual);
}


/*--------------------------------- separador ----------------------------------*/


void eliminarNivelDeLista(t_referenciaNivel * referenciaNivel){
        log_info(loggerOrquestador,"Eliminando el Nivel %s de la lista de niveles conectados",referenciaNivel->nombre);
        _Bool esElMismoNivel(void * elemento){
                return (referenciaNivel == elemento);
        }
        pthread_mutex_lock(&referenciaNivel->semaforoListos);
        pthread_cancel(referenciaNivel->threadPlanificador);
        int index = list_get_index(orquestador->listaDeNiveles, esElMismoNivel);
        list_remove(orquestador->listaDeNiveles, index);
        log_info(loggerOrquestador,"Destruyendo la referencia del nivel %s",referenciaNivel->nombre);
        destruirReferenciaNivel(referenciaNivel);
        pthread_mutex_unlock(&referenciaNivel->semaforoListos);

}

/*--------------------------------- separador ----------------------------------*/

int esteNivelExiste (t_list *self,char * nombreNivel){
        int c;
        int nivelExiste = 0;
        t_link_element * aux;
        aux = self->head;
        for (c = 0;c<self->elements_count;c++){
                if(string_equals_ignore_case(((t_referenciaNivel *)self->head->data)->nombre,nombreNivel)){
                        nivelExiste = 1;
                        c = self->elements_count;
                }
                self->head =  self->head->next;
        }
        self->head = aux;

        return nivelExiste;
}

/*--------------------------------- separador ----------------------------------*/

int existenRecursosEnNivel (t_list* self, char* nombreNivel, char* recursos ){

        t_referenciaNivel* nivel = getNivel(nombreNivel);
        printf("Nivel: %s\n",nivel->nombre);
        printf("Recursos: %s\n",nivel->recursos);
        int i,j;
        int flag, existenRecursos;
        for(i=0; i < strlen(recursos); i++)
        {
                flag = 0;
                for(j=0; j < strlen(nivel->recursos); j++){
                        if (recursos[i]==nivel->recursos[j]){

                                flag = 1;
                        }
                }

                if(flag == 0){
                        existenRecursos = 0;
                        break;

                } else {
                        existenRecursos = 1;
                }

        }

        return existenRecursos;
}


/*--------------------------------- separador ----------------------------------*/



void recibiMensajeDeNivel (t_referenciaNivel * nivel){
        t_tipoEstructura tipo;
        void * estructura;
        int recibio = socket_recibir(nivel->fd, &tipo, &estructura);
        if(!recibio){
                log_error(loggerOrquestador,"No se recibio correctamente el mensaje del nivel");
                free(estructura);
                return;
        }

        /*
        char * personajesInterbloqueados = string_new();
        char * recursosLiberados = string_new();

        switch(tipo){
                case D_STRUCT_PERSONAJESINTERBLOQUEADOS:
                        string_append(&personajesInterbloqueados, ((t_struct_personajesInterbloqueados*) estructura)->simbolosPersonajes);
                        free( ((t_struct_personajesInterbloqueados*) estructura)->simbolosPersonajes);
                        log_info(loggerOrquestador, "El nivel %s me mandó personajes interbloqueados. Son: %s", nivel->nombre, personajesInterbloqueados);
                        obtenerYEnviar_PersonajeQueMuere(personajesInterbloqueados, nivel);
                break;

                case D_STRUCT_RECURSOSLIBERADOS: //Estructura de tipo string, ejemplo: "FFMHH"
                        string_append(&recursosLiberados, ((t_struct_recursosLiberados*) estructura)->recursosLiberados);
                        free(((t_struct_recursosLiberados*) estructura)->recursosLiberados);
                        log_info(loggerOrquestador, "El nivel %s me mandó recursos que se liberaron: %s", nivel->nombre, recursosLiberados);
                        obtenerYEnviar_RecursosAsignadosYSobrantes(recursosLiberados, nivel);
                break;
                default:
                        log_error(loggerOrquestador, "El nivel envió una estructura de tipo incorrecto. Recibi el tipo %i", tipo);
                        break;

        }
         */
        free(estructura);
}

void destruirNivel(t_referenciaNivel * nivel){
        //free(nivel->nombre);
        //free(personajeAPlanificar);
        //log_destroy(nivel->logger);
/* DESTRUIR LISTAS
        list_destroy_and_destroy_elements(nivel->personajesListos, (void*) destruirReferenciaPersonaje);

        list_destroy_and_destroy_elements(nivel->personajesBloqueados, (void*) destruirReferenciaRecurso);
*/
		//list_destroy(nivel->personajesBloqueados);
		//list_destroy(nivel->personajesDibujados);
		//list_destroy(nivel->personajesListos);
        free(nivel);
}
void hacerAccionesDeFinalDeJuego(void){
        int i, cantNiveles;
        log_info(loggerOrquestador, "Cierro todos los planificadores y nivieles");
        terminoJuego = 1;
        t_referenciaNivel * nivel;
        cantNiveles = orquestador->listaDeNiveles->elements_count;
        for(i=0; i< cantNiveles; i++) {
                nivel = list_get(orquestador->listaDeNiveles, i);
                pthread_cancel(orquestador->hilosPlanificadores[i]);
                log_info(loggerOrquestador, "Hago el join del nivel %s", nivel->nombre);
                //pthread_join(orquestador->hilosPlanificadores[i],NULL);
                mandarMensajeANivelParaCerrarlo(nivel);
                //destruirNivel(nivel);

        }
        log_info(loggerOrquestador, "Se desconectaron todos los usuarios. Llamo a Koopa! (NOT AN ERROR)"); //EN este caso es error para que si o si muestre que termino
        log_destroy(loggerOrquestador);
        printf("Aca ejecuto koopa, esperando por el fs. Que dia sera cuando lleguemos bien aca? \n");

        free(orquestador);

}



void mandarMensajeANivelParaCerrarlo(t_referenciaNivel * nivel) {
        log_info(loggerOrquestador, "Envio mensaje al nivel para que se cierre.");
        int seEnvio = socket_enviarSignal(nivel->fd, S_Orquestador_Nivel_Finalizate);
        if (!seEnvio) {
                log_error(loggerOrquestador, "No se envio correctamente el mensaje a nivel %s para que se mate...", nivel->nombre);
                return;
        }

}
