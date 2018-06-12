CC=gcc
BIN_DIR=./bin/
LIB_DIR=./lib/
INC_DIR=./include/
SRC_DIR=./src/

all: $(BIN_DIR)t2fs.o
	ar crs $(LIB_DIR)libt2fs.a $(BIN_DIR)t2fs.o $(LIB_DIR)apidisk.o $(LIB_DIR)bitmap2.o

$(BIN_DIR)t2fs.o: $(SRC_DIR)t2fs.c $(INC_DIR)t2fs.h
	$(CC) -Wall -c $(SRC_DIR)t2fs.c -o $(BIN_DIR)t2fs.o

clean:
	rm -rf $(LIB_DIR)*.a $(BIN_DIR)t2fs.o
