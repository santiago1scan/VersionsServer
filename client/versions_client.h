/**
 * @file
 * @brief API de gestion de versiones
 * @author Erwin Meza Vega <emezav@unicauca.edu.co>
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
	char filename[PATH_MAX]; /**< Nombre del archivo original. */
	char hash[HASH_SIZE];           /**< Hash del contenido del archivo. */
	char comment[COMMENT_SIZE];	   /**< Comentario del usuario. */
}file_version;

/**
 * @brief Codigo de retorno de operacion
 */
typedef enum {
	VERSION_ERROR, /*!< Error no especificado */
	VERSION_CREATED, /*!< Version creada */
	VERSION_ADDED, /*!< Version agregada */
	VERSION_ALREADY_EXISTS, /*!< Version ya existe */
	FILE_ADDED, /*<! Archivo adicionado  */
	/* .. */
}return_code;

/**
 * @brief Adiciona un archivo al repositorio.
 *
 * @param filename Nombre del archivo a adicionar
 * @param comment Comentario de la version actual
 * @param socket socket to write 
 * @return Codigo de la operacion
 */
return_code add(char * filename, char * comment, int socket);

/**
 * @brief Lista las versiones de un archivo.
 *
 * @param filename Nombre del archivo, NULL para listar todo el repositorio.
 */
void list(char * filename);

/**
 * @brief Obtiene una version del un archivo.
 * Sobreescribe la version existente.
 *
 * @param filename Nombre de archivo.
 * @param version Numero secuencial de la version.
 *
 * @return 1 en caso de exito, 0 si ocurre un error.
 */
int get(char * filename, int version);

#endif
