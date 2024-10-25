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

#include <arpa/inet.h>
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
 * @brief infinite loop to receive new users in the server
 */
void loop_listening();

/**
 * @brief Handle user thread to listen the querys of user
 * @param args its a *void
 */
void *handler_user_thread(void *args);

/**
 * @brief delete the user with the socket given
 * @param socket socket of the user to delete
 */
void delete_user(int socket);

/**
 * @brief Handle the add request of the user
 * @param socket socket of the user
 * @param idUser id of the user
 */
void handle_add(int socket, int idUser);

/**
 * @brief Handle the get request of the user
 * @param socket socket of the user
 * @param idUser id of the user
 */
void handle_get(int socket, int idUser);

/**
 * @brief Handle the list request of the user
 * @param socket socket of the user
 * @param idUser id of the user
 */
void handle_list(int socket, int idUser);

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
	system("clear");
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
	
	loop_listening();
	
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

void loop_listening(){
	//TODO logica para conectar a un usuario y asignarle un hilo
	while(1){
		//Donde vamos a guardar info de la conexion
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		//Bloqueamos esperando conexiond e nuevo usuario
		int new_client_socket = accept(serverSocket, (struct sockaddr *)&client_addr, &client_len);
		if(new_client_socket == -1){
			perror("Error conecting the new user");
			continue;
		}
		//Sacamos la ip del usuario
		char *client_ip = inet_ntoa(client_addr.sin_addr);
		printf("Reciving new user with ip %s\n", client_ip);

		//Creamos el hilo que maneja el usuario
		int *socket_ptr = malloc(sizeof(int));
		*socket_ptr = new_client_socket;
		
		pthread_t thread_id;
		if(pthread_create(&thread_id, NULL, handler_user_thread, (void*)socket_ptr) != 0){
			free(socket_ptr);
			perror("Error creating the thread to the user");
		}
		pthread_detach(thread_id);

		//Agregamos de manera segura el socket del nuevo usuario a nuestra estructura
		pthread_mutex_lock(&mutexServer);
		size_t i = 0;
		while ( myServer->socketsUsers[i]>0)
			i++;
		myServer->socketsUsers[i] = new_client_socket;
		myServer->countUsers++;
		pthread_mutex_unlock(&mutexServer);
		
	}
	
}

void *handler_user_thread(void *args){
	int clientSocket = *(int *)args;

	size_t size_struct = sizeof(struct first_request); 
	struct first_request *request = malloc(size_struct);

	while (1) {
		int result = receive_first_request(clientSocket, request);
		if (result == ERROR_SOCKET || result == CLIENT_DISCONECT) {
			break;
		}

		if (request->request == ADD) 
			handle_add(clientSocket, request->idUser);
		else if (request->request == LIST)
			handle_list(clientSocket, request->idUser);
		else if (request->request == GET)
			handle_get(clientSocket, request->idUser);
		else
			printf("Solicitud desconocida del usuario %d\n", request->idUser);
	}

	printf("> Cliente con id %d se ha desconectado\n", request->idUser);
	close(clientSocket);
	delete_user(clientSocket);
	free(request);
}

void delete_user(int socket){
	pthread_mutex_lock(&mutexServer);
	size_t i = 0;
	while(myServer->socketsUsers[i] != socket)
		i++;
	myServer->socketsUsers[i] = -1;
	myServer->countUsers--;
	pthread_mutex_unlock(&mutexServer);
}

void handle_add(int socket, int idUser){
	printf(" -- El usuario %d ha solicitado un add --\n", idUser);

	switch (add(socket, idUser))
	{
	case VERSION_ERROR:
		printf("> Error adding the new version of the user %d\n", idUser);
		break;
	case VERSION_ALREADY_EXISTS:
		printf("> The version already exists for the user %d\n", idUser);
		break;
	case VERSION_ADDED:
		printf("> The version has been added for the user %d\n", idUser);
		break;
	default:
		break;
	}
}

void handle_get(int socket, int idUser){
	printf("--El usuario %d ha solicitado un get--\n", idUser);
	switch (get(socket, idUser))
	{
	case VERSION_ADDED:
		printf("> The versions has been geted for the user %d\n", idUser);
		break;
	case VERSION_ERROR:
		printf("> Error listing the versions of the user %d\n", idUser);
		break;
	case VERSION_NOT_EXISTS:
		printf("> The version not exists for the user %d\n", idUser);
		break;
	}
}

void handle_list(int socket, int idUser){
	printf("-- El usuario %d ha solicitado un list --\n", idUser);
	switch (list(socket, idUser))
	{
	case VERSION_ADDED:
		printf("> The versions has been listed for the user %d\n", idUser);
		break;
	case VERSION_ERROR:
		printf("> Error geting the versions of the user %d\n", idUser);
		break;
	}
}
