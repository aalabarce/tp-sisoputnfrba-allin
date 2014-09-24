#include "Personaje.h"
#define MAX_INPUT 1024
char *direccionIP;


int main(int argc, char* argv[]){

        /*leo por parametro del programa el archivo config*/
        path = argv[1];
        if (argc != 2) {
                printf("Argumentos invalidos - Sintaxis correcta: personaje ruta");
                exit(1);
        }

        struct stat buffer;
        if (lstat(argv[1], &buffer) == -1) {
                printf("Archivo config incorrecto.");
                exit(1);
        }

        /*****Empieza el main*****/
        signal(SIGTERM, (void*) matarPersonajePorSignal);
        signal(SIGUSR1, (void*) aumentarVidaPorSignal);
        signal(SIGINT,(void*) manejarControlC);

        pthread_mutex_lock(&mPersonaje);
        personaje = crearPersonaje(path);
        int i=0;
        t_link_element* nivelactual = personaje->planDeNiveles->head;
        for(i=0;i<personaje->planDeNiveles->elements_count;i++){
                ((t_nivelPersonaje *)personaje->planDeNiveles->head->data)->primerObjetivo= ((t_nivelPersonaje *)personaje->planDeNiveles->head->data)->objetivosNivel->head->data;
                printf("Nivel: %s\n",((t_nivelPersonaje*)personaje->planDeNiveles->head->data)->nombre);
                printf("Primer Objetivo %c\n",((t_nivelPersonaje *)personaje->planDeNiveles->head->data)->primerObjetivo->simbolo);
                ((t_nivelPersonaje *)personaje->planDeNiveles->head->data)->reinicio =0;
                personaje->planDeNiveles->head =personaje->planDeNiveles->head->next;
        }
        personaje->planDeNiveles->head = nivelactual;
        char* dirLog = string_from_format("%s.log", personaje->nombre);
        logger = log_create(dirLog, personaje->nombre, true, LOG_LEVEL_TRACE);
        free(dirLog);
        //int cantidadDeNiveles = list_size(personaje->planDeNiveles);
        //pthread_t* vectorDeHilos = malloc(sizeof(pthread_t) * cantidadDeNiveles);
        avisarOrquestadorIniciaJuego();
        PasarPlanNiveles();
        pthread_mutex_unlock(&mPersonaje);


        int k=0;
        t_link_element *aux = personaje->planDeNiveles->head;
        while(aux!=NULL) //recorre el plan de niveles
        {
                t_personajeAux *info_personaje=malloc(sizeof(t_personajeAux));
                info_personaje->nombre = personaje->nombre;
                info_personaje->simbolo = personaje->simbolo;
                info_personaje->posX=1;
                info_personaje->posY=1;
                info_personaje->path = personaje->path;
                info_personaje->nivelActual = (t_nivelPersonaje*)aux->data;
                info_personaje->ultimoMovimiento=HORIZONTAL;
                int fdEpoll = epoll_crear();
                if (fdEpoll == -1) {
                        log_error(logger, "Error al crear epoll");
                        printf("Error al crear epoll");
                        exit(1);
                }
                info_personaje->objetivoActual.objetivo = info_personaje->nivelActual->objetivosNivel->head;
                info_personaje->fdPersonaje = fdEpoll;
                log_info(logger,"Creando hilo personaje");
                pthread_create(&hilo[k+1],NULL,(void*) &jugarNivel, (void *)info_personaje); //lanza multiples hilos
                //vectorDeHilos[k] = *hilo;
                aux = aux->next;
                k+=1;
        }

        for(i=0;i<k;i++){
                pthread_join(hilo[i+1],NULL);
        }

        if(personaje->cantVidas > 0){
        avisarOrquestadorTermineJuego();
        destruirPersonaje();
        log_info(logger,  "Terminaste todos los niveles! Idolo!");
        log_destroy(logger);
        }else{
                log_info(logger,"Se te acabaron las vidas, queres volver a jugar?(Y/N)");
                printf("Se te acabaron las vidas, queres volver a jugar?(Y/N)\n");
                char buffer[MAX_INPUT];
                while (fgets(buffer, MAX_INPUT, stdin) != NULL)
                {
                    buffer[strlen(buffer) - 1] = '\0';
                    buffer[0] = toupper(buffer[0]);
                    if (strcmp(buffer, "Y") == 0)
                    {
                        printf("Pusiste que si!\n");

                        reiniciarJuego();
                        break;
                    }
                    else if (strcmp(buffer, "N") == 0)
                    {
                        avisarOrquestadorTermineJuego();
                        destruirPersonaje();
                        printf("No quisiste volver a jugar, mal por vos\n");
                        log_info(logger,"No quisiste volver a jugar, mal por vos");
                        log_destroy(logger);
                        break;
                    }
                    else
                        printf("Tenes que poner Y/N nada mas!\n");
                }

        }
        return 0;
}


/*
 * Nombre: validarConfigPersonaje/1
 * Argumentos:
 *                 - config (t_config *) (Puntero a una config)
 *
 * Devuelve:
 *                 bool (int por que no tengo bool :D)
 *
 * Funcion: Chequea que se haya creado correctamente el archivo config del personaje.
 * Se fija todos los parametros que tiene que tener e imprime un cartel si no estan.
 * Si hay algun error, devuelve 0. Si esta OK, devuelve 1 :)
 */
int validarConfigPersonaje(t_config * config) {
        int configOk=1;
        char* strObjetivo;

        if (!config_has_property(config, "nombre")) {
                printf("No ha incluido \"nombre\" en el archivo de configuracion.");
                configOk = 0;
        }

        if (!config_has_property(config, "simbolo")) {
                printf("No ha incluido \"simbolo\" en el archivo de configuracion.");
                configOk = 0;
        }

        if (!config_has_property(config, "vidas")) {
                printf("No ha incluido \"vidas\" en el archivo de configuracion.");
                configOk = 0;
        }

        if (!config_has_property(config, "orquestador")) {
                printf("No ha incluido \"orquestador\" en el archivo de configuracion.");
                configOk = 0;
        }

        if (!config_has_property(config, "planDeNiveles")) {
                printf("No ha incluido \"planDeNiveles\" en el archivo de configuracion.");
                configOk = 0;
        }

        if (config_has_property(config, "planDeNiveles")) {
                int i=0;
                char ** arrayNiveles = config_get_array_value(config, "planDeNiveles");

                while( arrayNiveles[i] != NULL) {
                        strObjetivo = string_from_format("obj[%s]", arrayNiveles[i]);

                        if (!config_has_property(config, strObjetivo) ) {
                                printf("No ha incluido \"%s\" en el archivo de configuracion.", strObjetivo);
                                configOk = 0;
                        }

                        free(strObjetivo);
                        i++;
                }

                string_iterate_lines(arrayNiveles, (void*) free);
                free(arrayNiveles);
        }

        return configOk;
}

