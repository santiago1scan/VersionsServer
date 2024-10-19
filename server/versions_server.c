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
 * @param comment Comentario
 * @param result Nueva version en memoria
 *
 * @return Resultado de la operacion
 */
return_code create_version(char * filename, char * comment, file_version * result);

/**
 * @brief Verifica si existe una version para un archivo
 *
 * @param filename Nombre del archivo
 * @param hash Hash del contenido
 *
 * @return 1 si la version existe, 0 en caso contrario.
 */
int version_exists(char * filename, char * hash);

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
*
* @return 1 si la operacion es exitosa, 0 en caso contrario.
*/
int store_file(char * filename, char * hash);

/**
* @brief Almacena un archivo en el repositorio
*
* @param hash Hash del archivo: nombre del archivo en el repositorio
* @param filename Nombre del archivo
* 
* @return 1 si la operacion es exitosa, 0 en caso contrario.
*/
int retrieve_file(char * hash, char * filename);

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

return_code add(char * filename, char * comment) {

	file_version v;

	// 1. Crea la nueva version en memoria

	create_version(filename, comment, &v);
	// 2. Verifica si ya existe una version con el mismo hash
	if(version_exists(filename, v.hash) == 1)
		return VERSION_ALREADY_EXISTS;
	// 3. Almacena el archivo en el repositorio.
	if( store_file(filename, v.hash) != 1)
		return VERSION_ERROR;
	// 4. Agrega un nuevo registro al archivo versions.db
	if(add_new_version(&v) != 1)
		return VERSION_ERROR;

	// Si la operacion es exitosa, retorna VERSION_ADDED
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


void list(char * filename) {

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

int version_exists(char * filename, char * hash) {
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
		if(strcmp(versions[i].filename, filename) == 0 && strcmp(versions[i].hash, hash) == 0)
			return 1;
	// Verifica si en la bd existe un registro que coincide con filename y hash
	return 0;
}

int get(char * filename, int version) {
	//abre la base de datos de versiones .versions/versions.db
	//y validamos que se haya abierto correctamente
	file_version r;
	FILE * fp = fopen(".versions/versions.db", "rb");

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
				if(!retrieve_file(r.hash, r.filename));
					return 1;
			}
			cont++;		
		}
	}
	fclose(fp);

}

int store_file(char * filename, char * hash) {
	char dst_filename[PATH_MAX];
	snprintf(dst_filename, PATH_MAX, "%s/%s", VERSIONS_DIR, hash);
	return copy(filename, dst_filename);
}

int retrieve_file(char * hash, char * filename) {
	char src_filename[PATH_MAX];
	snprintf(src_filename, PATH_MAX, "%s/%s", VERSIONS_DIR, hash);
	return copy(src_filename, filename);
}

