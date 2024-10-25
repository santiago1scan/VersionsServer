/**
 * @file
 * @brief Implementacion del API de gestion de versiones
 * @author Erwin Meza Vega <emezav@unicauca.edu.co>
 * @author Miguel Angel Calambas Vivas <mangelcvivas@unicauca.edu.co>
 * @author Esteban Santiago Escandon Causaya <estebanescandon@unicauca.edu.co>
 * @copyright MIT License
*/

#include "versions_client.h"


/**
 * @brief Crea una version en memoria del archivo
 * Valida si el archivo especificado existe y crea su hash
 * @param filename Nombre del archivo
 * @param comment Comentario
 * @param result Nueva version en memoria
 *
 * @return Resultado de la operacion
 */
return_code create_version(char * filename, char * comment, file_version * result);


/**
 * @brief Obtiene el hash de un archivo.
 * @param filename Nombre del archivo a obtener el hash
 * @param hash Buffer para almacenar el hash (HASH_SIZE)
 * @return Referencia al buffer, NULL si ocurre error
 */
char *get_file_hash(char * filename, char * hash);

/**
 * @brief Copia un archivo
 *
 * @param source Archivo fuente
 * @param destination Destino
 *
 * @return 1 si la operacion es exitosa, 0 en caso contrario.
 */
int copy(char * source, char * destination);

/**
* @brief Almacena un archivo en el repositorio
*
* @param filename Nombre del archivo
* @param hash Hash del archivo: nombre del archivo en el repositorio
* @param socket socket de conexion
* @param sizeFile tamaño del archivo
*
* @return 1 si la operacion es exitosa, 0 en caso contrario.
*/
int store_file(char * filename, char * hash, int socket, int sizeFile);

/**
* @brief Almacena un archivo en el repositorio
*
* @param hash Hash del archivo: nombre del archivo en el repositorio
* @param filename Nombre del archivo
* @param socket socket de conexion
* @param sizeFile tamaño del archivo
* @return 1 si la operacion es exitosa, 0 en caso contrario.
*/
int retrieve_file(char * filename, char * hash, int socket, int sizeFile);


/**
 * @brief Adiciona una nueva version de un archivo.
 *
 * @param filename Nombre del archivo.
 * @return 1 en caso de exito, 0 en caso de error.
 */
int getFileSize(char * filename);


return_code create_version(char * filename, char * comment, file_version * result) {
	file_version v;
	struct stat statbuff;
	// Verifica si el archivo existe y es regular
	if(stat(filename, &statbuff) < 0)
		return VERSION_ERROR;

	if( !S_ISREG(statbuff.st_mode) )
		return VERSION_ERROR;
	// Obtiene el hash del archivo que sera el nombre dentro del versionado
	char *hash = get_file_hash(filename, v.hash);
	if(hash == NULL)
		return VERSION_ERROR;

	// Llena la estructura result con los datos del archivo
    strncpy(result->filename, filename, sizeof(result->filename) - 1);
    result->filename[sizeof(result->filename) - 1] = '\0'; 

    strncpy(result->comment, comment, sizeof(result->comment) - 1);
    result->comment[sizeof(result->comment) - 1] = '\0';
	
	strncpy(result->hash, hash, sizeof(result->hash) - 1);
	result->hash[sizeof(result->hash) - 1] = '\0';

	return VERSION_CREATED;

}

return_code add(char * filename, char * comment, int client_socket) {
	
	file_version v;
	return_code status;
	struct file_request versionsSend ;
	strncpy(versionsSend.nameFile, filename, sizeof(versionsSend.nameFile) - 1);
    versionsSend.nameFile[sizeof(versionsSend.nameFile) - 1] = '\0'; 

	// 1. Crea la nueva version en memoria

	create_version(filename, comment, &v);
	strncpy(versionsSend.hashFile, v.hash, sizeof(versionsSend.hashFile) - 1);
    versionsSend.hashFile[sizeof(versionsSend.hashFile) - 1] = '\0'; 



	if(send_file_request(client_socket, &versionsSend )!= OK){
		printf("Falla escritura \n");
		return VERSION_ERROR;

	}
	printf("Se envio el sendRequest \n");
	if(receive_status_code(client_socket, &status) != OK){
		printf("!!!!!!!!!Falla al recibir del servidor \n");
		return VERSION_ERROR;
	}


	
	
	if(status == VERSION_ALREADY_EXISTS){
		printf("!!!!!!!!!la version ya existe\n");
		return status;
	}

	int file_size = getFileSize(filename);
	if(file_size == -1){
		printf("!!!!!!!!!!!!!!!!erro al leer el tamaño \n");
		return VERSION_ERROR;
	}	
	struct file_transfer sendVersionsTransfer;
	sendVersionsTransfer.filseSize = file_size; 
	
	strncpy(sendVersionsTransfer.comment, comment, sizeof(sendVersionsTransfer.comment) - 1);
    sendVersionsTransfer.comment[sizeof(sendVersionsTransfer.comment) - 1] = '\0'; 




	if(send_file_transfer(client_socket, &sendVersionsTransfer) != OK){
		printf("!!!!!!!!!Falla escritura \n");
		return VERSION_ERROR;
	}
	printf("Se envio el fle transfer \n");
	if(send_file(client_socket, filename, file_size) != 0){
		printf("!!!!!!!!!Error al mandar el archivo \n");
		return VERSION_ERROR;
	}
	printf("!!!!!!!!!Se envio el sendFIle\n");
	return_code satusOperation;
	
	if(receive_status_code(client_socket, &satusOperation ) != OK ){
		printf("!!!!!!!!!Error de conexion \n");
		return VERSION_ERROR;
	}
	printf("Se recibio el satus code del servidor \n");
	if(satusOperation != VERSION_ADDED){
		printf("!!!!!!!!!Error al agregar \n");
		return VERSION_ERROR;
	}
	printf("__________Se recibio el satus code del servidor archivo agregado \n");
	// Si la operacion es exitosa, retorna VERSION_ADDED
	return VERSION_ADDED;
}




