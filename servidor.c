#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "comm.h"

#define PATH_MAX 4096
#define cero 0

// Paths
const char *rel_path="./users";
char *abs_path;

// Variables de mutex
pthread_mutex_t mutex;
pthread_cond_t copiado;
int copia = 0;

int sd_copy(int sd){
	// Función auxiliar que copia el socket y lo devuelve  
    pthread_mutex_lock(&mutex);
    copia = 1;
    pthread_cond_signal(&copiado);
    pthread_mutex_unlock(&mutex);
    return sd;
}

void get_username_path(char *username, char *name){
    sprintf(username, "%s/%s", abs_path, name);         
}

void register_server(int * newsd) {
    int sd = sd_copy(*newsd);
    char res = '0';
    char name[256];
    read_line(sd, name, 256);
    printf("Nombre recibido: %s\n", name);

    char *userpath = calloc(PATH_MAX, sizeof(char));
    get_username_path(userpath, name);

    if(access(userpath, F_OK) == 0){
        res = '1';
        write_line(sd, &res);
        free(userpath);
        return;
    }
    FILE *userfile;
    userfile = fopen(userpath, "w+");
    if(userfile == NULL){
        res = '2';
        write_line(sd, &res);
        free(userpath);
        return;
    }
    fclose(userfile);
  
    write_line(sd, &res);
    close(sd);
    free(userpath);
	return;
}

void unregister_server(int * newsd) {
	int sd = sd_copy(*newsd);
    char res = '0';
    char name[256];
    read_line(sd, name, 256);
    printf("Nombre recibido: %s\n", name);

    char *userpath = calloc(PATH_MAX, sizeof(char));
    get_username_path(userpath, name);

    if(access(userpath, F_OK) != 0){
        res = '1';
        write_line(sd, &res);
        free(userpath);
        return;
    }

    DIR *dir = opendir(abs_path);
    struct dirent* users;
    while((users = readdir(dir)) != NULL){
        if(strcmp(users->d_name, name) == 0){
            if(remove(userpath) == -1){
                res = '2';
                write_line(sd, &res);
                free(userpath);
                return;
            } 
        }
    }

    write_line(sd, &res);
    free(userpath);
	return;
}

void connect_server(int * newsd) {
	int sd = sd_copy(*newsd);
    char res = '0';
    char nombre[256];
    read_line(sd, nombre, 256);
    printf("Nombre recibido: %s\n", nombre);
    char puerto[8];
    read_line(sd, puerto, 8);
    printf("Puerto recibido: %s\n", puerto);
    write_line(sd, &res);
	return;
}

void publish_server(int * newsd) {
	int sd = sd_copy(*newsd);
    char res = '0';
    char nombre[256];
    read_line(sd, nombre, 256);
    printf("Nombre recibido: %s\n", nombre);
    char fichero[256];
    read_line(sd, fichero, 256);
    printf("Fichero recibido: %s\n", fichero);
    char descr[256];
    read_line(sd, descr, 256);
    printf("Descripcion recibido: %s\n", descr);
    write_line(sd, &res);
	return;
}

void delete_server(int * newsd) {
	int sd = sd_copy(*newsd);
    char res = '0';
    char nombre[256];
    read_line(sd, nombre, 256);
    printf("Nombre recibido: %s\n", nombre);
    char fichero[256];
    read_line(sd, fichero, 256);
    printf("Fichero recibido: %s\n", fichero);

    write_line(sd, &res);
	return;
}

void list_users_server(int * newsd) {
	int sd = sd_copy(*newsd);
    char res = '0';
    char nombre[256];
    read_line(sd, nombre, 256);
    printf("Nombre recibido: %s\n", nombre);

    write_line(sd, "0");
    write_line(sd, "1");
    for (int i = 0; i < 1; i++) {
        write_line(sd, "alicia");
        write_line(sd, "1111.2222.3333.4444");
        write_line(sd, "42069");
    }
	return;
}

void list_content_server(int * newsd) {
    
	int sd = sd_copy(*newsd);
    char res = '0';
    char nombre[256];
    read_line(sd, nombre, 256);
    printf("Nombre recibido: %s\n", nombre);

    write_line(sd, &res);
	return;
    
}