/*
 * Nombre: crearPersonaje/1
 * Argumentos:
 *                 - ruta (char*)
 *
 * Devuelve:
 *                 t_personaje * (Puntero a un personaje)
 *
 * Funcion: Crea un personaje nuevo a partir de un archivo que se le pasa como argumento.
 */
t_personaje* crearPersonaje(char *ruta){
        t_config *config = config_create(ruta);

        if ( validarConfigPersonaje(config) ) {
                t_personaje *nuevoPersonaje = malloc(sizeof(t_personaje));
                nuevoPersonaje->nombre = strdup(config_get_string_value(config, "nombre"));
                nuevoPersonaje->simbolo = (config_get_string_value(config, "simbolo"))[0];
                pthread_mutex_lock(&mVidas);
                nuevoPersonaje->cantVidas = config_get_int_value(config, "vidas");
                nuevoPersonaje->cantVidasOriginales = config_get_int_value(config, "vidas");
                pthread_mutex_unlock(&mVidas);
                nuevoPersonaje->orquestador = strdup(config_get_string_value(config, "orquestador"));
                nuevoPersonaje->posX = 1;
                nuevoPersonaje->posY = 1;
                nuevoPersonaje->planDeNiveles = crearPlanDeNiveles(config);
                nuevoPersonaje->nivelActual.nivel = nuevoPersonaje->planDeNiveles->head;
                nuevoPersonaje->path = ruta;
                nuevoPersonaje->personajes = list_create();
                nuevoPersonaje->objetivoActual.objetivo = ((t_nivelPersonaje *) nuevoPersonaje->nivelActual.nivel->data)->objetivosNivel->head;
                config_destroy(config);
                return nuevoPersonaje;
        }
        else {
                config_destroy(config);
                exit(1);
        }
}

/*
 * Nombre: crearPlanDeNiveles/1
 * Argumentos:
 *                 - config (t_config *) (Puntero a una config)
 *
 * Devuelve:
 *                 t_list * (es un puntero a una lista creada por la funcion)
 *
 * Funcion: Crea una nueva lista con planes de nivel asiociados.
 * Los niveles se van a ir agregando de a uno en la lista y despues se retorna.
 * Cuidado que esta llama primero a crearObjetivosNivel para que cree todos los objetivos asociados al nivel.
 * Una vez que crea los objetivos, agrega el nivel a la lista.
 */
t_list * crearPlanDeNiveles(t_config *config){
        t_list *plan = list_create();
        t_nivelPersonaje *nivel;
        char ** arrayNiveles = config_get_array_value(config, "planDeNiveles");
        int i=0;

        while( arrayNiveles[i] != NULL) {
                nivel = malloc(sizeof(t_nivelPersonaje));
                nivel->nombre = strdup(arrayNiveles[i]);
                nivel->objetivosNivel = crearObjetivosNivel(config, arrayNiveles[i]);
                nivel->personajeSigueJugando = 1;
                nivel->tengoPendienteAgregarRecurso = 0;
                nivel->tengoQuePedirPosicionRecursos = 1;
                nivel->flag =1;
                printf("Creo nivel %s\n", nivel->nombre );

                list_add_in_index(plan, list_size(plan), nivel);
                i++;
        }

        string_iterate_lines(arrayNiveles, (void*) free);
        free(arrayNiveles);

        return plan;
}

/*
 * Nombre: crearObjetivosNivel/1
 * Argumentos:
 *                 - config (t_config *) (Puntero a una config)
 *                 - nivel (char *) (string con nombre del nivel)
 *
 * Devuelve:
 *                 t_list * (es un puntero a una lista creada por la funcion)
 *
 * Funcion: Crea una nueva lista con los objetivos del nivel asiociados.
 * Los objetivos se van a ir agregando de a uno en la lista y despues se retorna.
 */
t_list * crearObjetivosNivel(t_config *config, char * nivel){
        t_list *objetivos = list_create();
        t_objetivoNivel *objetivo;
        char* strObjetivo = string_from_format("obj[%s]", nivel);
        char ** arrayObjetivos = config_get_array_value(config, strObjetivo);
        int i=0;

        while( arrayObjetivos[i] != NULL) {
                objetivo = malloc(sizeof(t_objetivoNivel));
                objetivo->simbolo = arrayObjetivos[i][0];

                printf("creo objetivo %c\n", objetivo->simbolo  );//AGREGAR ESTA FUNCION PARA VER COMO VA CREANDO LOS OBJETIVOS

                list_add_in_index(objetivos, list_size(objetivos), objetivo);
                i++;
        }

        string_iterate_lines(arrayObjetivos, (void*) free);
        free(arrayObjetivos);
        free(strObjetivo);
        return objetivos;
}

void avisarOrquestadorIniciaJuego(void) {
        log_info(logger, "Me conecto al orquestador y le digo que inicio el juego");

        int fdOrquestador = socket_crearYConectarCliente(socket_ip(personaje->orquestador), socket_puerto(personaje->orquestador));
        if (fdOrquestador== -1){
                log_error(logger, "Errores al conectarse al orquestador IP: %s:%d", socket_ip(personaje->orquestador), socket_puerto(personaje->orquestador));
                exit(1);
        }

        t_struct_signal * signal_struct = malloc(sizeof(t_struct_signal));
        signal_struct->signal=S_Personaje_Orquestador_EmpeceJuego;
        int seEnvio = socket_enviar(fdOrquestador, D_STRUCT_SIGNAL , signal_struct);
        if(!seEnvio) {
                log_error(logger, "No se pudo enviar al orquestador el nombre del nivel que requiero");
                socket_cerrarConexion(fdOrquestador);
                finalizacionInesperada();
        }
        log_info(logger, "Signal de comienzo de juego enviado");
        free(signal_struct);
        socket_cerrarConexion(fdOrquestador);
}

void avisarOrquestadorTermineJuego(void){
        log_info(logger, "Me conecto al orquestador IP: %s", personaje->orquestador);
        int fdOrquestador = socket_crearYConectarCliente(socket_ip(personaje->orquestador), socket_puerto(personaje->orquestador));

        log_info(logger,  "Aviso a orquestador que termine el juego");
        int seEnvio = socket_enviarSignal(fdOrquestador, S_Personaje_Orquestador_TermineJuego);
        if(!seEnvio){
                log_error(logger, "No se envio el aviso al orquestador que termine el juego...");
                exit(1);
        }

        log_info(logger, "Cierro conexion con el orquestador");
        socket_cerrarConexion(fdOrquestador);
}

void avisarOrquestadorTermineJuegoMal(void){
        int fdOrquestador = socket_crearYConectarCliente(socket_ip(personaje->orquestador), socket_puerto(personaje->orquestador));

        log_info(logger,  "Aviso a orquestador que termine el juego MAL (nada grave)");
        int seEnvio = socket_enviarSignal(fdOrquestador, Termine_Juego);
        if(!seEnvio){
                log_error(logger, "No se envio el aviso al orquestador que termine el juego...");
                exit(1);
        }

        log_info(logger, "Cierro conexion con el orquestador");
        socket_cerrarConexion(fdOrquestador);
}



