/*
 * @file
 * @brief Sistema de Control de Versiones
 * @author Erwin Meza Vega <emezav@gmail.com>
 * @author Miguel Calambas
 * @author Santiago Escandon
 * Sistema de Control de Versiones
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

#include "./client/versions_client.h"

/**
* @brief Imprime la ayuda
*/
void usage();

/**
 * @brief handle the signals to terminate the app
 * @param sig int of signal
 */
void handle_terminate(int sig);

/**
*@brief do the action to add 

 */
 status_operation_socket actionAdd();
 /**
*@brief do the action to add 

 */
 status_operation_socket actionGet();
 /**
*@brief do the action to add 

 */
 status_operation_socket actionList();

/**
 * @brief setup the id client in a file or generate a new one
 * @return int id of the client
 */
int setup_idClient();

int client_socket; /* socket of the conexion with the server */

int main(int argc, char *argv[]) {
	signal(SIGINT, handle_terminate);
	signal(SIGTERM, handle_terminate);
	char cadena[PATH_MAX];
    char *token;
	char order[PATH_MAX], argument2[PATH_MAX], argument3[PATH_MAX];
 
	// Validar argumentos de linea de comandos
	if(argc != 3){
		printf("Invalid argumetns\n");
		usage();
		exit(EXIT_FAILURE);
	}
    //Extraemos y validamos ip y puerto
    int server_port = atoi(argv[2]);
	if(server_port == 0){
		printf("Invalid port\n");
		usage();
		exit(EXIT_FAILURE);
	}
	const char *server_ip = argv[1];

    //Creamos las estructuras para crear la conexion
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    //Convertimos la ip en el formato necesario
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        printf("Direccion ip invalida\n");
		usage();
        exit(EXIT_FAILURE);
    }

	//Creamos el socket
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(client_socket == -1){
		perror("Error al crear el socekt");
		exit(EXIT_FAILURE);
	}

	if(connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error al intentar conectarse con el servidor");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
	
	//system("clear");
    printf("Conectado al servidor %s en el puerto %d\n", server_ip, server_port);

	//Cargamos o generamos el id del cliente
	int idClient = setup_idClient();
	while(1){
		type_request peticionRequest;
		scanf("%s %s %s", order, argument2, argument3);
		if(EQUALS(order, "add")){
			peticionRequest = ADD;
			struct first_request *peticion = malloc(sizeof(struct first_request));
			peticion->request = peticionRequest;
			peticion->idUser = idClient;
			if(write(client_socket, (void*)peticion, sizeof(struct first_request))== -1){
				printf("Falla escritura");
			}
			free(peticion);
				//add(argument2, argument3);
			
			printf("El CLietnte solicita add\n");
		}
		if(EQUALS(order, "list")){
			peticionRequest = LIST;
			struct first_request *peticion = malloc(sizeof(struct first_request));
			peticion->request = peticionRequest;
			peticion->idUser = idClient;
			if(write(client_socket, (void*)peticion, sizeof(struct first_request))== -1){
				printf("Falla escritura");
			}
			free(peticion);
			//list(argument2);
			printf("El CLietnte solicita add\n");
		}
		if(EQUALS(order, "get")){
			peticionRequest = GET;
			struct first_request *peticion = malloc(sizeof(struct first_request));
			peticion->request = peticionRequest;
			peticion->idUser = idClient;
			status_operation_socket restult_first_request = send_first_request(client_socket, peticionRequest);
			if(restult_first_request != OK){
				printf("Error");
			}
			
			
			free(peticion);
			
			//get(argument2, argument3);
			printf("El CLietnte solicita add\n");
		}
		
		
	}
	exit(EXIT_SUCCESS);

}

status_operation_socket actionAdd(){
	return 0;
}

status_operation_socket actionGet(){
	peticionRequest = GET;
	struct first_request *peticion = malloc(sizeof(struct first_request));
	peticion->request = peticionRequest;
	peticion->idUser = idClient;
	status_operation_socket restult_first_request = send_first_request(client_socket, peticionRequest);
	if(restult_first_request != OK){
		printf("Error");
		return ERROR;
	}
	if(get(argument2, argument3)) == 0{
		printf("Error in get");
		return ERROR;
	}	
	return restult_first_request;
}

status_operation_socket actionList(){
	return 0;
}
void usage() {
	printf("Uso: rversions IP PORT Conecta el cliente a un servidor en la IP y puerto especificados.\n");
	printf("Los comandos, una vez que el cliente se ha conectado al servidor, son los siguientes:\n");
	printf("add ARCHIVO \"Comentario\" : Adiciona una version del archivo al repositorio\n");
	printf("list ARCHIVO               : Lista las versiones del archivo existentes\n");
	printf("list                       : Lista todas las versiones de los archivos existentes\n");
	printf("get numver ARCHIVO         : Obtiene una version del archivo del repositorio\n");
}

void handle_terminate(int sig){
	printf("Ending the client process...\n");
	close(client_socket);
	exit(EXIT_SUCCESS);
}

int setup_idClient() {
    char *filename = "idClient";

    // Verificar si el archivo no existe
    if (access(filename, F_OK) == -1) {
		printf("No existe el archivo\n");
        // Inicializar la semilla del generador de números aleatorios
        srand(time(NULL));
        int random_number = rand()%1000000 + 1;

        // Crear y abrir el archivo en modo de escritura
        FILE *file = fopen(filename, "w");
        if (file == NULL) {
            perror("Error al crear el archivo");
            exit(EXIT_FAILURE);
        }

        // Escribir el número aleatorio en el archivo
        fprintf(file, "%d\n", random_number);

        // Cerrar el archivo
        fclose(file);
		return random_number;
    } else {
        // El archivo ya existe, abrirlo en modo de lectura
        FILE *file = fopen(filename, "r");
        if (file == NULL) {
            perror("Error al abrir el archivo");
            exit(EXIT_FAILURE);
        }

        // Leer el número del archivo
        int id;
        fscanf(file, "%d", &id);
        fclose(file);

        return id;
    }
}