void disconnect_server(int * newsd) {
	int sd = sd_copy(*newsd);
    char res = '0';
    char nombre[256];
    read_line(sd, nombre, 256);
    printf("Nombre recibido: %s\n", nombre);
    write_line(sd, &res);
	return;
}


int main(int argc, char **argv) {
	// Se comprueba el número de args
	if (argc != 3 || strcmp(argv[1], "-p") != 0) {
		printf("Error de formato: ./servidor -p PUERTO\n");
		return -1;
	}
	
	// Se checkea el puerto
	int puerto = atoi(argv[2]);
	if (puerto <= 0) {
		printf("Error: mal puerto\n");
		return -1;
	}

	// Conseguir la IP
	char hostname[1024];
	if (gethostname(hostname, 1024) == -1) {
		printf("Error: no se consiguió el nombre local\n");
		return -1;
	}
	
	struct hostent *hp;
	struct in_addr in;
	
	hp = gethostbyname(hostname);
	if (hp == NULL) {
		printf("Error: gethostbyname NULL\n");
		return -1;
	}
	
	memcpy(&in.s_addr, *(hp->h_addr_list), sizeof(in.s_addr));
	
	// Mensaje de inicialización
	printf("s> init server %s:%d\n", inet_ntoa(in), puerto);

	// Se crean los attr de los threads
	pthread_attr_t attr_thr;
	pthread_attr_init(&attr_thr);
	pthread_attr_setdetachstate(&attr_thr, PTHREAD_CREATE_DETACHED);
	
	// Se le da valor al path de las tuplas
	abs_path = realpath(rel_path, NULL);

	// Creación del socket del servidor
	int socket_server = create_server_socket(puerto, SOCK_STREAM);
	if (socket_server < 0) {
		return -1;
	}
	char buffer[16];
	memset(buffer, cero, sizeof(buffer));
	int new_sd;
	
	// Bucle de espera a las peticiones
	while(1) {
		new_sd = accept_server(socket_server);
		
		// Si se acepta correctamente la petición
		if (new_sd >= 0) {
			
			if (read_line(new_sd, buffer, 16) == -1) {
				printf("Error: no se lee la operación\n");
				close(new_sd);
			}
			
			// Si se lee bien la operación
			else {

				printf("Conexión aceptada: procesando %s\n", buffer);
				
				pthread_t thread;
				// Llamada a las funciones
                if (strcmp(buffer, "REGISTER") == 0) {
                    pthread_create(&thread, &attr_thr, (void*)register_server, (void*)&new_sd);
                }
                else if (strcmp(buffer, "UNREGISTER") == 0) {
                    pthread_create(&thread, &attr_thr, (void*)unregister_server, (void*)&new_sd);
                }
                else if (strcmp(buffer, "CONNECT") == 0) {
                    pthread_create(&thread, &attr_thr, (void*)connect_server, (void*)&new_sd);
                }
                else if (strcmp(buffer, "PUBLISH") == 0) {
                    pthread_create(&thread, &attr_thr, (void*)publish_server, (void*)&new_sd);
                }
                else if (strcmp(buffer, "DELETE") == 0) {
                    pthread_create(&thread, &attr_thr, (void*)delete_server, (void*)&new_sd);
                }
                else if (strcmp(buffer, "LIST_USERS") == 0) {
                    pthread_create(&thread, &attr_thr, (void*)list_users_server, (void*)&new_sd);
                }
                else if (strcmp(buffer, "LIST_CONTENT") == 0) {
                    pthread_create(&thread, &attr_thr, (void*)list_content_server, (void*)&new_sd);
                }
                else if (strcmp(buffer, "DISCONNECT") == 0) {
                    pthread_create(&thread, &attr_thr, (void*)disconnect_server, (void*)&new_sd);
                }

				pthread_mutex_lock(&mutex);
				while (copia == 0){
					pthread_cond_wait(&copiado, &mutex);
				}
				copia = 0;
				pthread_mutex_unlock(&mutex);
			}
		}
	}
	return 0;
}
