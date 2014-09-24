#include "Procesonivel.h"

pthread_mutex_t enemigosMatando = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mDeadLock = PTHREAD_MUTEX_INITIALIZER;
t_list * ListaCajas;
int ordenDeLlegada = 0;


int main(int argc, char* argv[]){
	//pthread_mutex_init(&mutexListaItems, NULL);
	ListaItems  = list_create();
	/*Leo por parametro del programa el archivo config*/
	char * path = argv[1];
	if (argc != 2) {
		log_error(logger,"nivel -> Sintaxis correcta: personaje y ruta\n");
		exit(1);
	}

	struct stat buffer;
	if (stat(argv[1], &buffer) == -1) {
		log_error(logger,"Archivo incorrecto.\n");
		exit(1);
	}

	/*Empieza el main */
	nivel = crearNivel(path);

	/*  Inicializo logger */
	char* dirLog = string_from_format("%s.log", nivel->nombre);
	logger = log_create(dirLog, nivel->nombre, false, LOG_LEVEL_TRACE);
	free(dirLog);

	/* Inicializo nivel-gui */
	nivel_gui_inicializar();
	nivel_gui_get_area_nivel(&max_filas, &max_cols);

	/*      Agrego cajas y enemigos */
	agregarListaItemsCajas();
	if (nivel->enemigos > 0)
		agregarListaEnemigos();

	personajesActuales = list_create();

	epoll_agregarFdInotify(nivel->fdEpoll, nivel->fdInotify);
	conectarOrquestador();
	pthread_t hiloEPoll;

	pthread_t thrPlanificador;
	pthread_create(&thrPlanificador, NULL, detectarInterbloqueo, NULL);
	log_info(logger, "Levanto hilo para interbloqueo");

	pthread_create(&hiloEPoll,NULL,lanzarEPoll,NULL);

	while(1){
		nivel_gui_dibujar(ListaItems, nivel->nombre);
	}


	return 0;
}

void* lanzarEPoll (){

	while (1){
		epoll_escucharGeneral(nivel->fdEpoll, -1, NULL, accionMeMandaronAlgo, accionSeDesconecto);
	}

	return NULL;
}



int cantidad_de_algo(char* conf, char* algo)
{
	FILE *file;
	char line[BUFSIZ];
	int linen = 1;
	int cantidad = 0;
	file = fopen(conf, "r");
	while(fgets(line, sizeof(line), file)){
		if(strstr(line, algo) != NULL) {
			++cantidad;
		}
		++linen;
	}
	fclose(file);
	return cantidad;
}


/* Nombre: crearCajasDisponibles/1
 * Argumentos:
 *               - config (t_config *) (Puntero a una config) y  cantidadCajas
 *
 * Devuelve:
 *               t_list * (es un puntero a una lista creada por la funcion)
 *
 * Funcion: Crea una nueva lista con todas las cajas que hay en el nivel.
 */
t_list * crearCajasDisponibles(t_config *config, unsigned int cantidadCajas){
	int cantCajasPorNivel = cantidadCajas;
	char* stringCaracteristicas;
	char** arrayCaracteristicas;
	char* coma=",";
	int i;
	t_list *cajasDisponibles = list_create();
	t_caja *caja;

	for(i=1;i<=cantCajasPorNivel;i++) {
		char* numeroCaja = string_from_format("Caja%i", i);
		stringCaracteristicas = config_get_string_value(config, numeroCaja);
		arrayCaracteristicas = string_split(stringCaracteristicas, coma);

		caja = malloc(sizeof(t_caja));
		caja->nombre = string_duplicate(arrayCaracteristicas[0]);
		caja->simbolo = arrayCaracteristicas[1][0];
		caja->instanciasTotales = atoi(arrayCaracteristicas[2]);
		caja->instanciasActuales = atoi(arrayCaracteristicas[2]);
		caja->posX = atoi(arrayCaracteristicas[3]);
		caja->posY = atoi(arrayCaracteristicas[4]);
		list_add_in_index(cajasDisponibles, list_size(cajasDisponibles), caja);

	}

	string_iterate_lines(arrayCaracteristicas, (void*) free);       //FORMA DE LIBERAR CADA STRING INTERNO DEL ARRAY QUE CREE ANTES
	free(arrayCaracteristicas);
	return cajasDisponibles;
}

/* Nombre: crearNivel/1
 * Argumentos:
 *               - ruta(char *) (Puntero a la ruta de un config)
 * Devuelve:
 *               t_nivel * (es un puntero a un nivel creado)
 */

t_nivel* crearNivel(char *ruta){

	t_config *config = config_create(ruta);
	t_nivel *newNivel = malloc(sizeof(t_nivel));
	unsigned int cantidadCajas = cantidad_de_algo(ruta, "Caja"); // Tener en cuenta que no puede estar el comentario de la caja en el archivo de config, ya que contaria como otra instancia.


	newNivel->nombre = string_duplicate(config_get_string_value(config, "Nombre"));
	newNivel->cajasDisponibles = crearCajasDisponibles(config, cantidadCajas);
	newNivel->orquestador = string_duplicate(config_get_string_value(config, "Plataforma"));
	newNivel->fdOrquestador = 0;
	newNivel->enemigos = config_get_int_value(config, "Enemigos");
	newNivel->sleepEnemigos  = config_get_int_value(config, "Sleep_Enemigos");
	newNivel->fdNivelServer = 0;
	newNivel->fdEpoll = epoll_crear();
	newNivel->tiempoDeadLock = config_get_int_value(config, "TiempoChequeoDeadlock");
	newNivel->recovery = config_get_int_value(config, "Recovery");
	newNivel->algoritmo = string_duplicate(config_get_string_value(config, "Algoritmo"));
	newNivel->quantum = config_get_int_value(config, "Quantum");
	newNivel->retardo = config_get_int_value(config, "Retardo");
	newNivel->fdInotify = inotify_init();
	if (newNivel->fdInotify < 0) {
		log_error(logger, "Error en el inotify_init");
	}
	newNivel->inotifyWatch = inotify_add_watch(newNivel->fdInotify, NIVEL_RUTASARCHIVO, IN_MODIFY);
	newNivel->rutaConfig = string_duplicate(ruta);

	config_destroy(config);
	return newNivel;
}