void finalizacionInesperada(void) {
        avisarOrquestadorTermineJuegoMal();

        destruirPersonaje();
        log_error(logger,  "Terminaste de jugar! CON ERRORES! (Alguna cagada te mandaste, pero la pudiste zafar)");
        log_destroy(logger);
        exit(1);
}

t_nivelPersonaje *dataNivelActual(void){
        if (personaje->nivelActual.nivel != NULL) {
                return (personaje->nivelActual.nivel->data);
        }
        else {
                return NULL;
        }
}

char* string_concat(const char* primero, const char* segundo)
{
        char *strconcat;
        strconcat = (char*)malloc(strlen(primero)+strlen(segundo)+1);
        if(strconcat == NULL){
                log_error(logger,"String concat: la asignacion fallo!");
        }
        else{
                memcpy(strconcat,primero,strlen(primero));
                memcpy(strconcat+strlen(primero),segundo,strlen(segundo));
        }
        return strconcat;

}

char* crear_streamPersonaje(){
        char* stream1=malloc(sizeof(char));
        int i = dataNivelActual()->objetivosNivel->elements_count;
        printf("cantidad %d\n",i);
        int j=0;
        int tamano=sizeof(char);
        for(j=0;j<i;j++)
        {
                char c =((t_objetivoNivel*)list_get(dataNivelActual()->objetivosNivel,j))->simbolo;
                if(j==0)
                {

                        memcpy(stream1,&c,sizeof(char));

                }
                else{
                        stream1=realloc(stream1,(j+1)*tamano);
                        memcpy(stream1+(j)*tamano,&c,sizeof(char));

                }

        }
        stream1=realloc(stream1,(j+1)*tamano);
        memset(stream1+(j)*tamano,'\0',sizeof(char));
        return stream1;

}

char* crear_stream(t_list* list){
        char* stream;
        int i=0;
        while(list->elements_count < i){
                stream = (char*)list->head->data;
                list->head = list->head->next;
                stream=string_concat(stream,(char*)list->head->data);
                i++;
        }

        return stream;
}

t_nivelPersonaje* getNivel_Personaje(char* nombreNivel){
        //ESTA FUNCION LA HAGO ACA ADENTRO, PORQUE NECESITO EL NOMBRE DEL NIVEL  Y ESTO NO SE PUEDE MANDAR POR LA FUNCION DE LIST FIND.
        _Bool esNivelIgual(void * element){
                t_nivelPersonaje* nivelDeLista = (t_nivelPersonaje*) element;
                return ( strcmp(nombreNivel, nivelDeLista->nombre) == 0);
        }

        return (t_nivelPersonaje*) list_find(personaje->planDeNiveles, esNivelIgual);
}

t_nivelPersonaje* getNivel_PersonajexFD(int fd){
        //ESTA FUNCION LA HAGO ACA ADENTRO, PORQUE NECESITO EL NOMBRE DEL NIVEL  Y ESTO NO SE PUEDE MANDAR POR LA FUNCION DE LIST FIND.
        _Bool esNivelIgual(void * element){
                t_nivelPersonaje* nivelDeLista = (t_nivelPersonaje*) element;
                return ( nivelDeLista->fdPlanificadorNivel == fd);
        }

        return (t_nivelPersonaje*) list_find(personaje->planDeNiveles, esNivelIgual);
}


int PasarPlanNiveles(void){

        int fdOrquestador = socket_crearYConectarCliente(socket_ip(personaje->orquestador), socket_puerto(personaje->orquestador));
        log_info(logger, "Me conecto al orquestador IP: %s, FD: %d", personaje->orquestador,fdOrquestador);
        if (fdOrquestador== -1){
                log_error(logger, "Errores al conectarse al orquestador IP: %s:%d", socket_ip(personaje->orquestador), socket_puerto(personaje->orquestador));
                finalizacionInesperada();
        }
        log_info(logger,"Se creo el cliente y se espera envio");

        // CREO EL STREAM DE NIVELES A PASAR
        t_struct_datosNivel* stream = malloc(sizeof(t_struct_datosNivel));

        int cant = personaje->planDeNiveles->elements_count;
        int i;
        int noexistenivel=0;
        void * estructuraRecibida;
        t_tipoEstructura tipoRecibido;
        t_link_element* nivelactual = personaje->nivelActual.nivel;
        for(i=0;i<cant;i++)
        {
                stream->nombre = dataNivelActual()->nombre;
                stream->recursos = crear_streamPersonaje();
                log_info(logger,"Enviando datos nivel %s y %s",stream->nombre,stream->recursos);

                int seEnvio = socket_enviar(fdOrquestador, D_STRUCT_DATONIVELYRECURSO, stream);
                if(!seEnvio)
                {
                        log_error(logger, "No se pudo enviar al orquestador el nombre del nivel que requiero");
                        socket_cerrarConexion(fdOrquestador);
                        finalizacionInesperada();
                }

                int recibi = socket_recibir(fdOrquestador, &tipoRecibido, &estructuraRecibida);
                if(!recibi){
                        log_error(logger, "No se pudo recibir del orquestador las direcicones de nivel y planificador");
                        socket_cerrarConexion(fdOrquestador);
                        finalizacionInesperada();
                }

                if(tipoRecibido != D_STRUCT_NOMBRENIVEL ) {
                        log_error(logger, "Se recibio mensaje incorrecto del planificador. Esperaba tipo '%d' y recibi '%d'", D_STRUCT_DIRECCIONES, tipoRecibido);
                        socket_cerrarConexion(fdOrquestador);
                        finalizacionInesperada();
                }

                char* resp = ((t_struct_nombreNivel*) estructuraRecibida)->nombre;
                log_info(logger,"Resuesta recibida: %s",resp);
                if( strcmp(resp,"KO") == 0 )
                {
                        noexistenivel++;
                        log_info(logger,"El nivel %s no exite o el nivel no posee los recursos solicitados",stream->nombre);
                        socket_cerrarConexion(fdOrquestador);
                        finalizacionInesperada();
                        break;
                }
                personaje->nivelActual.nivel = personaje->nivelActual.nivel->next;

        }
        personaje->nivelActual.nivel = nivelactual;
        free(stream->recursos);
        free(stream);
        socket_cerrarConexion(fdOrquestador);
        return 1;
}

