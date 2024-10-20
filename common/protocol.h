/**
 * @file
 * @brief Protocolo de comunicacion
 * @author Miguel Angel Calambas Vivas <mangelcvivas@unicauca.edu.co>
 * @author Esteban Santiago Escandon Causaya <estebanescandon@unicauca.edu.co>
 * @copyright MIT License
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 256 /* Default buffer size */
#define COMMENT_SIZE 80 /** < Longitud del comentario */
#define HASH_SIZE 256 /**< Longitud del hash incluyendo NULL*/
#define PATH_MAX 4096 /**< Longitud maxima de una ruta de archivo. */

/**
 * @brief type of request of user
 */
typedef enum {
    LIST,/*!< Request to list versions*/
    ADD, /*!<Request to add a file*/
    GET, /*< Request to get a version of a file*/
}type_request;

/**
 * @brief information of the first request to do some type of request
 */
struct first_request{
    type_request request; /*!< Request to make*/
    int sizeNextMessage; /*!< size of the next message of user (optional)*/
};

/**
 * @brief Struct to send information of the file
 */
struct file_request{
    char pathFile[PATH_MAX]; /*!< path of the file*/
    char hashFile[HASH_SIZE];/*!< Hash of the file (optional)*/
    int version;             /*!< version of the file (optional)*/
};

/**
 * @brief struct to send a file 
 */

struct file_transfer{
    size_t filseSize;           /*!< Size of the file to send*/
    char comment[COMMENT_SIZE]; /*!< Comment of the file (optional)*/
};

/**
 * @brief Start the protocol for send a file
 * @param socket socket to send the file
 * @return 0 en caso de exito, -1 en caso de fallido
 */
int send_file(int socket);

/**
 * @brief Start the protocol for recive a file
 * @param socket socket to recieve a file
 * @return 0 en caso de exito, -1 en caso de fallido
 */
int receive_file(int socket, char **filedata, struct file_request *information);
