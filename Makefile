CC=gcc
CFLAGS=-g -Wall
QFLAGS=-lrt -g -I/usr/include/tirpc
BIN_FILES= server 

.PHONY: all clean 

all: $(BIN_FILES)

clean: 
	rm -f $(BIN_FILES) *.o
	rm -f *.so

server: server.c libcomm.so 
	$(CC) $(CFLAGS) $(QFLAGS) server.c rpc/info_clnt.o rpc/info_xdr.o -lnsl -lpthread -ldl -ltirpc -L. -lcomm -o $@ 

libcomm.so: comm.o
	$(CC) -shared $(CFLAGS) -o libcomm.so comm.o
	 
comm.o: comm.c
	$(CC) $(CFLAGS) -c comm.c -o comm.o
