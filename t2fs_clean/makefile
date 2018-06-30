#
# Makefile de EXEMPLO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# É NECESSARIO ADAPTAR ESSE ARQUIVO de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "support.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "t2fs"
# 

CC=gcc
LIB_DIR=./lib/
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/
EXE_DIR=./exemplo/
TEST_DIR=./teste/
all: t2fs.o
	ar crs $(LIB_DIR)libt2fs.a $(BIN_DIR)t2fs.o $(LIB_DIR)apidisk.o $(LIB_DIR)bitmap2.o

t2fs.o: $(SRC_DIR)t2fs.c $(INC_DIR)t2fs.h
	$(CC) -Wall -c $(SRC_DIR)t2fs.c -o $(BIN_DIR)t2fs.o

t1.o: $(TEST_DIR)t1.c $(INC_DIR)t2fs.h
	$(CC) -o $(EXE_DIR)t1 $(TEST_DIR)t1.c -L$(LIB_DIR) -lt2fs -lm -Wall

main.o: $(TEST_DIR)main.c $(INC_DIR)t2fs.h
	$(CC) -o $(EXE_DIR)main $(TEST_DIR)main.c -L$(LIB_DIR) -lt2fs -lm -Wall

clean:
	rm -rf $(LIB_DIR)*.a $(BIN_DIR)t2fs.o $(TEST_DIR)*.o


