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
	struct file_request *versionsSend = malloc(sizeof(struct file_request));
	strncpy(versionsSend-> nameFile, filename, sizeof(versionsSend->nameFile) - 1);
    versionsSend->nameFile[sizeof(versionsSend->nameFile) - 1] = '\0'; 
	versionsSend->sizeNameFile = sizeof(filename);
	// 1. Crea la nueva version en memoria

	create_version(filename, comment, &v);
	strncpy(versionsSend->hashFile, v.hash, sizeof(versionsSend->hashFile) - 1);
    versionsSend->hashFile[sizeof(versionsSend->hashFile) - 1] = '\0'; 
	if(send_file_request(client_socket, (void *)&versionsSend )!= OK){
		printf("Falla escritura");
	}
	size_t bitsRide;
	int versionsExits;
	bitsRide = read(client_socket,&versionsExits, sizeof(int));
	if(bitsRide != sizeof(int)){
		return VERSION_ERROR;
	}
	if(versionsExits == 1){
		return VERSION_ALREADY_EXISTS;
	}

	struct file_transfer *sendVersionsTransfer = malloc(sizeof(struct file_transfer ));
	
	//Tengo que validar en todo lado el di cliente
	// Obtiene la longitud del archivo
	struct stat st;
	if(stat(filename, &st) != 0){
		return VERSION_ERROR;
	}
	off_t file_size = st.st_size;
	sendVersionsTransfer->filseSize = file_size;
	strncpy(sendVersionsTransfer->comment, comment, sizeof(sendVersionsTransfer->comment) - 1);
    sendVersionsTransfer->comment[sizeof(sendVersionsTransfer->comment) - 1] = '\0'; 
	
	if(send_file_transfer(client_socket, (void *)&sendVersionsTransfer)== OK){
		printf("Falla escritura");
	}
	if(retrieve_file(v.hash,filename, client_socket, file_size) == 0){
		printf("Envio de archivo fallida ");
	}
	// Si la operacion es exitosa, retorna VERSION_ADDED
	return VERSION_ADDED;
}




void list(char * filename, int socket) {
	//char path= "versions";
	//Abre el la base de datos de versiones (versions.db)
	
	FILE * fp = fopen(".versions/versions.db", "r");
	file_version  r;
	if(fp  == NULL ){
		return;
	}

	//Leer hasta el fin del archivo 
	int cont = 1;
	while(!feof(fp)){
		
		//Realizar una lectura y retornar
		if(fread(&r, sizeof(file_version), 1, fp) != 1){
			break;
		}

		if(filename == NULL){
			//Si filename es NULL, muestra todos los registros.
			printf("%d %s %s  %.5s \n", cont, r.filename, r.comment, r.hash);
			cont = cont + 1;
		
		}else if(strcmp(r.filename,filename)==0){
			printf("%d %s %s  %.5s \n", cont, r.filename, r.comment, r.hash);
			cont = cont + 1;
		}
		//Si el registro corresponde al archivo buscado, imprimir
		//Muestra los registros cuyo nombre coincide con filename.
	}	
	fclose(fp);
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
	struct file_request *versionsSend = malloc(sizeof(struct file_request));
	strncpy(versionsSend-> nameFile, filename, sizeof(versionsSend->nameFile) - 1);
    versionsSend->nameFile[sizeof(versionsSend->nameFile) - 1] = '\0'; 
	
	strncpy(versionsSend->hashFile, v.hash, sizeof(versionsSend->hashFile) - 1);
    versionsSend->hashFile[sizeof(versionsSend->hashFile) - 1] = '\0'; 
	if(send_file_request(socket, (void *)&versionsSend )!= OK){
		printf("Falla escritura");
	}
	size_t bitsRide;
	int versionsExits;
	bitsRide = read(socket,&versionsExits, sizeof(int));
	if(bitsRide != sizeof(int)){
		return VERSION_ERROR;
	}
	send_file_request(socket, (void *)&versionsSend);
	file_version r;
	FILE * fp = fopen(".versions/versions.db", "rb");
	if(store_file(filename, v.hash, socket, bitsRide))
	if( fp == NULL)
		return 0;
	//Leer hasta el fin del archivo verificando si el registro coincide con filename y version
	int cont = 1;
	while(!feof(fp)){
		if(fread(&r, sizeof(file_version), 1, fp) != 1)
			break;
		//Si el registro corresponde al archivo buscado, lo restauramos
		if(strcmp(r.filename,filename)==0){
			if(cont == version){
				if(!retrieve_file(r.hash, r.filename, socket, 0));
					return 1;
			}
			cont++;		
		}
	}
	fclose(fp);

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

