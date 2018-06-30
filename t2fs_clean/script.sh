#!/bin/bash

echo Rodando Script

cp t2fs_disk.dat ./exemplo/

make clean
make
make main.o
cd ./exemplo/
./main



