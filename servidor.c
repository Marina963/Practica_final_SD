#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "comm.h"

#define PATH_MAX 4096
#define cero 0

// Paths
const char *rel_path_data="./users_data";
char *abs_path_data;

const char *rel_path_connected="./users_connected";
char *abs_path_connected;

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

void get_userdata_path(char *username, char *name){
    sprintf(username, "%s/%s", abs_path_data, name);         
}

void get_user_connected_path(char *user_connected, char *name) {
    sprintf(user_connected, "%s/%s", abs_path_connected, name);
}

void register_server(int * newsd) {
    int sd = sd_copy(*newsd);
    char res = '0';
    char name[256];
    read_line(sd, name, 256);
    
    // Consigue el path del directorio
    char *user_data_path = calloc(PATH_MAX, sizeof(char));
    get_userdata_path(user_data_path, name);
	
	// Crea el directorio
	if (mkdir(user_data_path, 0755) != 0) {
		
		// Comprueba que no exista ya
		if (errno == EEXIST) {
			res = '1';
		}
		else {
			res = '2';
		}
	}
    
    // Devuelve el valor al cliente
    write_line(sd, &res);
    
    // Libera memoria y el socket
    close(sd);
    free(user_data_path);
	return;
}

void unregister_server(int * newsd) {
	int sd = sd_copy(*newsd);
    char res = '0';
    char name[256];
    read_line(sd, name, 256);
    
    // Consigue el path del directorio
    char *user_data_path = calloc(PATH_MAX, sizeof(char));
    get_userdata_path(user_data_path, name);

	// Abre el directorio del usuario
	DIR *dir = opendir(user_data_path);
	if (errno == ENOENT) {
		res = '1';
		write_line(sd, &res);
		
		// Libera memoria y el socket
		close(sd);
		free(user_data_path);
	}
	struct dirent* userfiles;
	
	// Borra los ficheros del usuario
	char *file_name;
	while ((userfiles = readdir(dir)) != NULL) {
		
		// Si el objeto no es un directorio
		if (strcmp(userfiles->d_name, ".") != 0 && strcmp(userfiles->d_name, "..") != 0) {
			
			// Se reserva espacio para el nombre del fichero y se obtiene su path absoluto
			file_name = calloc(PATH_MAX, sizeof(char));
			sprintf(file_name, "%s/%s", user_data_path, userfiles->d_name);
			
			// Se borra el fichero, si hay algún error, se actualiza la respuesta
			if (remove(file_name) == -1) {	
				perror("");
				res = '2';
			}
			
			// Se libera el espacio dinámico
			free(file_name);
		}
	}

	// Borra el directorio
    if (rmdir(user_data_path) != 0) {
		res = '2';    	
   	}
   	
   	// Devuelve el valor al cliente
    write_line(sd, &res);
    
    // Libera memoria y el socket
    close(sd);
    free(user_data_path);
	return;
}

void connect_server(int * newsd) {
	int sd = sd_copy(*newsd);
    char res = '0';
    char name[256];
    read_line(sd, name, 256);
   
    char port[8];
    read_line(sd, port, 8);
    
    char *user_data_path = calloc(PATH_MAX, sizeof(char));
    get_userdata_path(user_data_path, name);

    if(access(user_data_path, F_OK) != 0){
        res = '1';
        write_line(sd, &res);
        free(user_data_path);
        return;
    }

    char *user_connected_path = calloc(PATH_MAX, sizeof(char));
    get_user_connected_path(user_connected_path, name);

    if (access(user_connected_path, F_OK) == 0) {
        res = '2';
        write_line(sd, &res);
        free(user_data_path);
        free(user_connected_path);
        return ;
    }

    FILE * userfile;
    userfile = fopen(user_connected_path, "w+");
    if (userfile == NULL) {
        res = '3';
        write_line(sd, &res);
        free(user_data_path);
        free(user_connected_path);
        return;
    }


    if (fprintf(userfile, "%s\n", port) < 0 ) {res = '3';}

    write_line(sd, &res);
    fclose(userfile);
    close(sd);
    free(user_data_path);
    free(user_connected_path);
	return;
}