void comunicarseConOrquestador(t_personajeAux* nivelEnJuego){

        log_info(logger, "Comunicandome con el orquestador");

        log_info(logger, "Me conecto al orquestador IP: %s", personaje->orquestador);
        int fdOrquestador = socket_crearYConectarCliente(socket_ip(personaje->orquestador), socket_puerto(personaje->orquestador));

        if (fdOrquestador== -1){
                log_error(logger, "Errores al conectarse al orquestador IP: %s:%d", socket_ip(personaje->orquestador), socket_puerto(personaje->orquestador));
                finalizacionInesperada();
        }
        log_info(logger, "Se creo la comunicacion con el orquestador y se va a realizar el envio de paquetes");

        /*ENVIO EL NOMBRE DEL NIVEL ACTUAL*/
        t_struct_nombreNivelYSimbolo* stringNombreNivel = malloc(sizeof(t_struct_nombreNivelYSimbolo));
        stringNombreNivel->nombre = nivelEnJuego->nivelActual->nombre;
        stringNombreNivel->simboloPersonaje = personaje->simbolo;

        log_info(logger, "Pido al orquestador que me conecte al plnificador del nivel: '%s'", stringNombreNivel->nombre);
        int seEnvio = socket_enviar(fdOrquestador, D_STRUCT_CONECTAME , stringNombreNivel);
        if(!seEnvio) {
                log_error(logger, "No se pudo enviar al orquestador el nombre del nivel que requiero");
                socket_cerrarConexion(fdOrquestador);
                finalizacionInesperada();
        }

        if (epoll_agregarSocketCliente(nivelEnJuego->fdPersonaje, fdOrquestador)== -1){
                log_error(logger, "Error al agregar personaje o nivel al epoll");
                return ;
        }else{
                log_info(logger, "Agregue el fd al epoll");
        }
        nivelEnJuego->nivelActual->fdPlanificadorNivel = fdOrquestador;
        list_add(personaje->personajes,nivelEnJuego);
        free(stringNombreNivel);
}

/*
 * Nombre: destruirNivel/1
 * Argumentos:
 *                 - self (t_nivelPersonaje *) (Puntero a un nivel del personaje)
 *
 * Devuelve:
 *                 void
 *
 * Funcion: Destrulle el nivel del personaje pasado por argumento, liberando la memoria que ocupaba este y tambien la lista de objetivos que estaba contenida en el nivel.
 */
void destruirNivel(t_nivelPersonaje *self) {
        log_trace(logger, "Destruyo nivel %s",self->nombre);
        free(self->nombre);
        list_destroy_and_destroy_elements(self->objetivosNivel, (void*) free); // ESTA LE PUEDO DAR FREE DIRECTAMENTE, PORQUE LA ESTRUCTURA SOLO TIENE UNA VARIABLE Y ES CHAR (NO ME RESERVA MAS MEMORIA DINAMICA)
        free(self);
}

/*
 * Nombre: destruirPersonaje/1
 * Argumentos: ninguno
 *
 * Devuelve:
 *                 void
 *
 * Funcion: Destrulle el personaje del proceso, liberando la memoria que ocupaba.
 */
void destruirPersonaje(void){
        log_info(logger, "El personaje se esta destruyendo!");
        free(personaje->nombre);
        free(personaje->orquestador);
        list_destroy_and_destroy_elements(personaje->planDeNiveles, (void*) destruirNivel);         // NO ES UN SIMPLE FREE, PORQUE TENGO UN A LISTA DE LISTAS, ENTOCNES HAGO UNA FUNC APARTE PARA ELIMINAR NIVEL POR NIVEL
        free(personaje);
}


t_objetivoNivel *dataObjetivoActual(t_personajeAux* persaux){
        if (persaux->objetivoActual.objetivo != NULL) {
                return (persaux->objetivoActual.objetivo->data);
        }
        else {
                return NULL;
        }
}



int autoMover(t_personajeAux* nivelEnJuego){
        unsigned int personajeX = nivelEnJuego->posX, personajeY = nivelEnJuego->posY;
        unsigned int objetivoX = nivelEnJuego->objetivoActual.posX, objetivoY = nivelEnJuego->objetivoActual.posY;
        t_posicion * posxposy = malloc(sizeof(t_posicion));
        log_info(logger, "Me quiero mover a la posicion: (%i;%i)", objetivoX, objetivoY);
        if( ( (nivelEnJuego->ultimoMovimiento == VERTICAL) && (personajeX != objetivoX)) || (personajeY    == objetivoY)) {

                if (personajeX < objetivoX) {
                        (nivelEnJuego->posX)++;
                        nivelEnJuego->ultimoMovimiento = HORIZONTAL;
                }
                else {
                        (nivelEnJuego->posX)--;
                        nivelEnJuego->ultimoMovimiento = HORIZONTAL;
                }
        }
        else {
                if ( personajeY != objetivoY ) {
                        if (personajeY < objetivoY) {
                                (nivelEnJuego->posY)++;
                                nivelEnJuego->ultimoMovimiento=VERTICAL;
                        }
                        else {
                                (nivelEnJuego->posY)--;
                                nivelEnJuego->ultimoMovimiento=VERTICAL;
                        }
                }

        }

        log_info(logger, "Me movi a la posicion: (%i;%i)", nivelEnJuego->posX, nivelEnJuego->posY);

        posxposy->posX = nivelEnJuego->posX;
        posxposy->posY = nivelEnJuego->posY;

        mandarPosicionAMoverAPlanificador(posxposy,nivelEnJuego);
        recibirSignalMoviCorrectamente(nivelEnJuego);
        free(posxposy);
        return 1;

}

void mandarPosicionAMoverAPlanificador(t_posicion * posxposy,t_personajeAux* nivelEnJuego){
        int fdNivel = nivelEnJuego->nivelActual->fdPlanificadorNivel;

        int seEnvio = socket_enviar(fdNivel,D_STRUCT_POSXPOSY, posxposy);
        if(!seEnvio){
                log_error(logger, "No se envio el aviso al nivel que me mueva");
                finalizacionInesperada();
        }
        log_info(logger,"Le envio las posiciones al planificador (%d,%d)",posxposy->posX,posxposy->posY);

}


void recibirSignalMoviCorrectamente(t_personajeAux* nivelEnJuego){

        t_tipoEstructura tipo;
        void * estructura;
        int recibio = socket_recibir(nivelEnJuego->nivelActual->fdPlanificadorNivel, &tipo, &estructura);
        if(!recibio){
                log_error(logger,"No se recibio correctamente el mensaje del fd %d \n", nivelEnJuego->nivelActual->fdPlanificadorNivel);
                return;
        }
        log_info(logger, "TIPO DE SEÑAL: %d", tipo);

        if (tipo == D_STRUCT_SIGNAL) {
                t_signal signal = ((t_struct_signal*) estructura)->signal;
                switch (signal) {
                case S_Nivel_Personaje_ConfirmaQueSeMovio:
                        log_info(logger, "Me confirmo el nivel que me movio correctamente");
                        break;
                default:
                        log_error(logger,"Me mandaron mal una signal, me llego %d", signal);
                        return;
                }
        }

        else {
                log_info(logger, "Recibi una estructura del tipo %d \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n", tipo);
        }

}


