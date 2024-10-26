/**
 * @file
 * @brief Implementacion del API de gestion de versiones para el lado del servidor
 * @author Erwin Meza Vega <emezav@unicauca.edu.co>
 * @author Miguel Angel Calambas Vivas <mangelcvivas@unicauca.edu.co>
 * @author Esteban Santiago Escandon Causaya <estebanescandon@unicauca.edu.co>
 * @copyright MIT License
*/

#include "versions_server.h"

/**
 * @brief Crea una version en memoria del archivo
 * 
 * @param filename Nombre del archivo
 * @param hash hash of the file\
 * @param idClient id del cliente
 * @param result Nueva version en memoria
 * @return Resultado de la operacion VERSION_ERROR,VERSION_CREATED,VERSION_ADDED,VERSION_ALREADY_EXISTS,VERSION_NOT_EXISTS,FILE_ADDED.   
 */
return_code create_version(char * filename, char * hash, int idClient ,file_version * result);

/**
 * @brief Verifica si existe una version para un archivo
 *
 * @param filename Nombre del archivo
 * @param hash Hash del contenido
 * @param clientId id del cliente
 * @return 1 si la version existe, 0 en caso contrario.
 */
int version_exists(char * filename, int clientId,char * hash);

/**
* @brief Almacena un archivo en el repositorio con el hash como nombre
*
* @param file info del archivo a guardar
* @param hash Hash del archivo: nombre del archivo en el repositorio
* @param socket socket ha comunicar
* @param sizeFile tamanio del archivo
* @return OK,ERROR_SOCKET,CLIENT_DISCONECT,INVALID_RESPONSE,ERROR,   
*/
status_operation_socket store_file(char * file, char * hash, int socket, int sizeFile);

/**
* @brief Envia un archivo almacenado en el repositorio
*
* @param hash Hash del archivo: nombre del archivo en el repositorio
* @param socket socket ha comunicar
* @param sizeFile tamanio del archivo
* 
* @return OK,ERROR_SOCKET,CLIENT_DISCONECT,INVALID_RESPONSE,ERROR,   
*/
status_operation_socket retrieve_file(char * hash, int socket ,int sizeFile);

/**
 * @brief Adiciona una nueva version de un archivo en el .db.
 *
 * @param filename Nombre del archivo.
 * @param comment Comentario de la version.
 * @param hash Hash del contenido.
 *
 * @return 1 en caso de exito, 0 en caso de error.
 */
int add_new_version(file_version * v);


return_code create_version(char * filename, char * hash, int idClient,file_version * result) {
	//Estructuras necesarias
	struct stat statbuff;

	// Llena la estructura result con los datos del archivo
    strncpy(result->filename, filename, sizeof(result->filename) - 1);
    result->filename[sizeof(result->filename) - 1] = '\0'; 

	strncpy(result->hash, hash, sizeof(result->hash) - 1);
	result->hash[sizeof(result->hash) - 1] = '\0';

	result->idCliente = idClient;

	return VERSION_CREATED;

}

return_code add(int socket, int idCliente) {
	file_version v;

	//1.Resibir la informacion de nombre y hash 
	size_t size_info = sizeof(struct file_request);
	struct file_request info_file;
	size_t bytes_read = receive_file_request(socket, &info_file);

	if(bytes_read != OK)
		return VERSION_ERROR;

	//Crea la nueva version en memoria
	create_version(info_file.nameFile, info_file.hashFile, idCliente,&v);
	
	//2.Validar si existe, y dar respuesta

	size_t existVersion = version_exists(info_file.nameFile, idCliente, v.hash);

	//2.1 Notificamos al usuario

	return_code response_user = existVersion ?VERSION_ALREADY_EXISTS:VERSION_NOT_EXISTS;

	if(send_status_code(socket, response_user) != OK)
		return VERSION_ERROR;
	if(existVersion)
		return VERSION_ALREADY_EXISTS;
	//3.Resibir el tamanio del archivo con el comentario
	struct file_transfer info_file_transfer;
	// printf("Se ha intentado recibir el file_transfer\n");

	if( receive_file_transfer(socket, &info_file_transfer) != OK )
		return VERSION_ERROR;

	//4.Resibir el archivo 
	
	strncpy(v.comment, info_file_transfer.comment, sizeof(v.comment) - 1);
	v.comment[sizeof(v.comment) - 1] = '\0';

	//Almacena el archivo en el repositorio.
	if( store_file(info_file.nameFile, v.hash, socket, info_file_transfer.filseSize) != OK){	
		send_status_code(socket, VERSION_ERROR);
		return VERSION_ERROR;
	}
	//Agrega un nuevo registro al archivo versions.db
	if(add_new_version(&v) != 1){
		send_status_code(socket, VERSION_ERROR);
		return VERSION_ERROR;
	}

	//5.Responder el estado de si se guardon
	// Si la operacion es exitosa, retorna VERSION_ADDED
	return_code response = VERSION_ADDED;

	send_status_code(socket, response);
	
	return VERSION_ADDED;
}

