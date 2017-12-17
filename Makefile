CC=gcc
CFLAGS=-I
BUILD_DIR=./build/
PROJECT_NAME=tak
ARGS=

all: main.o jpk.o parse.o
	$(CC) $(BUILD_DIR)main.o $(BUILD_DIR)jpk.o $(BUILD_DIR)parse.o -o $(BUILD_DIR)$(PROJECT_NAME)

main.o: ./src/main.c jpk.o parse.o
	$(CC) -c ./src/main.c -o $(BUILD_DIR)$@

jpk.o: ./src/jpk.c parse.o
	$(CC) -c ./src/jpk.c -o $(BUILD_DIR)$@

parse.o: ./src/parse.c
	$(CC) -c ./src/parse.c -o $(BUILD_DIR)$@

clean:
	rm -f build/*.o
