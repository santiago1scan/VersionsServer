/*
 * @file
 * @brief Sistema de Control de Versiones
 * @author Erwin Meza Vega <emezav@gmail.com>
 * @author Miguel Calambas
 * @author Santiago Escandon
 * Sistema de Control de Versiones
 * Uso:
 *      versions add ARCHIVO "Comentario" : Adiciona una version del archivo al repositorio
 *      versions list ARCHIVO             : Lista las versiones del archivo existentes
 *      versions list                     : Lista todos los archivos almacenados en el repositorio
 *      versions get NUMVER ARCHIVO       : Obtiene una version del archivo del repositorio
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <signal.h>
#include <pthread.h>

#include "./server/versions_server.h"

#define MAX_USERS 100
/**
* @brief Imprime la ayuda
*/
void usage();

/**
 * @brief Terminate the server ending all the runing process and closing comunications
 * @param sig number of the signal sended
 */
void handle_terminate(int sig);

/**
 * @brief Structure to save the actives users
 */
struct Server{
	int countUsers;
	int *socketsUsers;
};

struct Server *myServer = NULL; /* Global variable to manage multiples users*/
int serverSocket; /* Server socket*/
pthread_mutex_t mutexServer; /* Mutex for sync myServer variable*/

int main(int argc, char *argv[]) {
	//Config the handlers of signals
    signal(SIGINT, handle_terminate);
    signal(SIGTERM, handle_terminate);
	
	struct stat s;

	//Crear el directorio ".versions/" si no existe
	#ifdef __linux__
		mkdir(VERSIONS_DIR, 0755);
	#elif _WIN32
		mkdir(VERSIONS_DIR);
	#endif

	// Crea el archivo .versions/versions.db si no existe
	if (stat(VERSIONS_DB_PATH, &s) != 0) {
		creat(VERSIONS_DB_PATH, 0755);
	}
	// Validar argumentos de linea de comandos
	if(argc != 2){
		usage();
		exit(EXIT_FAILURE);
	}
	int PORT = atoi(argv[1]);
	if(PORT <= 0){
		printf("Invalid port, it mus be numeric\n");
		exit(EXIT_FAILURE);
	}

	//Empezamos a inicializar el servidor
	printf("> Starting server\n");

	//Initializate myServer structure
	myServer = malloc(sizeof(struct Server));
	myServer->socketsUsers = calloc(MAX_USERS, sizeof(int));
	myServer->countUsers = 0;

	//Initializate the mutex
	pthread_mutex_init(&mutexServer,NULL);

	//Obtain the server socket
	serverSocket = socket(AF_INET, SOCK_STREAM,0);

	//Initializate bind config
	struct sockaddr_in addr;    
    memset(&addr, 0, sizeof(struct sockaddr_in)); //rellenamos la estructura de ceros
    addr.sin_family = AF_INET; //Config of socket
    addr.sin_port = htons(PORT); //Config port
    addr.sin_addr.s_addr = INADDR_ANY; //0.0.0.0

	if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
        perror("Sorry, we cant run the bind");
        exit(EXIT_FAILURE);
    }  

	if (listen(serverSocket, MAX_USERS) == -1) {
        perror("Error tring listen");
        exit(EXIT_FAILURE);
    }

	printf("> Server listening on port:%d\n", PORT);
	//TODO logica para conectar a un usuario y asignarle un hilo
	while(1);
	exit(EXIT_SUCCESS);

}

void usage() {
	printf("Uso: \n");
	printf("rversionsd PORT:   Escucha por conexiones del cliente en el puerto especificado.\n");
}

void handle_terminate(int sig){
	printf("--Ending the Server--\n");
	
	//Liberamos memoria de myServer de manera segura
	pthread_mutex_lock(&mutexServer);
	
	size_t j = 0;
	for(int i = 0; j<myServer->countUsers;i++){
		if(myServer->socketsUsers[i]>0){
			close(myServer->socketsUsers[i]);
			j++;
		}
	}

	free(myServer->socketsUsers);
	free(myServer);
	pthread_mutex_unlock(&mutexServer);
	pthread_mutex_destroy(&mutexServer);
	
	exit(EXIT_SUCCESS);
}