int add_new_version(file_version * v) {
	// Abre el archivo versions.db en modo append 
	//y verificamos que se haya abierto correctamente
	pthread_mutex_lock(&mutexDB);
	FILE * fp;
	fp = fopen(".versions/versions.db", "ab");
	
	if(fp == NULL){
		pthread_mutex_unlock(&mutexDB);
		return 0;
	}
	// Escribe la estructura v en el archivo, verifica que se haya escrito correctamente
	// y cierra el archivo 
	if( fwrite(v, sizeof(file_version), 1, fp) != 1){
		fclose(fp);
		pthread_mutex_unlock(&mutexDB);
		return 0;
	}
	fclose(fp);
	pthread_mutex_unlock(&mutexDB);
	return 1;
}

return_code list(int socket, int idCliente) {
	//1. Resibimos la informacion del archivo
	struct file_request file;
	char message[SIZE_ELEMENT_LIST];

	if( receive_file_request(socket, &file) != OK){
		snprintf(message, SIZE_ELEMENT_LIST, "END");
		send_element_list(socket, message);
		return VERSION_ERROR;
	}
	char filename[file.sizeNameFile +1];
	strncpy(filename, file.nameFile, file.sizeNameFile);
	filename[file.sizeNameFile] = '\0'; // Asegurarse de que la cadena esté terminada en nulo
	
	//2. Responder con la lista de versiones hasta un END
	//   si no hay simplemente manda el END
	//	Abre el la base de datos de versiones (versions.db) de manera segura
	pthread_mutex_lock(&mutexDB);
	FILE * fp = fopen(".versions/versions.db", "r");
	file_version  r;
	if(fp  == NULL ){
		pthread_mutex_unlock(&mutexDB);
		snprintf(message, SIZE_ELEMENT_LIST, "END");
		send_element_list(socket, message);
		return VERSION_ERROR;
	}

	//Leer hasta el fin del archivo 
	int cont = 1;

	while(!feof(fp)){
		//Realizar una lectura y retornar
		if(fread(&r, sizeof(file_version), 1, fp) != 1){
			break;
		}
		if(strcmp(filename, "") ==0 && r.idCliente == idCliente){
			//Si filename es NULL, muestra todos los registros.
			snprintf(message, SIZE_ELEMENT_LIST, "%d %s %s  %.5s", cont, r.filename, r.comment, r.hash);
			if( send_element_list(socket, message) != OK)
				break;
			cont ++;
		
		}else if(EQUALS(r.filename,filename) && r.idCliente == idCliente){
			snprintf(message, SIZE_ELEMENT_LIST, "%d %s %s  %.5s", cont, r.filename, r.comment, r.hash);
			if( send_element_list(socket, message) != OK)
				break;
			cont++;
		}
		//Si el registro corresponde al archivo buscado, imprimir
		//Muestra los registros cuyo nombre coincide con filename.
	}	

	snprintf(message, SIZE_ELEMENT_LIST, "END");
	send_element_list(socket, message);
	fclose(fp);
	pthread_mutex_unlock(&mutexDB);
	return VERSION_ADDED;
}

