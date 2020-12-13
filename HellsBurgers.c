
#include <stdio.h>      // libreria estandar
#include <stdlib.h>     // para usar exit y funciones de la libreria standard
#include <string.h>
#include <pthread.h>    // para usar threads
#include <semaphore.h>  // para usar semaforos
#include <unistd.h>


#define LIMITE 50
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sal = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t coc = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pan = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t leer = PTHREAD_MUTEX_INITIALIZER;

//creo estructura de semaforos 
struct semaforos {
    sem_t sem_mezclar;
	sem_t sem_salar;
	sem_t sem_armarMedallones;
	sem_t sem_cocinar;
	sem_t sem_armarHamburguesaCocinar;
	sem_t sem_armarHamburguesaPan;
	sem_t sem_armarHamburguesaLechugaYTomate;
};

//creo los pasos con los ingredientes
struct paso {
   char accion [LIMITE];
   char ingredientes[4][LIMITE];
   
};

//creo los parametros de los hilos 
struct parametro {
 int equipo_param;
 struct semaforos semaforos_param;
 struct paso pasos_param[8];
};

//funcion para imprimir las acciones y los ingredientes de la accion
void* escribirArchivo(void *data, char *accionIn) {
	struct parametro *mydata = data;
	FILE * salida;
	salida = fopen ("salida.txt", "a+");
	//calculo la longitud del array de pasos 
	int sizeArray = (int)( sizeof(mydata->pasos_param) / sizeof(mydata->pasos_param[0]));
	//indice para recorrer array de pasos 
	int i;
	for(i = 0; i < sizeArray; i ++){
		//pregunto si la accion del array es igual a la pasada por parametro (si es igual la funcion strcmp devuelve cero)
		if(strcmp(mydata->pasos_param[i].accion, accionIn) == 0){
			fprintf(salida, "\tEquipo %d - accion %s \n " , mydata->equipo_param, mydata->pasos_param[i].accion);
			//calculo la longitud del array de ingredientes
			int sizeArrayIngredientes = (int)( sizeof(mydata->pasos_param[i].ingredientes) / sizeof(mydata->pasos_param[i].ingredientes[0]) );
			//indice para recorrer array de ingredientes
			int h;
			fprintf(salida, "\tEquipo %d -----------ingredientes : ----------\n",mydata->equipo_param); 
				for(h = 0; h < sizeArrayIngredientes; h++) {
					//consulto si la posicion tiene valor porque no se cuantos ingredientes tengo por accion 
					if(strlen(mydata->pasos_param[i].ingredientes[h]) != 0) {
								fprintf(salida, "\tEquipo %d ingrediente  %d : %s \n",mydata->equipo_param,h,mydata->pasos_param[i].ingredientes[h]);
					}
				}
		}
	}
	printf("\n");
}

void* leerReceta (void *data) {

	pthread_mutex_lock(&leer);
	struct parametro *mydata = data;
	FILE *receta;
	char *token;
	int i = 0;
	char line [100];
	//Lectura del archivo receta, la cual contendrá las acciones junto a los ingredientes necesarios.
	receta = fopen ("receta.txt", "r");
	//Si no puede leer el archivo
	if (receta == NULL){
		printf("El archivo no pudo abrirse. \n");
	}

	//Lectura de linea por linea de la receta
	while (fgets(line, 150, receta)) {
		
		int c = 0;
		//Separa la accion de los ingredientes por el signo "|"
		token = strtok(line , "|");
		if (token == NULL){
			printf("No hay token\n");
		}

		//El primer token de cada linea es la accion a realizar, se guarda en la variable correspondiente
		strcpy(mydata->pasos_param[i].accion, token);
		
		//Recorre el resto de elementos de la linea
		while (token != NULL) {
			token = strtok(NULL , "|");
			if (token != NULL){
				//Copia ingredientes en la variable correspondiente
				strcpy(mydata->pasos_param[i].ingredientes[c], token);
				c ++;
			}
		}
		i++;
	}
	pthread_mutex_unlock(&leer);

}

//funcion para tomar de ejemplo
void* cortar(void *data) {
	//creo el nombre de la accion de la funcion 
	char *accion = "cortar";
	//creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;
	//llamo a la funcion imprimir le paso el struct y la accion de la funcion
	pthread_mutex_lock(&mutex);
	escribirArchivo(mydata,accion);
	pthread_mutex_unlock(&mutex);
	//uso sleep para simular que que pasa tiempo
	usleep( 900000 );
	//doy la señal a la siguiente accion (cortar me habilita mezclar)
    sem_post(&mydata->semaforos_param.sem_mezclar);
	
    pthread_exit(NULL);
}

