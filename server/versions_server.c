/**
 * @file
 * @brief Implementacion del API de gestion de versiones
 * @author Erwin Meza Vega <emezav@unicauca.edu.co>
 * @author Miguel Angel Calambas Vivas <mangelcvivas@unicauca.edu.co>
 * @author Esteban Santiago Escandon Causaya <estebanescandon@unicauca.edu.co>
 * @copyright MIT License
*/

#include "versions_server.h"

/**
 * @brief Crea una version en memoria del archivo
 * Valida si el archivo especificado existe y crea su hash
 * @param filename Nombre del archivo
 * @param hash hash of the file\
 * @param idClient id del cliente
 * @param result Nueva version en memoria
 *
 * @return Resultado de la operacion
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
 * @brief Obtiene el hash de un archivo.
 * @param filename Nombre del archivo a obtener el hash
 * @param hash Buffer para almacenar el hash (HASH_SIZE)
 * @return Referencia al buffer, NULL si ocurre error
 */
char *get_file_hash(char * filename, char * hash);

/**
 * @brief Crea un arhcivo con la informacion dicha
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
* @param file info del archivo a guardar
* @param hash Hash del archivo: nombre del archivo en el repositorio
* @param socket socket ha comunicar
* @param sizeFile tamanio del archivo
* @return 1 si la operacion es exitosa, 0 en caso contrario.
*/
int store_file(char * file, char * hash, int socket, int sizeFile);

/**
* @brief Almacena un archivo en el repositorio
*
* @param hash Hash del archivo: nombre del archivo en el repositorio
* @param filename Nombre del archivo
* @param sizeFile tamanio del archivo
* @param socket socket ha comunicar
* 
* @return 1 si la operacion es exitosa, 0 en caso contrario.
*/
int retrieve_file(char * hash, int socket ,int sizeFile);

/**
 * @brief Adiciona una nueva version de un archivo.
 *
 * @param filename Nombre del archivo.
 * @param comment Comentario de la version.
 * @param hash Hash del contenido.
 *
 * @return 1 en caso de exito, 0 en caso de error.
 */
int add_new_version(file_version * v);


return_code create_version(char * filename, char * hash, int idClient,file_version * result) {
	file_version v;
	struct stat statbuff;
	// Verifica si el archivo existe y es regular
	if(stat(filename, &statbuff) < 0)
		return VERSION_ERROR;

	if( !S_ISREG(statbuff.st_mode) )
		return VERSION_ERROR;

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
	struct file_request *info_file = malloc(size_info);

	size_t bytes_read = read(socket, info_file, size_info);
	
	//Validamos que el estado sea correcto
	return_code_protocol state= validateRead( bytes_read);
	if( state != ALL_OK)
		return state;
	//Crea la nueva version en memoria
	create_version(info_file->pathFile, info_file->hashFile, idCliente,&v);
	
	//2.Validar si existe, y dar respuesta

	size_t existVersion = version_exists(info_file->pathFile, idCliente,v.hash);

	//Respondemos si existe el usuario
	state =validateWrite(write(socket, (void *)existVersion, sizeof(int))); 
	if( state != ALL_OK)
		return state;

	//Si existe acabamos la ejecucion
	if(existVersion == 1)
		return VERSION_ALREADY_EXISTS;
	
	//3.Resibir lel tamanio del archivo con el comentario
	size_t size_file_transfer = sizeof(struct file_transfer);
	struct file_transfer *info_file_transfer = malloc(size_file_transfer);

	bytes_read = read(socket, info_file_transfer, size_file_transfer);

	state = validateRead(bytes_read);
	if(state != ALL_OK)
		state;

	//4.Resibir el archivo 
	
	strncpy(v.comment, info_file_transfer->comment, sizeof(v.comment) - 1);
	v.comment[sizeof(v.comment) - 1] = '\0';

	//Almacena el archivo en el repositorio.
	if( store_file(info_file->pathFile, v.hash, socket, info_file_transfer->filseSize) != 1){
		validateRead(write(socket, VERSION_ERROR, sizeof(return_code_protocol)));
		
		return VERSION_ERROR;
	}
	//Agrega un nuevo registro al archivo versions.db
	if(add_new_version(&v) != 1){
		validateRead(write(socket, VERSION_ERROR, sizeof(return_code_protocol)));
		return VERSION_ERROR;
	}

	//5.Responder el estado de si se guardon
	// Si la operacion es exitosa, retorna VERSION_ADDED
	return_code_protocol response = ALL_OK; // Asegúrate de que ALL_OK esté correctamente inicializado
    validateRead(write(socket, &response, sizeof(return_code_protocol)));
	return VERSION_ADDED;
}

int add_new_version(file_version * v) {
	// Abre el archivo versions.db en modo append 
	//y verificamos que se haya abierto correctamente
	FILE * fp;
	fp = fopen(".versions/versions.db", "ab");
	
	if(fp == NULL)
		return 0;
	// Escribe la estructura v en el archivo, verifica que se haya escrito correctamente
	// y cierra el archivo 
	if( fwrite(v, sizeof(file_version), 1, fp) != 1){
		fclose(fp);
		return 0;
	}
	fclose(fp);
	return 1;
}