void pedirNivelPosicionRecurso(int fdDeNivel,t_personajeAux* nivelEnJuego){
        log_info(logger,"Pidiendo la posicion de un recurso");
        int fdNivel = fdDeNivel;//Este es el FD de la conexion con le planificador.
        void * estructuraRecibio;
        t_posicion* posicionXY;
        t_tipoEstructura tipoRecibido;
        if (dataObjetivoActual(nivelEnJuego) != NULL) {
                t_struct_char * charEstructura = malloc(sizeof(t_struct_char));
                charEstructura->letra = dataObjetivoActual(nivelEnJuego)->simbolo;
                log_info(logger, "Pido al nivel la posicion del recuro: %c", charEstructura->letra);

                int seEnvio = socket_enviar(fdNivel, D_STRUCT_PEDIRPOSICIONRECURSO, charEstructura );
                if(!seEnvio){
                        log_error(logger, "No se envio el pedido de posicion del recurso %c al nivel", charEstructura->letra);
                        finalizacionInesperada();
                }
                log_info(logger,"Envie el pedido del recurso %c",charEstructura->letra);
                free(charEstructura);

                int recibio = socket_recibir(fdNivel, &tipoRecibido, &estructuraRecibio);
                if (!recibio) {
                        log_error(logger, "Error al recibir el pedido de posicion del recurso %c al nivel", charEstructura->letra);
                        finalizacionInesperada();
                }

                if(tipoRecibido == D_STRUCT_POSXPOSY) {
                        posicionXY = (t_posicion*) estructuraRecibio;
                        nivelEnJuego->objetivoActual.posX = posicionXY->posX;
                        nivelEnJuego->objetivoActual.posY = posicionXY->posY;
                        log_info(logger,  "Me devolvio la posicion del recurso %c es (%i;%i)", dataObjetivoActual(nivelEnJuego)->simbolo, posicionXY->posX , posicionXY->posY);
                        free(posicionXY);
                }
                else {
                        if (tipoRecibido == D_STRUCT_SIGNAL) {
                                log_info(logger,  "Me devolvio una senial");
                                t_signal signal = ((t_struct_signal*) estructuraRecibio)->signal;
                                if (signal == S_Planificador_Personaje_Matate){ //Aca me mataron y vamos a reiniciar
                                        nivelEnJuego->objetivoActual.objetivo->data = nivelEnJuego->nivelActual->primerObjetivo;
                                        printf("Primero Objetivo: %c\n",((t_objetivoNivel *)nivelEnJuego->objetivoActual.objetivo->data)->simbolo);
                                        //printf("Segundo Objetivo: %c\n",((t_objetivoNivel *)nivelEnJuego->objetivoActual.objetivo->next->data)->simbolo);
                                        nivelEnJuego->nivelActual->flag =0;
                                        nivelEnJuego->nivelActual->tengoQuePedirPosicionRecursos = 1;
                                        nivelEnJuego->nivelActual->reinicio =0;
                                }else if (signal == S_Planificador_Personaje_HabilitarMover){
                                		pedirNivelPosicionRecurso(fdNivel,nivelEnJuego);
                                }
                                else{
                                        log_error(logger, "Recibio mal el tipo. Se esperaba '%d' y recibio '%d'", S_Planificador_Personaje_Matate, signal);
                                        finalizacionInesperada();
                                }

                        }else{
                                log_error(logger, "Recibio mal el tipo. Se esperaba '%d' y recibio '%d'", D_STRUCT_POSXPOSY, tipoRecibido);
                                finalizacionInesperada();
                        }
                }

        }

}


t_personajeAux*  cambiarNivel(int fd){

        int cant = personaje->personajes->elements_count;
        int i = 0;
        for(i=0;i<cant;i++){
                t_personajeAux* nivelactual = list_get(personaje->personajes,i);
                if(nivelactual->nivelActual->fdPlanificadorNivel == fd){
                        return nivelactual;

                }
        }
        return 0;
}

void cambiarObjetivo(t_personajeAux* persaux){
        log_trace(logger, "Cambio de objetivo");
        if (persaux->objetivoActual.objetivo != NULL ){
                persaux->objetivoActual.objetivo = persaux->objetivoActual.objetivo->next;
                printf("Objetivo actual: %c\n",dataObjetivoActual(persaux)->simbolo);
                persaux->objetivoActual.posX = 0;
                persaux->objetivoActual.posY = 0;
        }


}



t_signal recibirPlanificadorSignal(t_personajeAux* nivelEnJuego) {

        int fdPlanificador = nivelEnJuego->nivelActual->fdPlanificadorNivel;
        t_signal signal;

        log_info(logger, "Recibi mensaje del planificador");
        int recibio = socket_recibirSignal(fdPlanificador, &signal);
        if(!recibio){
                log_error(logger, "No se recibio correctamente la señal del planificador");
                finalizacionInesperada();
        }

        if(signal== S_ERROR) {
                log_error(logger, "No se recibio una señal incorrecta: '%d'", signal);
                finalizacionInesperada();
        }

        log_info(logger, "Recibi signal del planificador: '%d'", signal);

        return signal;
}


int terminoNivel(t_personajeAux* persaux){
        if (persaux->objetivoActual.objetivo != NULL) {
                return (persaux->objetivoActual.objetivo->next == NULL);
        }
        else {
                return 1;
        }
}


int terminoJuego(t_personajeAux* persaux){
        /*        if (persaux->nivelActual.nivel != NULL) {
                return (persaux->nivelActual.nivel->next == NULL);
        }
        else {
                return 1;
        }*/
        return 1;
}

int estaEnObjetivo(t_personajeAux* persaux){
        unsigned int personajeX = persaux->posX;
        unsigned int personajeY = persaux->posY;

        unsigned int objetivoX = persaux->objetivoActual.posX;
        unsigned int objetivoY = persaux->objetivoActual.posY;

        return ( (personajeX == objetivoX) && (personajeY == objetivoY) );
}


/*
void avisarPlanificadorMeBloquie(void){

        int fdPlanificador = personaje->nivelActual.fdPlanificador;
        t_struct_recursoBloqueante * recursoBloqueante = malloc(sizeof(t_struct_recursoBloqueante));
        recursoBloqueante->recurso = dataObjetivoActual()->simbolo;

        log_info(logger,  "Aviso a planificador que me bloquie por el recurso %c", recursoBloqueante->recurso);
        int seEnvio = socket_enviar(fdPlanificador, D_STRUCT_RECURSOBLOQUEANTE, recursoBloqueante);
        if(!seEnvio){
                log_error(logger, "No se envio el aviso al planificador que me bloquie por el recurso %c", recursoBloqueante->recurso);
                free(recursoBloqueante);
                finalizacionInesperada();
        }

        free(recursoBloqueante);
}
 */


