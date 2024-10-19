# Encuentra todos los archivos .c en el directorio actual y subdirectorios
SRC := $(shell find . -name '*.c')

# Genera los nombres de los archivos .o correspondientes
OBJ := $(SRC:.c=.o)

all: rversions rversionsd

#compila version del cliente
rversions: rversions.o client/versions_client.o common/sha256.o
	gcc -o rversions rversions.o client/versions_client.o common/sha256.o

#compila version del servidor
rversionsd: rversionsd.o server/versions_server.o common/sha256.o
	gcc -o rversionsd rversionsd.o server/versions_server.o common/sha256.o

# Regla genérica para compilar .c a .o
%.o: %.c
	gcc -c $< -o $@

clean:
	rm -f versions *.o
	rm -rf docs
	rm rversions rversionsd

clean-repo:
	rm -rf .versions

doc:
	doxygen

install: all
	sudo cp versions /usr/local/bin

uninstall:
	sudo rm -f /usr/local/bin/versions