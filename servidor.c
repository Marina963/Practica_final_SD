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
#include "rpc/info.h"

#define PATH_MAX 4096
#define cero 0

// Paths
const char *rel_path_data="./users_data";
char *abs_path_data;

const char *rel_path_connected="./users_connected";
char *abs_path_connected;

const char *rel_path_files="./users_files";
char *abs_path_files;

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
	// Función que consigue el 
    sprintf(username, "%s/%s", abs_path_data, name);         
}

void get_user_connected_path(char *user_connected, char *name) {
    sprintf(user_connected, "%s/%s", abs_path_connected, name);
}

void print_data(char *name, char *op, char *f_name, char *date){
    CLIENT *clnt;
    enum clnt_stat res;

    clnt = clnt_create ("localhost", INFO, INFOVER, "tcp");
	if (clnt == NULL) {
		clnt_pcreateerror ("localhost");
		exit (1);
	}

    struct arg datos;
    memcpy(datos.user, name, 256);
    memcpy(datos.op, op, 16);
    memcpy(datos.f_name, f_name, 256);
    memcpy(datos.date, date, 32);
    
    res = print_info_x_1(datos, NULL, clnt);

    if (res != RPC_SUCCESS) {
		clnt_perror (clnt, "call failed");
	}

	clnt_destroy (clnt);
}