void logicaPlanificadorMoverPersonaje(int * punteroPersonajeSigueJugando, int* punteroTengoPendienteAgregarRecurso, t_personajeAux* persaux){
		int valor =0;
        log_info(logger, "Llegue a la logica para moverme");

        if(!persaux->nivelActual->flag){

                persaux->nivelActual->tengoQuePedirPosicionRecursos = 0;

        }

        int meDioRecurso; //Utilizada para ver la respuesta del nivel, a ver si le dieron el recurso o no, que pido.

        int fdNivel = persaux->nivelActual->fdPlanificadorNivel;

        if(*punteroTengoPendienteAgregarRecurso){

                log_info(logger,  "Tenia recurso pendiente a asignar '%c'... Me lo tomo ahora", dataObjetivoActual(persaux)->simbolo);

                if(terminoNivel(persaux)){

                        desconectarseNivelActual(persaux); //Se desconecta del planificador.

                        //usleep(10000);
                        persaux->nivelActual->tengoQuePedirPosicionRecursos = 1;

                        persaux->nivelActual->flag =1;

                        *punteroPersonajeSigueJugando = 0;

                        persaux->nivelActual->personajeSigueJugando =0;
                        *punteroTengoPendienteAgregarRecurso = 0;
                        valor=1;

                        return;
                }

                else {
                        cambiarObjetivo(persaux); //Cambia al siguiente objetivo.

                        pedirNivelPosicionRecurso(fdNivel,persaux); //Actualiza la posicion del proximo recurso.

                        *punteroTengoPendienteAgregarRecurso = 0;
                        valor=1;

                }
        }else if ( estaEnObjetivo(persaux) ){//Si estoy parado sobre el objetivo.
        		valor=0;
                //usleep(500);
                meDioRecurso = pedirNivelRecurso(persaux); // Le pide al nivel el recurso que esta parado.


                if (meDioRecurso == 0){ //Si NO me dio el recurso


                        *punteroTengoPendienteAgregarRecurso = 1;


                }
                else {        // Si me dio el recurso.

                        if(meDioRecurso ==1)//Si termino todos los objetivos del nivel

                        {
                                desconectarseNivelActual(persaux); //Se desconecta del plani y del nivel.

                                persaux->nivelActual->tengoQuePedirPosicionRecursos = 1;

                                persaux->nivelActual->flag =1;

                                persaux->nivelActual->personajeSigueJugando =0;

                                *punteroPersonajeSigueJugando = 0;

                        }
                        else{

                                int seEnvioSignalAPersonaje = socket_enviarSignal(persaux->nivelActual->fdPlanificadorNivel, No_Termine_Juego);

                                if(!seEnvioSignalAPersonaje){

                                        log_error(logger,"No se envio el aviso al planificador que termine...");

                                }else{

                                        log_info(logger, "Avise al planificador que termine");

                                }
                                persaux->nivelActual->tengoQuePedirPosicionRecursos = 1;

                                persaux->nivelActual->flag =1;

                        }
                }

        }
        if(!persaux->nivelActual->tengoQuePedirPosicionRecursos && !*punteroTengoPendienteAgregarRecurso  &&!valor){ //TODO ver el caso en que se queda bloqueado por falta de instancias.

                log_info(logger,"Moviendo al personaje");

                autoMover(persaux);
        }


}



void desconectarseNivelActual(t_personajeAux* nivelEnJuego){
        log_info(logger,"Me desconecto y aviso al planificador que termine el nivel");
        int seEnvio = socket_enviarSignal(nivelEnJuego->nivelActual->fdPlanificadorNivel, S_Personaje_Orquestador_TermineJuego);
        if(!seEnvio){
                log_error(logger, "No se envio el aviso al planificador que termine el nivel...");
                exit(1);
        }
        if (epoll_eliminarSocketCliente(nivelEnJuego->fdPersonaje, nivelEnJuego->nivelActual->fdPlanificadorNivel)== -1){
                log_error(logger, "Error al eliminar el nivel del epoll");
                return ;
        }
}


t_link_element *getPrimerObjetivo(void){
        t_nivelPersonaje *dataNivel = dataNivelActual();
        if (dataNivel != NULL ) {
                return (dataNivel->objetivosNivel->head);
        }
        else {
                return NULL;
        }
}


void reiniciarNivel(void){
        log_info(logger, "Reinicio nivel\n");
        ((t_nivelPersonaje *) personaje->nivelActual.nivel->data)->tengoPendienteAgregarRecurso = 0;
        personaje->objetivoActual.objetivo = getPrimerObjetivo();
        personaje->posX = 1;
        personaje->posY = 1;
}

void reiniciarJuego(void){
        log_info(logger, "Reinicio juego");
        destruirPersonaje();
        pthread_mutex_lock(&mPersonaje);
        personaje = crearPersonaje(path);
        int i=0;
        t_link_element* nivelactual = personaje->planDeNiveles->head;
        for(i=0;i<personaje->planDeNiveles->elements_count;i++){
                ((t_nivelPersonaje *)personaje->planDeNiveles->head->data)->primerObjetivo= ((t_nivelPersonaje *)personaje->planDeNiveles->head->data)->objetivosNivel->head->data;
                printf("Nivel: %s\n",((t_nivelPersonaje*)personaje->planDeNiveles->head->data)->nombre);
                printf("Primer Objetivo %c\n",((t_nivelPersonaje *)personaje->planDeNiveles->head->data)->primerObjetivo->simbolo);
                ((t_nivelPersonaje *)personaje->planDeNiveles->head->data)->reinicio =0;
                personaje->planDeNiveles->head =personaje->planDeNiveles->head->next;
        }
        personaje->planDeNiveles->head = nivelactual;
        char* dirLog = string_from_format("%s.log", personaje->nombre);
        logger = log_create(dirLog, personaje->nombre, true, LOG_LEVEL_TRACE);
        free(dirLog);
        int cantidadDeNiveles = list_size(personaje->planDeNiveles);
        pthread_t* vectorDeHilos = malloc(sizeof(pthread_t) * cantidadDeNiveles);
        avisarOrquestadorIniciaJuego();
        avisarOrquestadorTermineJuego();
        PasarPlanNiveles();
        pthread_mutex_unlock(&mPersonaje);


        int k=0;
        t_link_element *aux = personaje->planDeNiveles->head;
        while(aux!=NULL) //recorre el plan de niveles
        {
                t_personajeAux *info_personaje=malloc(sizeof(t_personajeAux));
                info_personaje->nombre = personaje->nombre;
                info_personaje->simbolo = personaje->simbolo;
                info_personaje->posX=1;
                info_personaje->posY=1;
                info_personaje->path = personaje->path;
                info_personaje->nivelActual = (t_nivelPersonaje*)aux->data;
                info_personaje->ultimoMovimiento=HORIZONTAL;
                int fdEpoll = epoll_crear();
                if (fdEpoll == -1) {
                        log_error(logger, "Error al crear epoll");
                        printf("Error al crear epoll");
                        exit(1);
                }
                info_personaje->objetivoActual.objetivo = info_personaje->nivelActual->objetivosNivel->head;
                info_personaje->fdPersonaje = fdEpoll;
                log_info(logger,"Creando hilo personaje para el nivel %s",info_personaje->nivelActual->nombre);
                pthread_create(&hilo[k+1],NULL,(void*) &jugarNivel, (void *)info_personaje); //lanza multiples hilos
                vectorDeHilos[k] = *hilo;
                aux = aux->next;
                k+=1;
        }
        for(i=0;i<k;i++){
                pthread_join(hilo[i+1],NULL);
        }

        if(personaje->cantVidas > 0){
                avisarOrquestadorTermineJuego();
                destruirPersonaje();
                log_info(logger,  "Terminaste todos los niveles! Idolo!");
                log_destroy(logger);
        }else{
                log_info(logger,"Se te acabaron las vidas, queres volver a jugar?(Y/N)");
                printf("Se te acabaron las vidas, queres volver a jugar?(Y/N)\n");
                char buffer[MAX_INPUT];
                while (fgets(buffer, MAX_INPUT, stdin) != NULL)
                {
                        buffer[strlen(buffer) - 1] = '\0';
                        buffer[0] = toupper(buffer[0]);
                        if (strcmp(buffer, "Y") == 0)
                        {
                                printf("Pusiste que si!\n");
                                reiniciarJuego();
                                break;
                        }
                        else if (strcmp(buffer, "N") == 0)
                        {
                                avisarOrquestadorTermineJuego();
                                destruirPersonaje();
                                printf("No quisiste volver a jugar, mal por vos\n");
                                log_info(logger,"No quisiste volver a jugar, mal por vos");
                                log_destroy(logger);
                                break;
                        }
                        else
                                printf("Tenes que poner Y/N nada mas!\n");
                }

        }
}


