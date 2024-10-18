Elaborado por: Erwin Meza Vega <emezav@unicauca.edu.co>

Los Sistemas de Control de Versiones (VCS) permiten guardar el rastro
de las modificaciones sobre determinados elementos. En el contexto de este
proyecto, se gestionarán archivos.

Se deberá implementar un sistema de control de versiones simple, que permita:

 * Adicionar un archivo al repositorio de versiones.
 * Listar las versiones de un archivo en el repositorio de versiones.
 * Listar todos los archivos existentes en el repositorio
 * Obtener la versión de un archivo del repositorio de versiones.

En esta implementación sólo se deberá realizar el control de versiones por
directorio, en el cual sólo se pueden agregar archivos que se encuentren en el
directorio actual.

Uso:
		versions add ARCHIVO "Comentario" : Adiciona una version del archivo al repositorio
		versions list ARCHIVO             : Lista las versiones del archivo existentes
        versions list                     : Lista todos los archivos almacenados en el repositorio
		versions get NUMVER ARCHIVO       : Obtiene una version del archivo del repositorio
# proyecto01-SO