void* mezclar(void *data) {
	
	char *accion = "mezclar";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_mezclar);

	pthread_mutex_lock(&mutex);
	escribirArchivo(mydata,accion);
	pthread_mutex_unlock(&mutex);

	usleep( 900000 );

	sem_post (&mydata->semaforos_param.sem_salar);
	pthread_exit(NULL);
}

void* salar(void *data) {

	
	char *accion = "salar";
	struct parametro *mydata = data;
	pthread_mutex_lock(&sal);
	
	sem_wait(&mydata->semaforos_param.sem_salar);
	pthread_mutex_lock(&mutex);
	escribirArchivo(mydata,accion);
	pthread_mutex_unlock(&mutex);
	
	usleep( 900000 );

	sem_post (&mydata->semaforos_param.sem_armarMedallones);
	pthread_mutex_unlock(&sal);
	pthread_exit(NULL);

}

void* armarMedallones(void *data){

	char *accion = "armar medallones";
	struct parametro *mydata = data;

	sem_wait(&mydata->semaforos_param.sem_armarMedallones);
	pthread_mutex_lock(&mutex);
	escribirArchivo (mydata,accion);
	pthread_mutex_unlock(&mutex);

	usleep( 900000 );

	sem_post(&mydata->semaforos_param.sem_cocinar);
	pthread_exit(NULL);
}

void* cocinar(void *data) {

	char *accion = "cocinar";
	struct parametro *mydata = data;
	// pthread_mutex_lock(&coc);

	sem_wait(&mydata->semaforos_param.sem_cocinar);

	pthread_mutex_lock(&mutex);
	escribirArchivo (mydata,accion);
	pthread_mutex_unlock(&mutex);

	usleep ( 900000 );
	// pthread_mutex_unlock(&coc);
	
	sem_post(&mydata->semaforos_param.sem_armarHamburguesaCocinar);
	pthread_exit(NULL);
}

void* cortarLechugaTomate(void *data){

	char *accion = "cortar lechuga y tomate";
	struct parametro *mydata = data;
	pthread_mutex_lock(&mutex);
	escribirArchivo (mydata,accion);
	pthread_mutex_unlock(&mutex);

	usleep ( 900000 );
	sem_post(&mydata->semaforos_param.sem_armarHamburguesaLechugaYTomate);
	pthread_exit(NULL);
}

void* hornearPan(void *data){

	char *accion = "hornear";
	struct parametro *mydata = data;
	pthread_mutex_lock(&pan);
	escribirArchivo (mydata,accion);
	

	usleep ( 900000 );
	sem_post(&mydata->semaforos_param.sem_armarHamburguesaPan);
	pthread_mutex_unlock(&pan);
	pthread_exit(NULL);
}

void* armarSuperHamburguesa(void *data){

	char *accion = "Armar super hamburguesa";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_armarHamburguesaCocinar);
	sem_wait(&mydata->semaforos_param.sem_armarHamburguesaLechugaYTomate);
	sem_wait(&mydata->semaforos_param.sem_armarHamburguesaPan);
	pthread_mutex_lock(&mutex);
	escribirArchivo (mydata,accion);
	pthread_mutex_unlock(&mutex);

	usleep ( 900000 );
	pthread_exit(NULL);

}