void list(int socket, int idCliente) {
	//1. Resibimos la informacion del archivo
	char filename[PATH_MAX];

	size_t bytes_read = read(socket, filename, PATH_MAX);

	return_code_protocol state = validateRead(bytes_read);

	if(state != ALL_OK)
		return;
	
	//Abre el la base de datos de versiones (versions.db)
	FILE * fp = fopen(".versions/versions.db", "r");
	file_version  r;
	if(fp  == NULL ){
		return;
	}

	//Leer hasta el fin del archivo 
	int cont = 1;
	size_t size_message =sizeof(int) + PATH_MAX + COMMENT_SIZE + HASH_SIZE;
	char message[size_message];
	while(!feof(fp)){
		
		//Realizar una lectura y retornar
		if(fread(&r, sizeof(file_version), 1, fp) != 1){
			break;
		}

		if(strcmp(filename, "") ==0){
			//Si filename es NULL, muestra todos los registros.
			snprintf(message, size_message, "%d %s %s  %.5s \n", cont, r.filename, r.comment, r.hash);
			cont = cont + 1;
		
		}else if(strcmp(r.filename,filename)==0){
			snprintf(message, size_message, "%d %s %s  %.5s \n", cont, r.filename, r.comment, r.hash);
			cont = cont + 1;
		}

		state = write(socket, message, size_message);
		if(state != ALL_OK)
			break;
		//Si el registro corresponde al archivo buscado, imprimir
		//Muestra los registros cuyo nombre coincide con filename.
	}	

	snprintf(message, size_message, " ");

	write(socket, message, size_message);

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
    int dest_fd;
    ssize_t bytesWritten;
    size_t source_len = strlen(source);

    // Abre el archivo destino en modo escritura (crea el archivo si no existe)
    dest_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) {
        perror("Error al abrir el archivo destino");
        return 0;
    }

    // Escribe el contenido de source en el archivo destino
    bytesWritten = write(dest_fd, source, source_len);
    if (bytesWritten != source_len) {
        perror("Error al escribir en el archivo destino");
        close(dest_fd);
        return 0;
    }

    // Cierra el archivo destino
    close(dest_fd);

    return 1;
}

int version_exists(char * filename, int idClient,char * hash) {
	//abre la base de datos de versiones .versions/versions.db
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
		return 0;
	}

	// Lee los registros de la base de datos
	fread(versions, sizeof(file_version), countVersions, fp);
	fclose(fp);
	for(int i = 0; i < countVersions; i++)
		if(strcmp(versions[i].filename, filename) == 0 && strcmp(versions[i].hash, hash) == 0 && versions[i].idCliente == idClient)
			return 1;
	// Verifica si en la bd existe un registro que coincide con filename y hash
	return 0;
}

int get(int socket, int idCliente) {
	//1.Resibir la informacion de nombre y version 
	size_t size_info = sizeof(struct file_request);
	struct file_request *info_file = malloc(size_info);

	size_t bytes_read = read(socket, info_file, size_info);
	
	//Validamos que el estado sea correcto
	return_code_protocol state= validateRead( bytes_read);
	if( state != ALL_OK)
		return state;
	
	int version = info_file->version;

	//2. Respondemos con al longitud del archivo
	char filename[PATH_MAX];
	strncpy(filename, info_file->pathFile, PATH_MAX - 1);
	filename[PATH_MAX - 1] = '\0'; // Asegúrate de que la cadena esté terminada en nulo
	//abre la base de datos de versiones .versions/versions.db
	//y validamos que se haya abierto correctamente
	file_version r;
	FILE * fp = fopen(".versions/versions.db", "rb");

	if( fp == NULL)
		return 0;
	
	//ESTA RE MAL ESTOOOOOO
	//Tengo que validar en todo lado el di cliente
	// Obtiene la longitud del archivo
	struct stat st;
	if (stat(filename, &st) != 0) {
		fclose(fp);
		return VERSION_ERROR;
	}
	off_t file_size = st.st_size;

	// Envia la longitud del archivo al cliente
	state = validateWrite(write(socket, &file_size, sizeof(off_t)));
	if (state != ALL_OK) {
		fclose(fp);
		return state;
	}

	//Leer hasta el fin del archivo verificando si el registro coincide con filename y version
	int cont = 1;
	while(!feof(fp)){
		if(fread(&r, sizeof(file_version), 1, fp) != 1)
			break;
		//Si el registro corresponde al archivo buscado, lo restauramos
		if(strcmp(r.filename,filename)==0){
			if(cont == version){
				if(!retrieve_file(r.hash, socket,file_size));
					return 1;
			}
			cont++;		
		}
	}
	fclose(fp);

}

int store_file(char * file, char * hash, int socket, int sizeFile) {
	char dst_filename[PATH_MAX];
	snprintf(dst_filename, PATH_MAX, "%s/%s", VERSIONS_DIR, hash);
	return receive_file(socket, dst_filename, sizeFile);
}

int retrieve_file(char * hash, int socket,int sizeFile) {
	char src_filename[PATH_MAX];
	snprintf(src_filename, PATH_MAX, "%s/%s", VERSIONS_DIR, hash);
	return send_file(socket, src_filename, sizeFile);
}