void agregarListaItemsCajas(){
	int cantElementos = nivel->cajasDisponibles->elements_count;
	int i;

	for (i=0;i<cantElementos;i++) {
		t_caja *caja = list_get(nivel->cajasDisponibles, i);
		CrearCaja(ListaItems, caja->simbolo, caja->posX, caja->posY,caja->instanciasActuales);
	}

}

int posicionRandomEnemigoX(int* posicion){
	nivel_gui_get_area_nivel(&max_filas, &max_cols);
	int max=max_cols;
	int min=1;
	*posicion = rand()%(max-min)+min;
	if (*posicion != 0) {
		return *posicion;
	}else {
		return 1;
	}

}

int posicionRandomEnemigoY(int* posicion){
	nivel_gui_get_area_nivel(&max_filas, &max_cols);
	int max=max_filas;
	int min=1;
	*posicion = rand()%(max-min)+min;
	if (*posicion != 0) {
		return *posicion;
	}else {
		return 1;
	}

}


void agregarListaEnemigos (){
	int cantidad = nivel->enemigos;
	int i = 0;
	pthread_t hilosEnemigos[cantidad];
	t_mover_enemigo *enemigo;
	for(i=0; i<cantidad; i++) {
		enemigo = malloc(sizeof(t_mover_enemigo));
		int enemigoX,enemigoY;
		enemigo->posicion_x = posicionRandomEnemigoX(&enemigoX);
		enemigo->posicion_y = posicionRandomEnemigoY(&enemigoY);
		enemigo->simbolo = (char)(((int)'0')+i);
		CrearEnemigo(ListaItems,enemigo->simbolo,enemigo->posicion_x,enemigo->posicion_y);
		pthread_create(&hilosEnemigos[i],NULL,algoritmoMueveEnemigos,(void*)enemigo);
	}

}
int hayCajaEnPosicion(int x, int y){
	int i=0, loEncontre=0;
	int cantidadCajas = list_size(nivel->cajasDisponibles);
	while (i<cantidadCajas)
	{
		t_caja *caja = list_get(nivel->cajasDisponibles,i);
		if(caja->posX==x && caja->posY==y){
			loEncontre = 1;

		}
		i++;
	}
	if(loEncontre == 0){
		return 0;
	} else { return 1;}

}

ITEM_NIVEL * getCaja(char simboloCaja){
	_Bool esSimboloCajaIgual(void * element){
		ITEM_NIVEL* cajaActual = (ITEM_NIVEL*) element;
		return ( cajaActual->id == simboloCaja);
	}

	return list_find(ListaItems, esSimboloCajaIgual);
}

t_personaje* getPersonajePorSimbolo(char simboloPersonaje) {
	_Bool esSimboloPersonajeIgual(void * element){
		t_personaje* personajeActual = (t_personaje*) element;
		return (personajeActual->simbolo == simboloPersonaje);
	}

	return list_find(personajesActuales, esSimboloPersonajeIgual);

}


void moverEnemigosEnL(t_mover_enemigo * enemigo){

	coords posiblesMovimientos[] = {{2, 1}, {2, -1},{-2, 1}, {-2, -1},{1, 2}, {1, -2},{-1, 2}, {-1, -2}};




	int max_x = 8;
	int min_x = 0;
	int res = rand()%(max_x-min_x)+min_x;
	//      int meMoviSoloUnoEnX = 0;
	//      int meMoviSoloUnoEnY = 0;
	int x = posiblesMovimientos[res].x;
	int y = posiblesMovimientos[res].y;

	if (x>0){
		if ( (enemigo->posicion_x + x) > max_cols) {
			x = -x;
		}
	}

	if (x<0){
		if ( (enemigo->posicion_x + x) < 1) {
			x = -x;
		}
	}
	// MOVERSE EN X
	if ( abs(x) == 1 ) {
		if(!hayCajaEnPosicion(enemigo->posicion_x + x,enemigo->posicion_y)) {
			MoverEnemigo(ListaItems,enemigo->simbolo, enemigo->posicion_x + x, enemigo->posicion_y);
			usleep(nivel->sleepEnemigos);
		}
	} else {
		if(!hayCajaEnPosicion(enemigo->posicion_x + x/2,enemigo->posicion_y)) {
			MoverEnemigo(ListaItems,enemigo->simbolo, enemigo->posicion_x + x/2, enemigo->posicion_y);
			usleep(nivel->sleepEnemigos);
		}       //else { meMoviSoloUnoEnX = 1; }
		if(!hayCajaEnPosicion(enemigo->posicion_x + x,enemigo->posicion_y) & !hayCajaEnPosicion(enemigo->posicion_x + x/2,enemigo->posicion_y) ) {
			MoverEnemigo(ListaItems,enemigo->simbolo, enemigo->posicion_x + x, enemigo->posicion_y);
			usleep(nivel->sleepEnemigos);
		}
	}

	enemigo->posicion_x = enemigo->posicion_x + x;

	// MOVERSE EN Y

	if (y>0){
		if ( (enemigo->posicion_y + y) > max_filas) {
			y = -y;
		}
	}

	if (y<0){
		if ( (enemigo->posicion_y + y) < 1) {
			y = -y;
		}
	}

	if ( abs(y) == 1 ) {
		if (!hayCajaEnPosicion(enemigo->posicion_x,enemigo->posicion_y + y)){
			MoverEnemigo(ListaItems,enemigo->simbolo, enemigo->posicion_x, enemigo->posicion_y + y);
			usleep(nivel->sleepEnemigos);
		}


	} else {
		if(!hayCajaEnPosicion(enemigo->posicion_x,enemigo->posicion_y + y/2)) {
			MoverEnemigo(ListaItems,enemigo->simbolo, enemigo->posicion_x, enemigo->posicion_y + y/2);
			usleep(nivel->sleepEnemigos);
		}
		if(!hayCajaEnPosicion(enemigo->posicion_x,enemigo->posicion_y + y) & !hayCajaEnPosicion(enemigo->posicion_x,enemigo->posicion_y + y/2) ) {
			MoverEnemigo(ListaItems,enemigo->simbolo, enemigo->posicion_x, enemigo->posicion_y + y);
			usleep(nivel->sleepEnemigos);
		}
	}


	enemigo->posicion_y = enemigo->posicion_y + y;



}