void quitarVida(void){
        personaje->cantVidas -=1;
        log_trace(logger, "Quito vida. Ahora tengo %i", personaje->cantVidas);
}


void avisarNivelReinicio(void){

        log_info(logger,  "Aviso al plani que me reinicie");
        int seEnvio = socket_enviarSignal(personaje->nivelActual.fdPlanificador, S_Personaje_Planificador_Reiniciar);
        if(!seEnvio){
                log_error(logger, "No se mando al plani la signal para reiniciar el nivel.");
                finalizacionInesperada();
        }

}


void matarPersonaje(void) {
        log_info(logger, "Me suicido");
        quitarVida();
        if (personaje->cantVidas == 0){
                log_info(logger,"Ya no tengo vidas disponibles!"); //la proxima vez que lo maten pierde el juego
        }else{
                log_info(logger,"Sigo teniendo vidas disponibles, me quedan %d",personaje->cantVidas);
        }
}


void logicaPlanificadorMatarPersonaje(void) {
        matarPersonaje();
}


void accionMeMandaronAlgo(epoll_data_t epollData){

        int fdNivel = epollData.fd;
        t_signal signalPlanificador;
        pthread_mutex_lock(&mVidas);

        t_personajeAux* nivelEnJuego = cambiarNivel(fdNivel);
        log_info(logger,"El Nivel %s me mando algo",nivelEnJuego->nivelActual->nombre);
        if(nivelEnJuego->nivelActual->reinicio == 1){
                t_config *config = config_create(nivelEnJuego->path);
                nivelEnJuego->nivelActual->objetivosNivel = crearObjetivosNivel(config, nivelEnJuego->nivelActual->nombre);
                nivelEnJuego->objetivoActual.objetivo = nivelEnJuego->nivelActual->objetivosNivel->head;
                nivelEnJuego->nivelActual->flag =0;
                nivelEnJuego->nivelActual->tengoQuePedirPosicionRecursos = 1;
                nivelEnJuego->nivelActual->reinicio =0;
        }
        signalPlanificador = recibirPlanificadorSignal(nivelEnJuego); // ESPERA A QUE EL PLANIFICADOR LE DIGA QUE SE MUEVA.

        switch (signalPlanificador) {
        case S_Planificador_Personaje_HabilitarMover:
                if(nivelEnJuego->nivelActual->personajeSigueJugando){
                        if(nivelEnJuego->nivelActual->tengoQuePedirPosicionRecursos){
                                pedirNivelPosicionRecurso(fdNivel,nivelEnJuego);
                                nivelEnJuego->nivelActual->flag =0;
                                nivelEnJuego->nivelActual->tengoQuePedirPosicionRecursos = 0;
                        }
                        else{

                                logicaPlanificadorMoverPersonaje(&nivelEnJuego->nivelActual->personajeSigueJugando, &nivelEnJuego->nivelActual->tengoPendienteAgregarRecurso, nivelEnJuego);
                        }

                }

                break;
        case S_Planificador_Personaje_Matate:
                nivelEnJuego->nivelActual->tengoPendienteAgregarRecurso = 0;
                logicaPlanificadorMatarPersonaje();
                nivelEnJuego->nivelActual->reinicio =1;
                break;
        }
        pthread_mutex_unlock(&mVidas);
        //usleep(100);

}

void accionSeDesconecto(epoll_data_t epollData){
        int fdNivel = epollData.fd;
        t_struct_nombreNivelYSimbolo* nombreNivel = malloc(sizeof(t_struct_nombreNivelYSimbolo));
        t_personajeAux* NivelaBorrar = cambiarNivel(fdNivel);
        //t_nivelPersonaje* NivelaBorrar = getNivel_PersonajexFD(fdNivel);
        nombreNivel->nombre = NivelaBorrar->nivelActual->nombre;
        nombreNivel->simboloPersonaje = personaje->simbolo;

        pthread_mutex_lock(&mVidas);
        log_info(logger,"Cantidad de vidas antes de morir: %d",personaje->cantVidas);
        if(NivelaBorrar != NULL) { //si es null, significa que el que se desconecto fue un personaje => el orq no tiene que hacer nada.
                log_info(logger,"Cerró conexion el nivel %s",NivelaBorrar->nivelActual->nombre);
                personaje->cantVidas -= 1;
                printf("CAntidad de vidas %d\n",personaje->cantVidas);
                if(personaje->cantVidas > 0){ //Si tengo mas vidas me tengo que reconectar y pedir por ese nivel.
                        NivelaBorrar->nivelActual->reinicio =1;
                        NivelaBorrar->nivelActual->tengoPendienteAgregarRecurso =0;
                        log_info(logger,"Reiniciando la conexion para el nivel '%s'. Cantidad de vidas actual: %d",NivelaBorrar->nivelActual->nombre,personaje->cantVidas);
                        int fdOrquestador = socket_crearYConectarCliente(socket_ip(personaje->orquestador), socket_puerto(personaje->orquestador));
                        if (fdOrquestador== -1){
                                log_error(logger, "Errores al conectarse al orquestador IP: %s:%d", socket_ip(personaje->orquestador), socket_puerto(personaje->orquestador));
                                finalizacionInesperada();
                        }
                        log_info(logger, "Se creo la comunicacion con el orquestador y se va a realizar el envio de paquetes");
                        int seEnvio = socket_enviar(fdOrquestador, D_STRUCT_CONECTAME ,nombreNivel);
                        if(!seEnvio) {
                                log_error(logger, "No se pudo enviar al orquestador el nombre del nivel que requiero");
                                socket_cerrarConexion(fdOrquestador);
                                finalizacionInesperada();
                        }
                        if (epoll_agregarSocketCliente(NivelaBorrar->fdPersonaje, fdOrquestador)== -1){
                                log_error(logger, "Error al agregar personaje o nivel al epoll");
                                return ;
                        }
                        NivelaBorrar->posX =1;
                        NivelaBorrar->posY =1;
                        free(nombreNivel);
                }else{
                	if(personaje->cantVidas < 0){

                	}else{

                		NivelaBorrar->nivelActual->tengoQuePedirPosicionRecursos = 1;
                		NivelaBorrar->nivelActual->flag =1;
                		int j=0;
                		int cant = personaje->planDeNiveles->elements_count;
                		for(j=0;j<cant;j++){

                			if(!string_equals_ignore_case(NivelaBorrar->nivelActual->nombre,((t_nivelPersonaje *)personaje->planDeNiveles->head->data)->nombre)){
                				printf("Estoy con el nivel %s\n",((t_nivelPersonaje *)personaje->planDeNiveles->head->data)->nombre);
                				socket_enviarSignal(((t_nivelPersonaje *)personaje->planDeNiveles->head->data)->fdPlanificadorNivel, S_Termine_Por_Control_C);

                			}
                			printf("Seteando a %s\n",((t_nivelPersonaje *)personaje->planDeNiveles->head->data)->nombre);
                			((t_nivelPersonaje *)personaje->planDeNiveles->head->data)->personajeSigueJugando = 0;
                			personaje->planDeNiveles->head = personaje->planDeNiveles->head->next;
                		}
                	}

                }

        }
        pthread_mutex_unlock(&mVidas);
        printf("Sali de aca\n");

}

