Elaborado por: 

- Erwin Meza Vega <emezav@unicauca.edu.co> (Codigo base)
- Miguel Angel Calambas Vivas <mangelcvivas@unicauca.edu.co>
- Esteban Santiago Escandon Causaya <estebanescandon@unicauca.edu.co>

# 1 Sistema de Control de Versiones Remoto
Los Sistemas de Control de Versiones (VCS) permiten guardar el rastro de

las modificaciones sobre determinados elementos. En el contexto de este pro-
yecto, se gestionarán versiones de archivos y directorios.

Se deberá implementar un sistema de control de versiones remoto, que
permita:
- Adicionar un archivo al repositorio de versiones.
- Listar las versiones de un archivo en el repositorio de versiones.
- Listar todos los archivos almacenados en el repositorio
- Obtener la versión de un archivo del repositorio de versiones.

Se deberán crear dos (programas), un programa servidor llamado rversionsd,
y un programa cliente llamado rversions.

## 1.1. Uso del cliente rversions
    $ ./rversions
    Uso: rversions IP PORT Conecta el cliente a un servidor en la IP y puerto especificados.
    
    Los comandos, una vez que el cliente se ha conectado al servidor, son los siguientes:
    	add archivo "Comentario"
    	list archivo
    	list
    	get numver archivo
    
## 1.2. Uso del servidor rversionsd
    $ ./rversionsd
    Uso: rversionsd PORT Escucha por conexiones del cliente en el puerto especificado.