t_personaje * getPersonajeMasCercano(t_mover_enemigo * enemigo){

	int i;
	int min = 99999;
	int distanciaTotal;
	t_personaje * compararPersonaje;
	t_personaje * personajeMasCercano;
	for (i=0; i<list_size(personajesActuales); i++){

		compararPersonaje = list_get(personajesActuales,i);
		distanciaTotal = (compararPersonaje->posX - enemigo->posicion_x) + (compararPersonaje->posY - enemigo->posicion_y);
		if ( (abs(distanciaTotal)) < min){
			personajeMasCercano = compararPersonaje;
			min = distanciaTotal;
		}

	}
	return personajeMasCercano;
}

t_mover_enemigo * autoMoverse(int objetivoX, int objetivoY, t_mover_enemigo * enemigo){

        if( ( (enemigo->ultimoMovimiento == VERTICAL) && (enemigo->posicion_x != objetivoX)) || (enemigo->posicion_y    == objetivoY)) {

                if (enemigo->posicion_x < objetivoX) {
                        (enemigo->posicion_x)++;
                        enemigo->ultimoMovimiento = HORIZONTAL;
                        if(hayCajaEnPosicion(enemigo->posicion_x,enemigo->posicion_y)){

                        }
                }
                else {
                        (enemigo->posicion_x)--;
                        enemigo->ultimoMovimiento = HORIZONTAL;
                }
        }
        else {
                if ( enemigo->posicion_y != objetivoY ) {
                        if (enemigo->posicion_y < objetivoY) {
                                (enemigo->posicion_y)++;
                                enemigo->ultimoMovimiento=VERTICAL;
                        }
                        else {
                                (enemigo->posicion_y)--;
                                enemigo->ultimoMovimiento=VERTICAL;
                        }
                }

        }

        return enemigo;

}

void matarPersonaje (char simbolo){

	t_personaje * personaje = getPersonajePorSimbolo(simbolo);
	int j=0;
	for(j=0; j<personaje->recursos->elements_count; j++){


		t_struct_pedirRecurso * caja = list_get(personaje->recursos,j);
		ITEM_NIVEL * cajaConseguida = getCaja(caja->letra);
		cajaConseguida->quantity++;
	}
	BorrarItem(ListaItems, simbolo);
	BorrarPersonajeActual(personajesActuales, simbolo);

}


void perseguirPersonajes(t_mover_enemigo * enemigo){

	t_personaje * personajeASeguir = getPersonajeMasCercano(enemigo);
	//log_info(logger, "El personaje que voy a seguir es %c, y esta en la posicion (%d,%d)", personajeASeguir->simbolo, personajeASeguir->posX, personajeASeguir->posY);
	autoMoverse(personajeASeguir->posX,personajeASeguir->posY,enemigo);
	if (hayCajaEnPosicion(enemigo->posicion_x, enemigo->posicion_y)){

	}
	MoverEnemigo(ListaItems,enemigo->simbolo, enemigo->posicion_x, enemigo->posicion_y);

	if( enemigo->posicion_x == personajeASeguir->posX && enemigo->posicion_y == personajeASeguir->posY && !hayCajaEnPosicion(personajeASeguir->posX,personajeASeguir->posY)){
		pthread_mutex_lock(&enemigosMatando);

		log_info(logger, "VINE A MATAR AL PERSONAJE JOJOJO");
		t_struct_simbolo_dibujame * personajeAMatar = malloc(sizeof(t_struct_simbolo_dibujame));
		personajeAMatar->letra = personajeASeguir->simbolo;



		int seEnvio = socket_enviar(nivel->fdOrquestador, D_STRUCT_SIMBOLO_DIBUJAME, personajeAMatar); // La señal no tiene nada que ver, pero se reutiliza.
		log_info(logger, "Le pido al planificador que mate al personaje %c", personajeASeguir->simbolo);
		if(!seEnvio){
			log_error(logger, "No le pude mandar al planificador que mate al personaje %c", personajeASeguir->simbolo);
		} else {


			int j=0;
			for(j=0; j<personajeASeguir->recursos->elements_count; j++){


				t_struct_pedirRecurso * caja = list_get(personajeASeguir->recursos,j);
				ITEM_NIVEL * cajaConseguida = getCaja(caja->letra);
				cajaConseguida->quantity++;

			}


			// Lo borro de la lista de ListaItems para que no se grafique mas

			BorrarItem(ListaItems, personajeASeguir->simbolo);

			BorrarPersonajeActual(personajesActuales, personajeASeguir->simbolo);

		}
		pthread_mutex_unlock(&enemigosMatando);

	}
	log_info(logger, "Me quedo atrapado aca? 1");
	usleep(nivel->sleepEnemigos);

}


