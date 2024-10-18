all: main.o sha256.o versions.o
	gcc -o versions main.o sha256.o versions.o

main.o: main.c
	gcc -c -o main.o main.c

sha256.o: sha256.c
	gcc -c -o sha256.o sha256.c

versions.o: versions.c
	gcc -c -o versions.o versions.c

clean:
	rm -f versions *.o
	rm -rf docs

clean-repo:
	rm -rf .versions

doc:
	doxygen

install: all
	sudo cp versions /usr/local/bin

uninstall:
	sudo rm -f /usr/local/bin/versions