void publish_server(int * newsd) {
	// Recibe nombre de usuario, nombre de fichero y descripción
	int sd = sd_copy(*newsd);
    char res = '0';
    char name[256];
    read_line(sd, name, 256);
    
    char file_name[256];
    read_line(sd, file_name, 256);
    
    char descr[256];
    read_line(sd, descr, 256);
    
    // Comprueba que el usuario esté registrado
    char *user_data_path = calloc(PATH_MAX, sizeof(char));
    get_userdata_path(user_data_path, name);

    if(access(user_data_path, F_OK) != 0){
        res = '1';
        write_line(sd, &res);
        free(user_data_path);
        return;
    }

	// Comprueba que el usuario esté conectado
	char *user_connected_path = calloc(PATH_MAX, sizeof(char));
    get_user_connected_path(user_connected_path, name);

    if (access(user_connected_path, F_OK) != 0) {
        res = '2';
        write_line(sd, &res);
        free(user_data_path);
        free(user_connected_path);
        return ;
    }
    
    // Comprueba que el fichero no exista ya
    char *userfile = calloc(PATH_MAX, sizeof(char));
    sprintf(userfile, "%s/%s", user_data_path, file_name);
    if (access(userfile, F_OK) == 0) {
    	res = '3';
    	write_line(sd, &res);
    	free(user_data_path);
    	free(user_connected_path);
    	free(userfile);
    	return ;
    }
    
    // Escribe la descripción en el fichero
    FILE * fd;
    fd = fopen(userfile, "w+");
    if (fd == NULL) {
    	res = '4';
    	write_line(sd, &res);
    	free(user_data_path);
    	free(user_connected_path);
    	free(userfile);
    	return ;
    }
    
    if (fprintf(fd, "%s\n", descr) < 0) {
    	res = '4';
    	write_line(sd, &res);
    	free(user_data_path);
    	free(user_connected_path);
    	free(userfile);
    	return ;
    }
    
    // Se envía la respuesta
    write_line(sd, &res);
    
    // Se libera memoria, el socket y el fichero
    free(user_data_path);
    free(user_connected_path);
    free(userfile);
    close(sd);
    fclose(fd);	
	return;
}

void delete_server(int * newsd) {
	int sd = sd_copy(*newsd);
    char res = '0';
    char name[256];
    read_line(sd, name, 256);
    
    char file_name[256];
    read_line(sd, file_name, 256);
    
    // Comprueba que el usuario esté registrado
    char *user_data_path = calloc(PATH_MAX, sizeof(char));
    get_userdata_path(user_data_path, name);

    if(access(user_data_path, F_OK) != 0){
        res = '1';
        write_line(sd, &res);
        free(user_data_path);
        return;
    }

	// Comprueba que el usuario esté conectado
	char *user_connected_path = calloc(PATH_MAX, sizeof(char));
    get_user_connected_path(user_connected_path, name);

    if (access(user_connected_path, F_OK) != 0) {
        res = '2';
        write_line(sd, &res);
        free(user_data_path);
        free(user_connected_path);
        return ;
    }
    
	// Comprueba que el fichero exista
    char *userfile = calloc(PATH_MAX, sizeof(char));
    sprintf(userfile, "%s/%s", user_data_path, file_name);
    if (access(userfile, F_OK) != 0) {
    	res = '3';
    	write_line(sd, &res);
    	free(user_data_path);
    	free(user_connected_path);
    	free(userfile);
    	return ;
    }
    
    // Borra el fichero
    if (remove(userfile) == -1) {
    	res = '4';
    	write_line(sd, &res);
    	free(user_data_path);
    	free(user_connected_path);
    	free(userfile);
    	return ;
    }
	
    // Devuelve el valor al cliente
    write_line(sd, &res);
    
    // Libera memoria socket
    free(user_data_path);
    free(user_connected_path);
    free(userfile);
    close(sd);
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
    char name[256];
    read_line(sd, name, 256);
    printf("Nombre recibido: %s\n", name);
    
    // Checkea a ver si el usuario existe
    char *user_data_path = calloc(PATH_MAX, sizeof(char));
    get_userdata_path(user_data_path, name);

    if(access(user_data_path, F_OK) != 0){
        res = '1';
        write_line(sd, &res);
        free(user_data_path);
        return;
    }
    
    // Checkea a ver si el usuario no está conectado
    char *user_connected_path = calloc(PATH_MAX, sizeof(char));
    get_user_connected_path(user_connected_path, name);

    if (access(user_connected_path, F_OK) != 0) {
        res = '2';
        write_line(sd, &res);
        free(user_data_path);
        free(user_connected_path);
        return ;
    }
    
    // Borra el fichero del usuario
    DIR *dir = opendir(abs_path_connected);
    struct dirent* users;
    while((users = readdir(dir)) != NULL){
        if(strcmp(users->d_name, name) == 0){
            if(remove(user_connected_path) == -1){
                res = '3';
                write_line(sd, &res);
                free(user_data_path);
                free(user_connected_path);
                return;
            } 
        }
    }
    
    // Manda la respuesta
    write_line(sd, &res);
    
    // Libera memoria y socket
    free(user_data_path);
    free(user_connected_path);
	close(sd);
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
	
	// Se le da valor a los paths
	abs_path_data = realpath(rel_path_data, NULL);
    abs_path_connected = realpath(rel_path_connected, NULL);
	
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