int pedirNivelRecurso(t_personajeAux* nivelEnJuego){

        int fdNivel = nivelEnJuego->nivelActual->fdPlanificadorNivel;
        t_signal signal;

        t_struct_pedirRecurso * recurso = malloc(sizeof(t_struct_pedirRecurso));
        recurso->letra = dataObjetivoActual(nivelEnJuego)->simbolo;
        log_info(logger, "Pido a nivel que me asigne el recurso: '%c'", recurso->letra);
        int seEnvio = socket_enviar(fdNivel, D_STRUCT_PEDIRRECURSO, recurso);
        if(!seEnvio){
                log_error(logger, "No se envio el pedido al nivel del recurso");
                free(recurso);
                finalizacionInesperada();
        }

        free(recurso);
        int recibio = socket_recibirSignal(fdNivel, &signal);
        if(!recibio){
                log_error(logger, "No se recibio correctamente la señal del planificador");
                finalizacionInesperada();
        }

        if (signal == S_Nivel_Personaje_TeAsigneRecurso ){
                log_info(logger,  "Me asigno el recurso %c", dataObjetivoActual(nivelEnJuego)->simbolo);
                if(nivelEnJuego->objetivoActual.objetivo->next != NULL){ //Dejo el siguiente objetivo como primero
                        nivelEnJuego->nivelActual->objetivosNivel->head = nivelEnJuego->objetivoActual.objetivo->next;
                        nivelEnJuego->objetivoActual.objetivo = nivelEnJuego->objetivoActual.objetivo->next;
                        log_info(logger,"cambiando a objetivo %c", dataObjetivoActual(nivelEnJuego)->simbolo);
                        return 2;
                }else{
                        return 1;
                }
        }

        if (signal == S_Nivel_Personaje_NoTeAsigneRecurso){
                log_info(logger,  "NO me asigno el recurso %c, voy a esperarlo!", dataObjetivoActual(nivelEnJuego)->simbolo);
                return 0;
        }
        if (signal == S_Planificador_Personaje_Matate){


        }
        /*Si no es ninguna de las dos*/
        log_error(logger, "No se recibio correctamente la señal del planificador, recibi signal %d",signal);

        finalizacionInesperada(); //me llego cualquier cosa, salgo!
        return 0;
}


void aumentarVida(void){
        personaje->cantVidas +=1;
        log_trace(logger, "Aumento vida. Ahora tengo %i", personaje->cantVidas);
}

void * matarPersonajePorSignal(void* attr){
        pthread_mutex_lock(&mVidas);
        matarPersonaje();
        pthread_mutex_unlock(&mVidas);
        return NULL;
}

void * aumentarVidaPorSignal(void* attr){
        pthread_mutex_lock(&mVidas);
        aumentarVida();
        pthread_mutex_unlock(&mVidas);
        return NULL;
}

void manejarControlC(){
        log_trace(logger,"El proceso fue interrumpido por el Administrador");
        int i;
        for(i=0;i<personaje->planDeNiveles->elements_count;i++){
                log_trace(logger,"Avisando al planificador del nivel %s que me mate",((t_personajeAux *)personaje->personajes->head->data)->nivelActual->nombre);
                socket_cerrarConexion(((t_personajeAux*)personaje->personajes->head->data)->nivelActual->fdPlanificadorNivel);
//                int seEnvio = socket_enviarSignal(((t_personajeAux*)personaje->personajes->head->data)->nivelActual->fdPlanificadorNivel, S_Termine_Por_Control_C);
//                if(!seEnvio){
//                        log_error(logger, "No se envio el aviso al planificador que termine el nivel...");
//                        exit(1);
//                }
                personaje->personajes->head = personaje->personajes->head->next;

        }
        destruirPersonaje();
        exit(1);
}


t_link_element* getNivelPorFd2(int fd){
        _Bool esNivelIgual(void * element){
                t_nivelPersonaje* nivelDeLista = (t_nivelPersonaje*) element;
                return (nivelDeLista->fdPlanificadorNivel == fd);
        }

        return (t_link_element*)list_find(personaje->planDeNiveles, esNivelIgual);
}

t_nivelActual * getNivel2(char* nombreNivel){
        _Bool esNivelIgual(void * element){
                t_nivelActual* nivelDeLista = (t_nivelActual*) element;
                return ( strcmp(nombreNivel,((t_nivelPersonaje *) nivelDeLista->nivel->data)->nombre) == 0);
        }

        return (t_nivelActual*) list_find(personaje->planDeNiveles, esNivelIgual);
}

void* jugarNivel(void * estructura)
{

        int no = -1;
        t_personajeAux* nivelEnJuego = (t_personajeAux*)estructura;
        log_info(logger,"jugando nivel %s!",nivelEnJuego->nivelActual->nombre);
        comunicarseConOrquestador(nivelEnJuego);        //Seconecta al orquestador, le pasa los niveles y el simbolo y se conecta a estos.

        while(nivelEnJuego->nivelActual->personajeSigueJugando){
                epoll_escucharGeneral(nivelEnJuego->fdPersonaje, no, NULL, accionMeMandaronAlgo, accionSeDesconecto);
        }
        return 0;
}
