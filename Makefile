CC=gcc
CFLAGS=-g -Wall
QFLAGS=-lrt
BIN_FILES= servidor 

.PHONY: all clean 

all: $(BIN_FILES)

clean: 
	rm -f $(BIN_FILES) *.o
	rm -f *.so

servidor: servidor.c libcomm.so
	$(CC) $(CFLAGS) $(QFLAGS) servidor.c -L. -lcomm -o $@ 

libcomm.so: comm.o
	$(CC) -shared $(CFLAGS) -o libcomm.so comm.o
	 
comm.o: comm.c
	$(CC) $(CFLAGS) -c comm.c -o comm.o
