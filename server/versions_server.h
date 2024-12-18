/**
 * @file
 * @brief API de gestion de versiones
 * @author Erwin Meza Vega <emezav@unicauca.edu.co>
 * @author Miguel Angel Calambas Vivas <mangelcvivas@unicauca.edu.co>
 * @auhtor Santiago Escandon
 * @copyright MIT License
*/

#ifndef VERSIONS_H
#define VERSIONS_H

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "../common/sha256.h"
#include "../common/protocol.h"

#define VERSIONS_DB "versions.db" /**< Nombre de la base de datos de versiones. */
#define VERSIONS_DIR ".versions" /**< Directorio del repositorio. */
#define VERSIONS_DB_PATH VERSIONS_DIR "/" VERSIONS_DB /**< Ruta completa de la base de datos.*/

#define EQUALS(s1, s2) (strcmp(s1, s2) == 0) /**< Verdadero si dos cadenas son iguales.*/

/**
 * @brief Version de un archivo.
 * Para cada version de un archivo se almacena el nombre original,
 * el comentario del usuario y el hash de su contenido.
 * El hash es a la vez el nombre del archivo dentro del
 * repositorio.
 */
typedef  struct __attribute__((aligned(512))) {
	char filename[PATH_MAX]; 	/**< Nombre del archivo original. */
	char hash[HASH_SIZE];       /**< Hash del contenido del archivo. */
	char comment[COMMENT_SIZE];	/**< Comentario del usuario. */
	int  idCliente; 			/**< id del cliente que subio la version */
}file_version;

extern pthread_mutex_t mutexDB; /**< Mutex para proteger el acceso a la base de datos. */

/**
 * @brief Adiciona un archivo al repositorio.
 * @param socket socket ha comunicar
 * @param idCliente id del cliente
 * @return Resultado de la operacion VERSION_ERROR,VERSION_CREATED,VERSION_ADDED,VERSION_ALREADY_EXISTS,VERSION_NOT_EXISTS,FILE_ADDED. 
 */
return_code add(int socket, int idCliente);

/**
 * @brief Lista las versiones de un archivo.
 * @param socket socket ha comunicar
 * @param idCliente id del cliente
 * @return Resultado de la operacion VERSION_ERROR,VERSION_CREATED,VERSION_ADDED,VERSION_ALREADY_EXISTS,VERSION_NOT_EXISTS,FILE_ADDED. 
 */
return_code list(int socket, int idCliente);

/**
 * @brief Obtiene una version del un archivo.
 * Sobreescribe la version existente.
 * @param socket socket ha comunicar
 * @param idCliente id del cliente
 * @return Resultado de la operacion VERSION_ERROR,VERSION_CREATED,VERSION_ADDED,VERSION_ALREADY_EXISTS,VERSION_NOT_EXISTS,FILE_ADDED. 
 */
return_code get(int socket, int idCLiente);

#endif