void register_server(int * newsd) {
    int sd = sd_copy(*newsd);
    char res = '0';

    char date[32];
    read_line(sd, date, 32);


    char name[256];
    read_line(sd, name, 256);

    print_data(name, "REGISTER", "", date);
    
    // Consigue el path del directorio
    char *user_data_path = calloc(PATH_MAX, sizeof(char));
    get_userdata_path(user_data_path, name);
	
	char *user_files_path = calloc(PATH_MAX, sizeof(char));
	sprintf(user_files_path, "%s/%s", abs_path_files, name);
	
	// Crea el directorio
	if (mkdir(user_data_path, 0755) != 0) {
		
		// Comprueba que no exista ya
		if (errno == EEXIST) {
			res = '1';
		}
		else {
			res = '2';
		}
		
		write_line(sd, &res);
		close(sd);
		return;
	}
	
	if (mkdir(user_files_path, 0755) != 0) {
		
		// Comprueba si existía o no
		if (errno != EEXIST) {
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

    char date[32];
    read_line(sd, date, 32);

    char name[256];
    read_line(sd, name, 256);

    print_data(name, "UNREGISTER", "", date);
    
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
		return ;
	}
	struct dirent* userfiles;
	
	// Borra los ficheros del usuario
	char file_name[PATH_MAX];
	while ((userfiles = readdir(dir)) != NULL) {
		
		// Si el objeto no es un directorio
		if (strcmp(userfiles->d_name, ".") != 0 && strcmp(userfiles->d_name, "..") != 0) {
			
			// Se obtiene su path absoluto
			sprintf(file_name, "%s/%s", user_data_path, userfiles->d_name);
			
			// Se borra el fichero, si hay algún error, se actualiza la respuesta
			if (remove(file_name) == -1) {	
				perror("");
				res = '2';
			}
			
		}
	}
	
	closedir(dir);
	
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

void connect_server(struct socket_info * new_sd_info) {
	
	// Se inicializan las variables
    char ip[64];
    memcpy(ip, new_sd_info->ip, 64);
    
	int sd = sd_copy(new_sd_info->sd);
    char res = '0';

    char date[32];
    read_line(sd, date, 32);

	// Recibe los parámetros
    char name[256];
    read_line(sd, name, 256);
   
    char port[8];
    read_line(sd, port, 8);

    print_data(name, "CONNECT", "", date);
    
    
    // Se mira si el usuario está registrado
    char *user_data_path = calloc(PATH_MAX, sizeof(char));
    get_userdata_path(user_data_path, name);

    if(access(user_data_path, F_OK) != 0){
        res = '1';
        write_line(sd, &res);
        free(user_data_path);
        close(sd);
        return;
    }

	// Se mira si el usuario está conectado
    char *user_connected_path = calloc(PATH_MAX, sizeof(char));
    get_user_connected_path(user_connected_path, name);

    if (access(user_connected_path, F_OK) == 0) {
        res = '2';
        write_line(sd, &res);
        free(user_data_path);
        free(user_connected_path);
        close(sd);
        return ;
    }
	free(user_data_path);

	// Se abre el fichero del cliente
    FILE * userfile;
    userfile = fopen(user_connected_path, "w+");
    if (userfile == NULL) {
        res = '3';
        write_line(sd, &res);
        free(user_connected_path);
        close(sd);
        return;
    }

	// Se guarda la IP y el puerto
    if (fprintf(userfile, "%s\n", ip) < 0 ) {res = '3';}
    if (fprintf(userfile, "%s\n", port) < 0 ) {res = '3';}
	
    write_line(sd, &res);
    fclose(userfile);
    close(sd);
    free(user_connected_path);
	return;
}

void publish_server(int * newsd) {
	// Recibe nombre de usuario, nombre de fichero y descripción
	int sd = sd_copy(*newsd);
    char res = '0';

    char date[32];
    read_line(sd, date, 32);

    char name[256];
    read_line(sd, name, 256);
    
    char file_name[256];
    read_line(sd, file_name, 256);
    
    char descr[256];
    read_line(sd, descr, 256);
    
    print_data(name, "PUBLISH", file_name, date);

    // Comprueba que el usuario esté registrado
    char *user_data_path = calloc(PATH_MAX, sizeof(char));
    get_userdata_path(user_data_path, name);

    if(access(user_data_path, F_OK) != 0){
        res = '1';
        write_line(sd, &res);
        free(user_data_path);
        close(sd);
        return;
    }

	// Comprueba que el usuario esté conectado
	char *user_connected_path = calloc(PATH_MAX, sizeof(char));
    get_user_connected_path(user_connected_path, name);

    if (access(user_connected_path, F_OK) != 0 || strcmp(name, "") == 0) {
        res = '2';
        write_line(sd, &res);
        free(user_data_path);
        free(user_connected_path);
		close(sd);
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
    	close(sd);
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
    	close(sd);
    	return ;
    }
    
    if (fprintf(fd, "%s\n", descr) < 0) {
    	res = '4';
    	write_line(sd, &res);
    	free(user_data_path);
    	free(user_connected_path);
    	free(userfile);
    	close(sd);
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

    char date[32];
    read_line(sd, date, 32);

    char name[256];
    read_line(sd, name, 256);
    
    char file_name[256];
    read_line(sd, file_name, 256);

    print_data(name, "DELETE", file_name, date);
    
    // Comprueba que el usuario esté registrado
    char *user_data_path = calloc(PATH_MAX, sizeof(char));
    get_userdata_path(user_data_path, name);

    if(access(user_data_path, F_OK) != 0){
        res = '1';
        write_line(sd, &res);
        free(user_data_path);
        close(sd);
        return;
    }

	// Comprueba que el usuario esté conectado
	char *user_connected_path = calloc(PATH_MAX, sizeof(char));
    get_user_connected_path(user_connected_path, name);

    if (access(user_connected_path, F_OK) != 0 || strcmp(name, "") == 0) {
        res = '2';
        write_line(sd, &res);
        free(user_data_path);
        free(user_connected_path);
        close(sd);
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
    	close(sd);
    	return ;
    }
    
    // Borra el fichero
    if (remove(userfile) == -1) {
    	res = '4';
    	write_line(sd, &res);
    	free(user_data_path);
    	free(user_connected_path);
    	free(userfile);
    	close(sd);
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

    char date[32];
    read_line(sd, date, 32);

    char name[256];
    read_line(sd, name, 256);

    print_data(name, "LIST_USERS", "", date);
    
    // Comprueba que el usuario esté registrado
    char *user_data_path = calloc(PATH_MAX, sizeof(char));
    get_userdata_path(user_data_path, name);

    if(access(user_data_path, F_OK) != 0){
        res = '1';
        write_line(sd, &res);
        free(user_data_path);
        close(sd);
        return;
    }

    free(user_data_path);
    
	// Comprueba que el usuario esté conectado
	char *user_connected_path = calloc(PATH_MAX, sizeof(char));
    get_user_connected_path(user_connected_path, name);

    if (access(user_connected_path, F_OK) != 0) {
        res = '2';
        write_line(sd, &res);
        free(user_connected_path);
        close(sd);
        return ;
    }
    
    free(user_connected_path);
    
    // Abre el directorio con los ficheros
    DIR* dir = opendir(abs_path_connected);
    if (dir == NULL) {
    	res = '3';
        write_line(sd, &res);
        close(sd);
        return;
    }
    struct dirent* userfiles;
    
    // Cuenta el número de ficheros
    int file_counter = 0;
    while ((userfiles = readdir(dir)) != NULL) {
    	if (userfiles->d_type == DT_REG) {
    		file_counter++;
    	}
    }
    closedir(dir);

	// Escribe la cabecera del mensaje    
    char *users_info = calloc(400*file_counter, sizeof(char));
    sprintf(users_info, "%c\n%d\n", res, file_counter);
    
    // Abre el directorio de nuevo
    dir = opendir(abs_path_connected);
    if (dir == NULL) {
    	res = '3';
        write_line(sd, &res);
        close(sd);
        return;
    }
    
    // Inicializa las variables
    char ip[64];
    char port[8];
    char file_name[PATH_MAX];
    char temp[2048];
    FILE * fd;
    
    while ((userfiles = readdir(dir)) != NULL) {
    	
    	if (userfiles->d_type == DT_REG) {
    		
    		// Abre el fichero
    		sprintf(file_name, "%s/%s", abs_path_connected, userfiles->d_name);
    		fd = fopen(file_name, "r");
    		if (fd == NULL) {
				res = '3';
    			write_line(sd, &res);
        		close(sd);
        		return;
    		}
    		
    		// Lee el puerto
    		if (fscanf(fd, "%s", ip) < 0) {
    			res = '3';
    			write_line(sd, &res);
    			fclose(fd);
        		close(sd);
        		return;
    		}
    		
    		// Lee el puerto
    		if (fscanf(fd, "%s", port) < 0) {
    			res = '3';
    			write_line(sd, &res);
    			fclose(fd);
        		close(sd);
        		return;
    		}
    		
    		sprintf(temp, "%s\n%s\n%s\n", userfiles->d_name, ip, port);
    		strcat(users_info, temp);
    		fclose(fd);
    	}
    }
    
    write_line(sd, users_info);
    closedir(dir);
    close(sd);
	return;
}

void list_content_server(int * newsd) {
    
    // Recibe el nombre del usuario que realiza la acción y el nombre del usuario del que se quiere saber los ficheros
	int sd = sd_copy(*newsd);
    char res = '0';

    char date[32];
    read_line(sd, date, 32);

    char user[256];
    read_line(sd, user, 256);
    
    char name[256];
    read_line(sd, name, 256);

    print_data(user, "LIST_CONTENT", "", date);
    
    // Comprueba que el usuario que realiza la operación esté registrado
    char *user_data_path = calloc(PATH_MAX, sizeof(char));
    get_userdata_path(user_data_path, user);

	if(access(user_data_path, F_OK) != 0){
        res = '1';
        write_line(sd, &res);
        free(user_data_path);
        close(sd);
        return;
    }
    
    // Comprueba que el usuario que realiza la operación esté conectado
	char *user_connected_path = calloc(PATH_MAX, sizeof(char));
    get_user_connected_path(user_connected_path, user);

    if (access(user_connected_path, F_OK) != 0) {
        res = '2';
        write_line(sd, &res);
        free(user_data_path);
        free(user_connected_path);
        close(sd);
        return ;
    }
    free(user_data_path);
    free(user_connected_path);
     
    // Comprueba que el usuario que cuyo contenido se quiere conocer esté registrado
    user_data_path = calloc(PATH_MAX, sizeof(char));
    get_userdata_path(user_data_path, name);

    if(access(user_data_path, F_OK) != 0){
        res = '3';
        write_line(sd, &res);
        free(user_data_path);
        close(sd);
        return;
    }
    
    // Abre el directorio con los ficheros
    DIR* dir = opendir(user_data_path);
    if (dir == NULL) {
    	res = '4';
        write_line(sd, &res);
        free(user_data_path);
        close(sd);
        return;
    }
    struct dirent* userfiles;
    
    // Cuenta el número de ficheros
    int file_counter = 0;
    while ((userfiles = (readdir(dir))) != NULL) {
    	if (userfiles->d_type == DT_REG) {
    		file_counter++;
    	}
    }
    closedir(dir);
    
    // Lee los ficheros del usuario cuyo contenido se quiere saber
    char * file_info = calloc(file_counter*600, sizeof(char));
    char temp[600];
    dir = opendir(user_data_path);
    FILE * fd;
    char filename[PATH_MAX];
    
    // Pone la cabecera
    sprintf(file_info, "%s\n%d\n", &res, file_counter);
    
    // Lee todos los ficheros
    while ((userfiles = (readdir(dir))) != NULL) {
    	if (userfiles->d_type == DT_REG) {
    		
    		// Abre el fichero
    		sprintf(filename, "%s/%s", user_data_path, userfiles->d_name);
    		fd = fopen(filename, "r");
    		if (fd == NULL) {
    			res = '4';
    			write_line(sd, &res);
    			free(user_data_path);
    			fclose(fd);
    			close(sd);
    			return;
    		}
    		
    		// Consigue la descripción
    		char desc[256];
    		if (fscanf(fd, "%[^\n]s\n", desc) < 0) {
    			res = '4';
    			write_line(sd, &res);
    			free(user_data_path);
    			fclose(fd);
    			close(sd);
    			return;
    		}
    		
    		// Guardas la información
    		sprintf(temp, "%s\n%s\n", userfiles->d_name, desc);
    		strcat(file_info, temp);
    		fclose(fd);
    	}
    }
    
    // Se envía la respuesta
    write_line(sd, file_info);
    close(sd);
    free(user_data_path);
	return;
    
}

void disconnect_server(int * newsd) {
	int sd = sd_copy(*newsd);
    char res = '0';

    char date[32];
    read_line(sd, date, 32);

    char name[256];
    read_line(sd, name, 256);
    
    print_data(name, "DISCONNECT", "", date);
    
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
    abs_path_files = realpath(rel_path_files, NULL);
	
	// Creación del socket del servidor
	int socket_server = create_server_socket(puerto, SOCK_STREAM);
	if (socket_server < 0) {
		return -1;
	}
	char buffer[16];
	memset(buffer, cero, sizeof(buffer));
	struct socket_info new_sd_info;
	int new_sd;
	
	// Bucle de espera a las peticiones
	while(1) {
		new_sd_info = accept_server(socket_server);
		
		// Si se acepta correctamente la petición
		if (new_sd_info.code_res == 0) {
			new_sd = new_sd_info.sd;
			
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
                    pthread_create(&thread, &attr_thr, (void*)connect_server, (void*)&new_sd_info);
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