void* algoritmoMueveEnemigos(void* arg){

	t_mover_enemigo *enemigo = (t_mover_enemigo *) arg;

	while(1){

		if ((personajesActuales->elements_count)==0)
			moverEnemigosEnL(enemigo);
		else perseguirPersonajes(enemigo);
	}
	return NULL;

}

char * stringDeRecursos(void){
	t_link_element* caja = nivel->cajasDisponibles->head;
	char * string = string_duplicate("");
	char * charStringeado;
	while(caja != NULL){
		charStringeado = string_from_format("%c", ((t_caja*) caja->data)->simbolo);
		string_append(&string, charStringeado);
		free(charStringeado);
		caja = caja->next;
	}

	return string;
}

/*
 * Nombre: destruirCaja/1
 * Argumentos:
 *              - caja a destruir
 *
 * Devuelve: void
 *
 * Funcion: te hace verga la caja
 */
void destruirCaja(t_caja *self){

	log_trace(logger, "nivel-> destruyo caja %s", self->nombre);
	free(self->nombre);
	free(self);
}

/*
 * Nombre: destruirNivel/1
 * Argumentos:
 *              - self (t_nivel *) (Puntero a un nivel)
 *
 * Devuelve: void
 *
 * Funcion: te hace verga el nivel
 */
void destruirNivel(t_nivel *self){
	free(self->nombre);
	free(self->orquestador);
	list_destroy_and_destroy_elements(self->cajasDisponibles, (void*) destruirCaja);
	log_trace(logger, "Nivel-> Me destruyo a mi mismo", self->nombre);
	free(self);
}

void rutinaDeCerrarTodo(void) {
	log_info(logger, "Empiezo a autodestruirme");
	nivel_gui_terminar();
	destruirNivel(nivel);
	log_error(logger,"Me autodestruyo (No es un error!)");
	log_destroy(logger);

	exit(1);
}

void conectarOrquestador(void){
	int fdOrquestador = socket_crearYConectarCliente(socket_ip(nivel->orquestador), socket_puerto(nivel->orquestador));
	if (epoll_agregarSocketCliente(nivel->fdEpoll, fdOrquestador)== -1){
		log_error(logger, "Error al agregar plataforma al epoll");
		return ;
	}
	t_struct_datosNivel * datosNivel = malloc(sizeof(t_struct_datosNivel));
	datosNivel->nombre = nivel->nombre;
	datosNivel->recursos = stringDeRecursos(); //EJ: "HMF" es el string con los 3 recursos del nivel
	datosNivel->algoritmo = nivel->algoritmo;
	datosNivel->quantum = nivel->quantum;
	datosNivel->retardo = nivel->retardo;

	int seEnvio = socket_enviar(fdOrquestador, D_STRUCT_DATOSNIVEL , datosNivel);

	if(!seEnvio) {
		log_error(logger,"No se pudo enviar lo que requiere el orquestador al conectarme\n");
		socket_cerrarConexion(fdOrquestador);
		rutinaDeCerrarTodo();
		return;
	}

	free(datosNivel->recursos);
	free(datosNivel);

	nivel->fdOrquestador = fdOrquestador;
}



void enviarPosicionAPlanificador(int fdEmisor, int x, int y){
	t_struct_posxposy * posxposy = malloc(sizeof(t_struct_posxposy));
	posxposy->posX = x;
	posxposy->posY = y;
	log_info(logger,"Envio la posicion de un recurso");
	int seEnvio = socket_enviar(fdEmisor, D_STRUCT_POSXPOSY , posxposy);

	if(!seEnvio) {
		log_error(logger,"No se pudo enviar la posicion al planificador\n");
	}

	free(posxposy);
}

void buscarPosicionRecurso (int* x, int* y, char simbolo){

	_Bool _esElRecursoQueBusco(ITEM_NIVEL* item) {
		return item->id == simbolo;
	}
	ITEM_NIVEL* item_a_enviar = list_find(ListaItems, (void*) _esElRecursoQueBusco);

	*x = item_a_enviar->posx;
	*y = item_a_enviar->posy;
}

