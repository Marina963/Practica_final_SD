struct socket_info {
	int sd;
	char ip[64];
	int code_res;
};

int create_server_socket(int port, int type);
int create_client_socket(char * remote, int port);
struct socket_info accept_server(int sd);
int send_message(int sd, char *buffer, int len);
int receive_message(int sd, char *buffer, int len);
ssize_t write_line(int fd, char*buffer);
ssize_t read_line(int fd, char *buffer, size_t n);
