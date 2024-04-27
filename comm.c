#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "comm.h"

int create_server_socket(int port, int type) {
	struct sockaddr_in addr_server;
	int sd, res;
	
	// Se crea el socket
	sd = socket(AF_INET, type, 0);
	if (sd < 0) {
		printf("Error en la creación del socket del servidor\n");
		return -1;
	}

	// Se inicializa la dirección	
	bzero((char *)&addr_server, sizeof(addr_server));
	addr_server.sin_family = AF_INET;
	addr_server.sin_addr.s_addr = INADDR_ANY;
	addr_server.sin_port = htons(port);
	
	// Se bindea
	res = bind(sd, (const struct sockaddr *)&addr_server, sizeof(addr_server));
	if (res == -1) {
		perror("");
		printf("Error en el bind del socket del servidor\n");
		return -1;
	}
	
	// Se hace el listen
	res = listen(sd, SOMAXCONN);
	if (res == -1) {
		printf("Error en el listen del socket del servidor\n");
		return -1;
	}
	
	return sd;
}

int create_client_socket(char * remote, int port) {
	struct sockaddr_in addr_server;
	struct hostent * h;
	int sd, res;
	
	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0) {
		return -1;
	}
	
	h = gethostbyname(remote);
	if (h == NULL) {
		printf("Error en gethostbyname\n");
		return -1;
	}
	
	bzero((char *)&addr_server, sizeof(addr_server));
	memcpy(&(addr_server.sin_addr), h->h_addr, h->h_length);
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(port);
	
	res = connect(sd, (struct sockaddr *)&addr_server, sizeof(addr_server));
	if (res < 0) {
		printf("Error en el connect del cliente\n");
		return -1;
	}
	
	return sd;
}

struct socket_info accept_server(int sd) {

	struct sockaddr_in addr_client;
	socklen_t size = sizeof(addr_client);
	
	struct socket_info res;	
	res.code_res = 0;

	res.sd = accept(sd, (struct sockaddr *)&addr_client, (socklen_t *)&size); 
	if (res.sd < 0) {
		printf("Error en el accept del servidor\n");
		res.code_res = -1;
		return res;
	}
	
	inet_ntop(AF_INET, &addr_client.sin_addr, res.ip, 64); 
	return res;
}

int send_message(int sd, char *buffer, int len) {
	int r, l=len;
	
	do {
		r = write(sd, buffer, l);
		if (r < 0) {return -1;}
		
		l = l-r;
		buffer = buffer + r;
	
	} while((l>0) && (r >= 0));
	
	return 0;
}

int receive_message(int sd, char *buffer, int len) {
	int r, l=len;
	
	do {
		r = read(sd, buffer, l);
		if (r < 0) {return -1;}
		
		l = l - r;
		buffer = buffer + r;
	} while((l>0) && (r >= 0));
	
	return 0;
}

ssize_t write_line(int fd, char*buffer) {
	return send_message(fd, buffer, strlen(buffer)+1);
}

ssize_t read_line(int fd, char *buffer, size_t n) {
	ssize_t n_read;
	size_t t_read;
	char *buf;
	char ch;
	
	if (n <= 0 || buffer == NULL) {
		errno = EINVAL;
		return -1;
	}
	
	buf = buffer;
	t_read = 0;
	
	while(1) {
		n_read = read(fd, &ch, 1);
		if (n_read == -1) {
			if (errno == EINTR) {continue;}
			else {return -1;}
		}
		else if(n_read == 0) {
			if (t_read == 0) {return 0;}
			else break;
		}
		else {
			if (ch == '\n') break;
			if (ch == '\0') break;
			if (t_read < n-1) {
				t_read++;
				*buf++ = ch;
			}
		}
	}
	*buf = '\0';	
	return t_read;
}