void actualizarAlgoritmo(char *ruta){

	t_config *config = config_create(ruta);

	if(config_has_property(config, "Algoritmo")){
		log_info(logger, "El archivo de configuracion fue modificado!");

		char * nuevoAlgoritmo = string_duplicate(config_get_string_value(config, "Algoritmo"));
		int nuevoQuantum = config_get_int_value(config, "Quantum");
		int nuevoRetardo = config_get_int_value(config, "Retardo");
		log_info(logger, "El nuevo algoritmo es: %s", nuevoAlgoritmo);
		log_info(logger, "El nuevo quantum es: %d", nuevoQuantum);
		log_info(logger, "El nuevo retardo es: %d", nuevoRetardo);

		t_struct_algoritmo* algoritmoActualizado = malloc(sizeof(t_struct_algoritmo));
		algoritmoActualizado->algoritmo = string_duplicate(nuevoAlgoritmo);
		algoritmoActualizado->quantum = nuevoQuantum;
		algoritmoActualizado->retardo = nuevoRetardo;
		int seEnvio = socket_enviar(nivel->fdOrquestador, D_STRUCT_ACTUALIZARALGORITMO , algoritmoActualizado);

		if(!seEnvio) {
			printf("no se pudo enviar");
			log_error(logger,"No se pudo enviar el nuevo algoritmo al planificador\n");
			return;
		}

		config_destroy(config);
		free(algoritmoActualizado);

	}

}


void manejarInotify(){
	char buffer[BUF_LEN];
	int length;
	int offset;
	struct inotify_event *event;
	length = read(nivel->fdInotify, buffer, BUF_LEN);

	offset = 0;

	if (length < 0) {
		log_error(logger, "Error al leer inotify");
	}

	while (offset < length) {

		event = (struct inotify_event *) &buffer[offset];

		if (event->len) {
			if (event->mask & IN_MODIFY) {

				actualizarAlgoritmo(nivel->rutaConfig);
			}
		}
		offset += sizeof (struct inotify_event) + event->len;
	}

}



void accionSeDesconecto(epoll_data_t epollData){

	log_info(logger, "ENTRO A DESTRUIRME");
	rutinaDeCerrarTodo();




}