int version_exists(char * filename, int idClient,char * hash) {
	//abre la base de datos de versiones .versions/versions.db
	pthread_mutex_lock(&mutexDB);
	FILE * fp = fopen(".versions/versions.db", "rb");

	if( fp == NULL)
		return 0;
	
	file_version * versions;

	// Obtiene el tamaño del archivo	
	fseek(fp, 0, SEEK_END);
	long fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// Obtiene la cantidad de registros en la base de datos
	//Y reserva memoria para almacenarlos
	int countVersions = fileSize / sizeof(file_version);
	versions = (file_version *) malloc(fileSize*sizeof(file_version));
	
	if(versions == NULL){
		fclose(fp);
		pthread_mutex_unlock(&mutexDB);
		return 0;
	}

	// Lee los registros de la base de datos
	fread(versions, sizeof(file_version), countVersions, fp);
	fclose(fp);
	pthread_mutex_unlock(&mutexDB);
	for(int i = 0; i < countVersions; i++)
		if(strcmp(versions[i].filename, filename) == 0 && strcmp(versions[i].hash, hash) == 0 && versions[i].idCliente == idClient)
			return 1;
			
	// Verifica si en la bd existe un registro que coincide con filename y hash
	return 0;
}

return_code get(int socket, int idCliente) {
	//1.Resibir la informacion de nombre y version 
	size_t size_info = sizeof(struct file_request);
	struct file_request info_file;

	if( receive_file_request(socket, &info_file) != OK)
		return VERSION_ERROR;
	
	int version = info_file.version;

	//2. Respondemos con al longitud del archivo
	char filename[PATH_MAX];
	strncpy(filename, info_file.nameFile, PATH_MAX - 1);
	filename[PATH_MAX - 1] = '\0';

	//abre la base de datos de versiones .versions/versions.db
	//y validamos que se haya abierto correctamente
	pthread_mutex_lock(&mutexDB);
	file_version r;
	FILE * fp = fopen(".versions/versions.db", "rb");

	if( fp == NULL){
		pthread_mutex_unlock(&mutexDB);
		return VERSION_ERROR;
	}
	

	//Leer hasta el fin del archivo verificando si el registro coincide con filename y version
	int cont = 1;
	struct file_transfer file_transfer;
	while(!feof(fp)){
		if(fread(&r, sizeof(file_version), 1, fp) != 1)
			break;
		//Si el registro corresponde al archivo buscado, lo restauramos
		if(EQUALS(r.filename,filename) && r.idCliente == idCliente){
			if(cont != version){
				cont++;
				continue;
			}

			struct stat st;
			if (stat(r.filename, &st) != 0) {
				pthread_mutex_unlock(&mutexDB);
				return VERSION_ERROR;
			}
			file_transfer.filseSize = st.st_size;
			
			if( send_file_transfer(socket, &file_transfer) != OK){
				pthread_mutex_unlock(&mutexDB);
				return VERSION_ERROR;
			}			
			
			if( retrieve_file(r.hash, socket, st.st_size) != OK){
				pthread_mutex_unlock(&mutexDB);
				return VERSION_ERROR;
			}

			fclose(fp);
			pthread_mutex_unlock(&mutexDB);
			return VERSION_ADDED;	
		}
	}
	file_transfer.filseSize = 0;
	send_file_transfer(socket, &file_transfer);
	fclose(fp);
	pthread_mutex_unlock(&mutexDB);
	return VERSION_NOT_EXISTS;

}

status_operation_socket store_file(char * file, char * hash, int socket, int sizeFile) {
	char dst_filename[PATH_MAX];
	snprintf(dst_filename, PATH_MAX, "%s/%s", VERSIONS_DIR, hash);
	return receive_file(socket, dst_filename);
}

status_operation_socket retrieve_file(char * hash, int socket,int sizeFile) {
	char src_filename[PATH_MAX];
	snprintf(src_filename, PATH_MAX, "%s/%s", VERSIONS_DIR, hash);
	return send_file(socket, src_filename);
}