void list(char * filename, int socket) {

	struct file_request versionsSend ;
	strncpy(versionsSend.nameFile, filename, sizeof(versionsSend.nameFile) - 1);
    versionsSend.nameFile[sizeof(versionsSend.nameFile) - 1] = '\0'; 

	if(send_file_request(socket, &versionsSend )!= OK){
		printf("Falla escritura \n");


	}
	printf("Se envio el sendRequest \n");
	char elementList[SIZE_ELEMENT_LIST];
	int count = 0;
	
	do{
		
		if(receive_element_list(socket, elementList)!= OK){
			
			printf("!!!!!ERROR al recibir element dentro del whilelist\n");
			
		}else{
			if(strcmp(elementList, "END")== 0){
				break;
			}
		}

		count= count +1;
		printf("%s \n",elementList);

	}while(1);
	if(count == 0 ){
		printf("no se encontro versionse a listar");
	}
}

char *get_file_hash(char * filename, char * hash) {
	char *comando;
	FILE * fp;
	struct stat s;

	//Verificar que el archivo existe y que se puede obtener el hash
	if (stat(filename, &s) < 0 || !S_ISREG(s.st_mode)) {
		perror("stat");
		return NULL;
	}

	//Generar el comando para obtener el hash
	sha256_hash_file_hex(filename, hash);

	return hash;

}

int copy(char * source, char * destination) {
	// Copia el contenido de source a destination (se debe usar open-read-write-close, o fopen-fread-fwrite-fclose)
	FILE *src, *dest;
    char buffer[1024];
    size_t bytesRead;

    // Abre el archivo fuente en modo lectura binaria
    src = fopen(source, "rb");
    if (src == NULL)
        return 0; 

    // Abre el archivo destino en modo escritura binaria
    dest = fopen(destination, "wb");
    if (dest == NULL) {
        fclose(src);
        return 0; 
    }

    // Lee del archivo fuente y escribe en el archivo destino
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytesRead, dest) != bytesRead) {
            perror("Error al escribir en el archivo destino");
            fclose(src);
            fclose(dest);
            return 0;
        }
    }

    // Cierra ambos archivos
    fclose(src);
    fclose(dest);

    return 1;
}


int get(char * filename, int version, int socket) {
	//abre la base de datos de versiones .versions/versions.db
	//y validamos que se haya abierto correctamente
	file_version v;
	struct file_request versionsSend;
	strncpy(versionsSend.nameFile, filename, sizeof(versionsSend.nameFile) - 1);
    versionsSend.nameFile[sizeof(versionsSend.nameFile) - 1] = '\0'; 
	
	versionsSend.version = version;

	if(send_file_request(socket, (void *)&versionsSend )!= OK){
		printf("Falla escritura\n");
	}

	struct file_transfer info_file;
	
	if(receive_file_transfer(socket, &info_file) != OK){
		return VERSION_ERROR;
	}

	if(info_file.filseSize == 0){
		return VERSION_NOT_EXISTS;
	}
	
	if(receive_file(socket, filename, info_file.filseSize) != 0){
		printf("!!!!!!!!!Error al recibir el archivo \n");
		return VERSION_ERROR;
	}

	return VERSION_ADDED;

}

int store_file(char * filename, char * hash, int socket, int sizeFile) {
	char dst_filename[PATH_MAX];
	snprintf(dst_filename, PATH_MAX, "%s/%s", VERSIONS_DIR, hash);
	return receive_file(socket, dst_filename, sizeFile);
}

int retrieve_file(char * hash, char * filename, int socket, int sizeFile) {
	char src_filename[PATH_MAX];
	snprintf(src_filename, PATH_MAX, "%s/%s", VERSIONS_DIR, hash);
	return send_file(socket, src_filename, sizeFile);
}


int getFileSize(char * filename){
	struct stat st;
	if(stat(filename, &st) != 0){
		perror("error obtener tamaño del archivo");
		return -1;
	}
	return st.st_size;
}


