# Encuentra todos los archivos .c en el directorio actual y subdirectorios
SRC := $(shell find . -name '*.c')

# Genera los nombres de los archivos .o correspondientes
OBJ := $(SRC:.c=.o)

all: rversions rversionsd

#compila version del cliente
rversions: rversions.o client/versions_client.o common/sha256.o common/protocol.o
	gcc -o rversions rversions.o client/versions_client.o common/sha256.o common/protocol.o

#compila version del servidor
rversionsd: rversionsd.o server/versions_server.o common/sha256.o common/protocol.o
	gcc -o rversionsd rversionsd.o server/versions_server.o common/sha256.o common/protocol.o

# Regla genérica para compilar .c a .o
%.o: %.c
	gcc -c $< -o $@

clean:
	find . -name '*.o' -exec rm -f {} +
	rm -rf docs
	rm -f rversions rversionsd

clean-repo:
	rm -rf .versions

doc:
	doxygen

install: all
	sudo cp versions /usr/local/bin

uninstall:
	sudo rm -f /usr/local/bin/versions