void accionMeMandaronAlgo(epoll_data_t epollData){
	log_info(logger, "Me quedo atrapado aca? 2");

	int fdEmisor = epollData.fd;
	log_info(logger,"Me mandaron algo, su fd es %d", fdEmisor);
	if(fdEmisor == nivel->fdInotify)
	{
		//se modificó el archivo config.
		manejarInotify();
	} else {

		t_tipoEstructura tipo;
		void * estructura;
		int recibio = socket_recibir(fdEmisor, &tipo, &estructura);
		if(!recibio){
			log_error(logger,"No se recibio correctamente el mensaje del fd %d \n", fdEmisor);
			return;
		}
		log_info(logger, "TIPO DE SEÑAL: %d", tipo);

		if (tipo == D_STRUCT_SIGNAL) {
			t_signal signal = ((t_struct_signal*) estructura)->signal;
			switch (signal) {
			case S_Orquestador_Nivel_Finalizate:
				rutinaDeCerrarTodo();
				break;
			default:
				log_error(logger,"Me mandaron mal una signal\n");
				return;
			}

		}

		if (tipo == D_STRUCT_PEDIRPOS_RECURSO) {
			int x, y;
			char simbolo = ((t_struct_pedir*) estructura)->recurso;
			char personaje = ((t_struct_pedir*) estructura)->personaje;
			log_info(logger,"Me estan pidiendo la posicion del recurso: %c", personaje);
			buscarPosicionRecurso(&x,&y,simbolo);
			enviarPosicionAPlanificador(fdEmisor,x,y);
		}

		if (tipo == D_STRUCT_SIMBOLO_DIBUJAME) {   //TODO HAY QUE VERIFICAR ACA.

			t_personaje * unPersonaje = malloc(sizeof(t_personaje));
			char simbolo = ((t_struct_simbolo_dibujame*) estructura)->letra;
			log_info(logger, "El personaje %c me pide que empiece a dibujarlo", simbolo);
			if ( (getPersonajePorSimbolo(simbolo)) == 0 ) {
				pthread_mutex_lock(&mDeadLock);
				CrearPersonaje(ListaItems, simbolo , 1, 1);
				unPersonaje->simbolo = simbolo;
				unPersonaje->posX = 1;
				unPersonaje->posY = 1;
				unPersonaje->recursos = list_create();
				unPersonaje->ordenDeLlegada = ordenDeLlegada;
				ordenDeLlegada++;
				list_add(personajesActuales, unPersonaje);
				pthread_mutex_unlock(&mDeadLock);

			}


		}

		if (tipo == D_STRUCT_SIMBOLOPERSONAJE){

			t_struct_simboloPersonaje* personajeABorrar = malloc(sizeof (t_struct_simboloPersonaje));
			personajeABorrar = (t_struct_simboloPersonaje*)estructura;
			log_info(logger, "LLego ACA 1");

            t_personaje * personaje = getPersonajePorSimbolo(personajeABorrar->simbolo);
            if(personaje !=NULL){

			int j=0;
			for(j=0; j<personaje->recursos->elements_count; j++){

				t_struct_pedirRecurso * caja = list_get(personaje->recursos,j);
				ITEM_NIVEL * cajaConseguida = getCaja(caja->letra);
				log_info(logger, "Libero caja %c, por haber terminado el nivel!", cajaConseguida->id);

				cajaConseguida->quantity++;

			}

			// Lo borro de la lista de ListaItems para que no se grafique mas
			BorrarItem(ListaItems, personajeABorrar->simbolo);

			// Lo borro de la lista de personajes para que los enemigos no lo sigan mas
			BorrarPersonajeActual(personajesActuales, personajeABorrar->simbolo);

			log_info(logger, "LLego ACA 3");

            }

		}

		if (tipo == D_STRUCT_POSXPOSY_SIMBOLO) {
			t_struct_posxposy_simbolo* datosPersonaje = malloc(sizeof(t_struct_posxposy_simbolo));
			datosPersonaje = ((t_struct_posxposy_simbolo*) estructura);
			pthread_mutex_lock(&enemigosMatando);

			t_personaje * personajeAEditar = getPersonajePorSimbolo(datosPersonaje->simbolo);
			pthread_mutex_unlock(&enemigosMatando);

			if(personajeAEditar != NULL){
				MoverPersonaje(ListaItems, datosPersonaje->simbolo, datosPersonaje->posX, datosPersonaje->posY);
				log_info(logger, "Movi al personaje %c a la posicion (%d,%d)", datosPersonaje->simbolo, datosPersonaje->posX, datosPersonaje->posY);



				int confirmoQueSeMovio = socket_enviarSignal(fdEmisor, S_Nivel_Personaje_ConfirmaQueSeMovio);
				log_info(logger, "1");
				if(confirmoQueSeMovio){
					log_info(logger, "2");
					log_info(logger, "Se envio la confirmacion de que movi al personaje");
					log_info(logger, "3");
					log_info(logger, "4");
					personajeAEditar->posX = datosPersonaje->posX;
					log_info(logger, "5");
					personajeAEditar->posY = datosPersonaje->posY;
					log_info(logger, "6");
				}

				else{
					log_error(logger, "NO se envio la confirmacion de que movi al personaje");
				}

			}

			else {

				int loMatoEnemigo = socket_enviarSignal(fdEmisor, S_Enemigo_Mato_Personaje);
				if(loMatoEnemigo){

					log_info(logger, "Un enemigo mato a un personaje");

				}


			}



			log_info(logger, "7");
			log_info(logger, "8");
		}

		if (tipo == D_STRUCT_ASIGNAR_RECURSO) {
			t_struct_asignar* personajeYSimbolo = malloc(sizeof(t_struct_asignar));
			personajeYSimbolo = (t_struct_asignar*)estructura;
			log_info(logger, "Me esta pidiendo el personaje %c que asigne un recurso %c",personajeYSimbolo->personaje, personajeYSimbolo->recurso);
			ITEM_NIVEL * caja = getCaja(personajeYSimbolo->recurso);
			if ( (caja->quantity) > 0 ){
				t_personaje * personaje = getPersonajePorSimbolo(personajeYSimbolo->personaje);
				if(personaje!=NULL){
				log_info(logger, "Me esta pidiendo %c que asigne un recurso %c",personaje->simbolo, personajeYSimbolo->recurso);
				t_struct_pedirRecurso * cajaParaAgregar = malloc(sizeof(t_struct_pedirRecurso));
				cajaParaAgregar->letra = caja->id;
				pthread_mutex_lock(&mDeadLock);
				list_add(personaje->recursos, cajaParaAgregar);
				pthread_mutex_unlock(&mDeadLock);
				caja->quantity-- ;
				socket_enviarSignal(fdEmisor, S_Nivel_Personaje_TeAsigneRecurso);
				log_info(logger, "El personaje me pide %c y se lo doy.\n", personajeYSimbolo->recurso);
				}else{
					log_info(logger,"ACAAAAAAAAAA13123123131231231232312");

				}

			}
			else {
				t_personaje * personaje = getPersonajePorSimbolo(personajeYSimbolo->personaje);
				socket_enviarSignal(fdEmisor, S_Nivel_Personaje_NoTeAsigneRecurso);
				log_info(logger, "El personaje me pide %c y NO se lo doy. Va a esperar...\n",personajeYSimbolo->recurso);
				personaje->recursoBuscadoActual = personajeYSimbolo->recurso;
			}


		}

		free(estructura);

	}

}


/* EMPIEZA DEADLOCK */


//DEADLOCK BITCH

/*
 *        Me envia un "personaje" y una "caja"
 *        Devuelve CANTIDAD de recursos de esa "caja" que posea el "personaje"
 */
int cuantosRecursosTienePersonaje(t_personaje* personaje, ITEM_NIVEL* caja){
        int cantidad=0;

        _Bool esMismoSimbolo(void * element){
                t_struct_pedirRecurso* recurso = (t_struct_pedirRecurso*) element;
                //log_trace(logger, "Comparo recurso con item: %c == %c", recurso->letra, caja->id);
                if (recurso->letra == caja->id)
                        cantidad +=1;
                return (recurso->letra == caja->id);
        }

        list_find(personaje->recursos, esMismoSimbolo);
      //  log_trace(logger, "Obtuve recurso, tiene tanta cantidad: %d", cantidad);
        return cantidad;
}

/*
 *        Me envia un "personaje" y una "caja"
 *        Devuelve CANTIDAD de recursos de esa "caja" que ESTE ESPERANDO el "personaje"
 */
int personajeNecesitaRecurso(t_personaje* personaje, ITEM_NIVEL* caja){
        int cantRecursosQueNecesita=0;
        //siempre es 1 la cantidad maxima de recursos que este esperando.
        if (personaje->recursoBuscadoActual == caja->id){
                cantRecursosQueNecesita=1;
        }
        return cantRecursosQueNecesita;
}
int traducirIJ(int i, int j, int cantRecursos){
        return (cantRecursos*i) +j;
}