void* ejecutarReceta(void *i) {
	
	//variables semaforos
	sem_t sem_mezclar;
	sem_t sem_salar;
	sem_t sem_armarMedallones;
	sem_t sem_cocinar;
	sem_t sem_armarHamburguesaCocinar;
	sem_t sem_armarHamburguesaPan;
	sem_t sem_armarHamburguesaLechugaYTomate;
	
	//variables hilos
	pthread_t p1, p2, p3, p4, p5, p6, p7, p8; 
	
	//numero del equipo (casteo el puntero a un int)
	int p = *((int *) i);

	//reservo memoria para el struct
	struct parametro *pthread_data = malloc(sizeof(struct parametro));

	//seteo los valores al struct
	
	//seteo numero de grupo
	pthread_data->equipo_param = p;

	//seteo semaforos
	pthread_data->semaforos_param.sem_mezclar = sem_mezclar;
	pthread_data->semaforos_param.sem_salar = sem_salar;
	pthread_data->semaforos_param.sem_armarMedallones = sem_armarMedallones;
	pthread_data->semaforos_param.sem_cocinar = sem_cocinar;
	pthread_data->semaforos_param.sem_armarHamburguesaCocinar = sem_armarHamburguesaCocinar;
	pthread_data->semaforos_param.sem_armarHamburguesaPan = sem_armarHamburguesaPan;
	pthread_data->semaforos_param.sem_armarHamburguesaLechugaYTomate = sem_armarHamburguesaLechugaYTomate;

	//seteo las acciones y los ingredientes 
	leerReceta(pthread_data);


	//inicializo los semaforos
	sem_init(&(pthread_data->semaforos_param.sem_mezclar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_salar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_armarMedallones),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_cocinar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_armarHamburguesaCocinar),0, 0);
	sem_init(&(pthread_data->semaforos_param.sem_armarHamburguesaPan),0, 0);
	sem_init(&(pthread_data->semaforos_param.sem_armarHamburguesaLechugaYTomate),0, 0);
	


	//creo los hilos a todos les paso el struct creado (el mismo a todos los hilos) ya que todos comparten los semaforos 
    pthread_create(&p1,                           //identificador unico
                            NULL,                          //atributos del thread
                                cortar,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
	pthread_create(&p2, 
						NULL,
							mezclar,
							pthread_data );

	pthread_create(&p3,
						NULL,
							salar,
							pthread_data);

	pthread_create(&p4,
						NULL,
							armarMedallones,
							pthread_data);
	
	pthread_create(&p5,
						NULL,
							cocinar,
							pthread_data);

	pthread_create(&p6,
						NULL,
							cortarLechugaTomate,
							pthread_data);

	pthread_create(&p7,
						NULL,
							hornearPan,
							pthread_data);

	pthread_create(&p8,
						NULL,
							armarSuperHamburguesa,
							pthread_data);
	

	//join de todos los hilos
	pthread_join (p1, NULL);
	pthread_join (p2, NULL);
	pthread_join (p3, NULL);
	pthread_join (p4, NULL);
	pthread_join (p5, NULL);
	pthread_join (p6, NULL);
	pthread_join (p7, NULL);
	pthread_join (p8, NULL);

	 
	//destruccion de los semaforos 
	sem_destroy(&sem_mezclar);
	sem_destroy(&sem_salar);
	sem_destroy(&sem_armarMedallones);
	sem_destroy(&sem_cocinar);
	sem_destroy(&sem_armarHamburguesaCocinar);
	sem_destroy(&sem_armarHamburguesaPan);
	sem_destroy(&sem_armarHamburguesaLechugaYTomate);
	
	//salida del hilo
	 pthread_exit(NULL);
}


int main ()
{
	//creo los nombres de los equipos 
	int rc;
	int *equipoNombre1 =malloc(sizeof(*equipoNombre1));
	int *equipoNombre2 =malloc(sizeof(*equipoNombre2));
	int *equipoNombre3 =malloc(sizeof(*equipoNombre3));
	*equipoNombre1 = 1;
	*equipoNombre2 = 2;
	*equipoNombre3 = 3;

	//creo las variables los hilos de los equipos
	pthread_t equipo1; 
	pthread_t equipo2;
	pthread_t equipo3;
 
	//inicializo los hilos de los equipos
    rc = pthread_create(&equipo1,                           //identificador unico
                            NULL,                          //atributos del thread
                                ejecutarReceta,             //funcion a ejecutar
                                equipoNombre1); 

    rc = pthread_create(&equipo2,                           //identificador unico
                            NULL,                          //atributos del thread
                                ejecutarReceta,             //funcion a ejecutar
                                equipoNombre2);

    rc = pthread_create(&equipo3,                           //identificador unico
                            NULL,                          //atributos del thread
                                ejecutarReceta,             //funcion a ejecutar
                                equipoNombre3);

   if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
     } 

	//join de todos los hilos
	pthread_join (equipo1,NULL);
	pthread_join (equipo2,NULL);
	pthread_join (equipo3,NULL);


    pthread_exit(NULL);
	exit(-1);
}


//Para compilar:   gcc HellsBurgers.c -o ejecutable -lpthread
//Para ejecutar:   ./ejecutable
