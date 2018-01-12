CC=gcc
CFLAGS=-Wall `pkg-config --cflags --libs gtk+-2.0` -export-dynamic
BUILD_DIR=./build/
PROJECT_NAME=tak
CONFIG_DIR=~/.pltak
ARGS=

all: rebuild_configdir main.o parse.o jpk.o tocsv.o gui.o config.o
	pip3 install xmltodict
	wget -P ./data/ http://www.e-deklaracje.mf.gov.pl/Repozytorium/Slowniki/KodyUrzedowSkarbowych_v3-0.xsd
	python3 ./src/convert.py ./data/KodyUrzedowSkarbowych_v3-0.xsd ./data/us_codes.dat
	rm ./data/KodyUrzedowSkarbowych_v3-0.xsd
	cp ./data/us_codes.dat ~/.pltak/
	rm ./data/us_codes.dat
	cp ./data/JPK_VAT_02.csv ~/.pltak/
	$(CC) $(BUILD_DIR)main.o $(BUILD_DIR)config.o $(BUILD_DIR)tocsv.o $(BUILD_DIR)jpk.o $(BUILD_DIR)parse.o $(BUILD_DIR)gui.o $(BUILD_DIR)utils.o -o $(BUILD_DIR)$(PROJECT_NAME) $(CFLAGS) -g 

rebuild_configdir:
ifeq ($(wildcard $(BUILD_DIR)),)
		mkdir $(BUILD_DIR)
endif
ifeq ($(wildcard $(CONFIG_DIR)),)
		mkdir $(CONFIG_DIR)
endif

main.o: ./src/main.c jpk.o tocsv.o gui.o
	$(CC) -g -c ./src/main.c -o $(BUILD_DIR)$@

gui.o: ./src/gui.c jpk.o config.o utils.o
	gcc -c -o $(BUILD_DIR)$@ ./src/gui.c $(CFLAGS)

config.o: ./src/config.c jpk.o
	$(CC) -c -g ./src/config.c -o $(BUILD_DIR)$@

tocsv.o: ./src/tocsv.c jpk.o
	$(CC) -g -c ./src/tocsv.c -o $(BUILD_DIR)$@

jpk.o: ./src/jpk.c parse.o
	$(CC) -g -c ./src/jpk.c -o $(BUILD_DIR)$@

parse.o: ./src/parse.c
	$(CC) -g -c ./src/parse.c -o $(BUILD_DIR)$@

utils.o: ./src/utils.c
	$(CC) -g -c ./src/utils.c -o $(BUILD_DIR)$@

clean:
	rm -f build/*.o
