CC=gcc
CFLAGS=-Wall `pkg-config --cflags --libs gtk+-2.0` -export-dynamic
BUILD_DIR=./build/
PROJECT_NAME=tak
ARGS=

all: main.o parse.o jpk.o tocsv.o gui.o config.o
	$(CC) $(CFLAGS) -g $(BUILD_DIR)main.o $(BUILD_DIR)config.o $(BUILD_DIR)tocsv.o $(BUILD_DIR)jpk.o $(BUILD_DIR)parse.o $(BUILD_DIR)gui.o -o $(BUILD_DIR)$(PROJECT_NAME)

main.o: ./src/main.c jpk.o tocsv.o gui.o
	$(CC) -g -c ./src/main.c -o $(BUILD_DIR)$@

gui.o: ./src/gui.c jpk.o config.o
	gcc -c -o $(BUILD_DIR)$@ ./src/gui.c $(CFLAGS)

config.o: ./src/config.c jpk.o
	$(CC) -c -g ./src/config.c -o $(BUILD_DIR)$@

tocsv.o: ./src/tocsv.c jpk.o
	$(CC) -g -c ./src/tocsv.c -o $(BUILD_DIR)$@

jpk.o: ./src/jpk.c parse.o
	$(CC) -g -c ./src/jpk.c -o $(BUILD_DIR)$@

parse.o: ./src/parse.c
	$(CC) -g -c ./src/parse.c -o $(BUILD_DIR)$@

clean:
	rm -f build/*.o