int* crearMatrizAsignacion(int cantPersonajes, int cantRecursos) {
        int i, j;
        int cantAsignados = 0;
        int* matAsignacion = malloc(cantPersonajes*cantRecursos*sizeof(int));
        t_personaje * personaje;
        ITEM_NIVEL * caja;

      //  log_trace(logger, "Entro mat asignacion (%ix%i)", cantPersonajes, cantRecursos);
        for (i = 0; i < cantPersonajes; i++) {
                for (j = 0; j < cantRecursos; j++) {
                        personaje = list_get(personajesActuales, i);
                        caja = list_get(ListaCajas, j);
                  //      log_trace(logger, "Voy a los fors i: (%i,%i)->%c : %c", i, j, personaje->simbolo, caja->id);
                        cantAsignados = cuantosRecursosTienePersonaje(personaje, caja);
                  //      log_trace(logger, "voy a los fors i: (%i,%i)=%i", i, j, cantAsignados);
                        matAsignacion[traducirIJ(i,j,cantRecursos)] = cantAsignados;
                }
        }
       // log_trace(logger, "Termine mat asignacion");
        return matAsignacion;
}

int* crearVectorDisponibles(int cantRecursos) {
        //CREO VECTOR DISPONIBLES
        int* vecDisponibles= malloc(sizeof(int)*cantRecursos);
        int j;
        int cantRecDisponibles=0;

        for (j = 0; j < cantRecursos; j++) {
                cantRecDisponibles = ((ITEM_NIVEL *)list_get(ListaCajas, j))->quantity;
                vecDisponibles[j] = cantRecDisponibles;
        }
        return vecDisponibles;
}

int* crearVectorTemporal(int cantRecursos, int* vecDisponibles) {
        int j;
        int* vecTemporal= malloc(sizeof(int)*cantRecursos);
        for (j = 0; j < cantRecursos; j++) {
                vecTemporal[j] = vecDisponibles[j];
        }
        return vecTemporal;
}

int* crearMatrizNecesidad(int cantPersonajes, int cantRecursos) {
        int i,j;
        int cantNecesita = 0;
        int* matNecesidad = malloc(cantPersonajes*cantRecursos*sizeof(int));

        for (i = 0; i < cantPersonajes; i++) {
                for (j = 0; j < cantRecursos; j++) {
                        cantNecesita = personajeNecesitaRecurso(list_get(personajesActuales, i),list_get(ListaCajas, j));
                        matNecesidad[traducirIJ(i,j,cantRecursos)] = cantNecesita;
                }
        }
        return matNecesidad;
}

int* crearVectorPersonajesMarcados(int cantPersonajes) {
        int* vecPersonajes= malloc(sizeof(int)*cantPersonajes);
        int i;
        for (i = 0; i < cantPersonajes; i++) {
                vecPersonajes[i] = 0;
        }
        return vecPersonajes;
}

int buscoFilaMenorAVecTemporal(int cantPersonajes, int cantRecursos, int* vecPersonajes, int* matNecesidad,        int* vecTemporal, int* vecDisponibles, int* matAsignacion){

        int i,j,cantFilasMenor,encontroUnaFila;
        encontroUnaFila=0;
        for (i = 0; i < cantPersonajes; i++) {

                //si el personaje no esta marcado, comparo
                if (vecPersonajes[i] == 0) {
                        cantFilasMenor = 0;

                        //comienzo a comparar fila de personaje con vecTemporal
                        for (j = 0; j < cantRecursos; j++) {
                                if (matNecesidad[traducirIJ(i,j,cantRecursos)] <= vecTemporal[j]) {
                                        cantFilasMenor++;
                                }
                        }

                        //si toda la fila es <= vecTemporal
                        if (cantFilasMenor == cantRecursos) {
                                encontroUnaFila = 1;
                                vecPersonajes[i] = 1; //marco el personaje (MODIFICA VECTOR)
                              //  log_info(logger, "MARCO AL PERSONAJE NUMERO %i", i);
                                for (j = 0; j < cantRecursos; j++) {
                                        vecTemporal[j] = vecTemporal[j] + matAsignacion[traducirIJ(i,j,cantRecursos)]; //sumo fila actual a matAsignacion (MODIFICA MATRIZ)
                                }
                                imprimirVector(vecTemporal, cantRecursos, "Disponible Resultante");
                                i = cantPersonajes; //hago que termine el FOR
                        }
                }
        }
        return encontroUnaFila;
        //TODO fijarse el tema de que actualize valores de "vecDisponibles" y "vecPersonajes"
}

