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
#include <fcntl.h>

#define BUFFER_SIZE 1024 /* Default buffer size */
#define COMMENT_SIZE 80 /** < Longitud del comentario */
#define HASH_SIZE 256 /**< Longitud del hash incluyendo NULL*/
#define PATH_MAX 4096 /**< Longitud maxima de una ruta de archivo. */
#define SIZE_ELEMENT_LIST sizeof(int) + PATH_MAX + COMMENT_SIZE + HASH_SIZE

/**
 * @brief type of request of user
 */
typedef enum {
    LIST,/*!< Request to list versions*/
    ADD, /*!<Request to add a file*/
    GET, /*< Request to get a version of a file*/
}type_request;

/**
 * @brief status of one message sended or received
 */
typedef enum{
    OK,               /*<! all went fine*/
    ERROR_SOCKET,     /*<! Error with the socket*/
    CLIENT_DISCONECT, /*<! Client has been disconected*/
    INVALID_RESPONSE,  /*<! The response is invalid*/
    ERROR,            /*<! General error*/
}status_operation_socket;

/**
 * @brief information of the first request to do some type of request
 */
struct first_request{
    type_request request; /*!< Request to make*/
    int          idUser;  /*!< id of the user*/
};

/**
 * @brief Struct to send information of the file
 */
struct file_request{
    int  sizeNameFile;        /*!< size of the name*/
    char nameFile[PATH_MAX];  /*!< name of the file*/
    int  sizeHashFile;        /*!< size of the hashfile*/
    char hashFile[HASH_SIZE]; /*!< Hash of the file (optional)*/
    int  version;             /*!< version of the file (optional)*/
};

/**
 * @brief struct to send a information of a file 
 */
struct file_transfer{
    size_t filseSize;             /*!< Size of the file to send*/
    char   comment[COMMENT_SIZE]; /*!< Comment of the file (optional)*/
};

typedef enum {
	VERSION_ERROR,          /*!< Error no especificado */
	VERSION_CREATED,        /*!< Version creada */
	VERSION_ADDED,          /*!< Version agregada */
	VERSION_ALREADY_EXISTS, /*!< Version ya existe */
    VERSION_NOT_EXISTS,     /*!< Versions not exist*/
	FILE_ADDED,             /*<! Archivo adicionado  */
}return_code;

/**
 * @brief Start the protocol for send a file
 * @param socket socket to send the file
 * @param pathFile path of file to send
 * @param sizeFile size of the file to send
 * @return OK,ERROR_SOCKET,CLIENT_DISCONECT,INVALID_RESPONSE,ERROR,   
 */
status_operation_socket send_file(int socket, char * pathFile, int sizeFile);

/**
 * @brief Start the protocol for recive a file
 * @param socket socket to recieve a file
 * @param pathFile path of file to been resived
 * @param sizeFile size of the file to recieve
 * @return OK,ERROR_SOCKET,CLIENT_DISCONECT,INVALID_RESPONSE,ERROR,   
 */
status_operation_socket receive_file(int socket, char *pathFile, int sizeFile);

/**
 * @brief Receive the structure first_request whit a code of status
 * @param socket socket to recieve a file
 * @param first_request_param first request to receive
 * @return  status of operation, posibles returns: OK,ERROR_SCOKET,CLIENT_DISCONECT, INVALID_RESONSE
 */
status_operation_socket receive_first_request(int socket, struct first_request *first_request_param);
/**
 * @brief Receive the structure file_request whit a code of status
 * @param socket socket to recieve a file
 * @param file_request file request to receive
 * @return  status of operation, posibles returns: OK,ERROR_SCOKET,CLIENT_DISCONECT, INVALID_RESONSE
 */
status_operation_socket receive_file_request(int socket, struct file_request *file_request_param);
/**
 * @brief Receive the structure file_transfer whit a code of status
 * @param socket socket to recieve a file
 * @param file_transefer file transfer to receive
 * @return status of operation, posibles returns: OK,ERROR_SCOKET,CLIENT_DISCONECT, INVALID_RESONSE
 */
status_operation_socket receive_file_transfer(int socket, struct file_transfer *file_transfer_param );

/**
 * @brief Receive the structure status operation whit a code of status
 * @param socket socket to recieve a file
 * @param status_operation status of operation 
 * @return status of operation, posibles returns: OK,ERROR_SCOKET,CLIENT_DISCONECT, INVALID_RESONSE
 */
status_operation_socket receive_status_code(int socket,return_code *status_operation);

/**
 * @brief Receive the element of a list
 * @param socket socket to recieve the element
 * @param elementList element to receive
 * @return status of operation, posibles returns: OK,ERROR_SCOKET,CLIENT_DISCONECT, INVALID_RESONSE
 */
status_operation_socket receive_element_list(int socket, char elementList[SIZE_ELEMENT_LIST]);

/**
 * @brief Send the first_request structure in the socket 
 * @param socket socket to comunicate
 * @param first_request_param struct to send
 * @return status of operation, posibles returns: OK,ERROR_SCOKET,CLIENT_DISCONECT, INVALID_RESONSE
 */
status_operation_socket send_first_request(int socket, struct first_request *first_request_param);

/**
 * @brief Send the file_request structure in the socket
 * @param socket socket to comunicate
 * @param file_request_param struct to send
 * @return status of operation, posibles returns: OK,ERROR_SCOKET,CLIENT_DISCONECT, INVALID_RESONSE
 */
status_operation_socket send_file_request(int socket, struct file_request *file_request_param);

/**
 * @brief Send the file_transfer structure in the socket
 * @param socket socket to comunicate
 * @param file_transfer_param struc to send
 * @return status of operation, posibles returns: OK,ERROR_SCOKET,CLIENT_DISCONECT, INVALID_RESONSE
 */

status_operation_socket send_file_transfer(int socket, struct file_transfer *file_transfer_param );

/**
 * @brief Send the return_code in the socket
 * @param socket socket to comunicate
 * @param code code to send
 * @return status of operation, posibles returns: OK,ERROR_SCOKET,CLIENT_DISCONECT, INVALID_RESONSE
 */
status_operation_socket send_status_code(int socket, return_code code);

/**
 * @brief send the element of a list
 * @param socket socket to recieve the element
 * @param elementList element to receive
 * @return status of operation, posibles returns: OK,ERROR_SCOKET,CLIENT_DISCONECT, INVALID_RESONSE
 */
status_operation_socket send_element_list(int socket, char elementList[SIZE_ELEMENT_LIST]);