void * detectarInterbloqueo(void * loQueSea){

        _Bool esCaja (void * element){
                ITEM_NIVEL* cajaActual = (ITEM_NIVEL*) element;
                return ( cajaActual->item_type == RECURSO_ITEM_TYPE);
        }

        ListaCajas = list_create();
        ListaCajas = list_filter(ListaItems, (void *)esCaja);
       // log_info(logger, "Creo la lista filtrada con solo cajas");
        int* matAsignacion, *vecDisponibles, *vecTemporal, *matNecesidad, *vecPersonajes;
        int cantPersonajes, cantRecursos;
        int cantCeros,i,j;
        int encontroUnaFila;
        char* personajesInterbloqueados, *stringPersonaje;
        int boolInterbloqueado;

        while(1){
                usleep(nivel->tiempoDeadLock);


                _Bool _esCaja (void * element){
                        ITEM_NIVEL* cajaActual = (ITEM_NIVEL*) element;
                        return ( cajaActual->item_type == RECURSO_ITEM_TYPE);
                }

                ListaCajas = list_filter(ListaItems, (void *)_esCaja);

                pthread_mutex_lock(&mDeadLock);
                cantPersonajes = personajesActuales->elements_count;
                cantRecursos = ListaCajas->elements_count;

                if(cantPersonajes > 1) {

                        matAsignacion = crearMatrizAsignacion(cantPersonajes, cantRecursos);
                        vecDisponibles = crearVectorDisponibles(cantRecursos);
                        vecTemporal = crearVectorTemporal(cantRecursos, vecDisponibles);
                        matNecesidad = crearMatrizNecesidad(cantPersonajes, cantRecursos);
                        vecPersonajes = crearVectorPersonajesMarcados(cantPersonajes);

                        //1) marco (vecPersonajes=1) cada personaje cuya fila en matAsignacion sea todos 0

                        imprimirVector(vecTemporal, cantRecursos, "Disponible Inicial");

                        for (i=0;i<cantPersonajes;i++){
                                cantCeros=0;
                                for(j=0; j<cantRecursos;j++){
                                        if (matAsignacion[traducirIJ(i,j,cantRecursos)] == 0){
                                                cantCeros++;
                                        }
                                }
                                if (cantCeros==cantRecursos){
                                      //  log_trace(logger, "MARCO AL PERSONAJE NUMERO %i", i);
                                        vecPersonajes[i]=1;
                                }
                        }


                        //2)busco fila en matNecesidad, de los personajes NO marcados, que sea <= vecTemporal
                        do {
                                encontroUnaFila = buscoFilaMenorAVecTemporal(cantPersonajes, cantRecursos, vecPersonajes, matNecesidad, vecTemporal, vecDisponibles, matAsignacion);
                        } while (encontroUnaFila==1);

                        boolInterbloqueado=0;
                        personajesInterbloqueados = string_new();
                        for (i = 0; i < cantPersonajes; i++) {
                                if (vecPersonajes[i] == 0){
                                        boolInterbloqueado = 1;
                                        stringPersonaje = string_from_format("%c", ((t_personaje*)list_get(personajesActuales,i))->simbolo );
                                        string_append(&personajesInterbloqueados, stringPersonaje);
                                        free(stringPersonaje);
                                }
                        }

                        if(boolInterbloqueado) {
                                log_info(logger,"TERMINO DEADLOCK. LOS PERSONAJES EN CONFLICTO SON:%s \n", personajesInterbloqueados);
                                if(nivel->recovery) {
                                        mandarAOrquestadorPersonajesInterbloqueados(personajesInterbloqueados);

                                        log_info(logger, "------------------------ LOS PERSONAJES INTERBLOQUEADOS SON LOS SIGUIENTES: %s -----------------------------", personajesInterbloqueados);

                                }
                        }

                        free(personajesInterbloqueados);
                        free(matAsignacion);
                        free(vecDisponibles);
                        free(vecTemporal);
                        free(matNecesidad);
                        free(vecPersonajes);

                }
                pthread_mutex_unlock(&mDeadLock);
        }
        return NULL;
}


void mandarAOrquestadorPersonajesInterbloqueados(char * personajesInterbloqueados){


	int i = 0;
	int min = 9999;
	char simboloAMatar;
	log_info(logger, "Esto");
	log_info(logger, "Esto probablemente no ande! Personaje %c", personajesInterbloqueados[0]);


	for(i=0; i < strlen(personajesInterbloqueados); i++){

		// lo busco en actuales
		log_info(logger,"Deadlock: Empiezo a buscar al personaje %c en bloqueados", personajesInterbloqueados[i]);
		t_personaje * personaje = getPersonajePorSimbolo(personajesInterbloqueados[i]);
		if (personaje == NULL) {
			log_info(logger,"Deadlock: No lo encontre personaje %c en actuales", personajesInterbloqueados[i]);
		}

		else {

			log_info(logger,"El orden de lleagada es %d,Minimo es %d",personaje->ordenDeLlegada,min);
			if (personaje->ordenDeLlegada < min)
			{
				simboloAMatar = personaje->simbolo;
				min = personaje->ordenDeLlegada;
			}
			log_info(logger,"llego hasta aca 3");
		}



	}


	t_struct_char * pInter = malloc(sizeof(t_struct_char));
	pInter->letra = simboloAMatar;



	log_info(logger, "A VER SI ANDA ESTO DEL CASTEO, IMPRIMO EL PERSONAJE A MATAR %c", pInter->letra);

	int seEnvio = socket_enviar(nivel->fdOrquestador, D_STRUCT_CHAR , pInter);


	free(pInter);

	if(!seEnvio) {
		log_error(logger,"No se pudo enviar lo que requiere el orquestador al conectarme\n");
		mandarAOrquestadorPersonajesInterbloqueados(personajesInterbloqueados);
		return;
	}
}

void imprimirVector(int* vec, int cant, char* desc) {
        int i;
        char * str = string_from_format("Vector %s: (", desc), *strAux;
        for (i=0; cant > i  ;i++){
                strAux = string_from_format("%d,", vec[i]);
                string_append(&str, strAux);
                free(strAux);
        }
        string_append(&str, ")");
      //  log_trace(logger, str);
        free(str);
}

void BorrarPersonajeActual(t_list* personajes, char simbolo) {
        bool _search_by_id(t_personaje* personaje) {
                return personaje->simbolo == simbolo;
        }

        list_remove_by_condition(personajes, (void*) _search_by_id);
}

