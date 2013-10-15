#include <openssl/ssl.h>

#define SERVER_PLAINTEXT     			0
#define SERVER_SECURED 				1
#define SERVER_DEAMON_MODE      		1
#define SERVER_STAND_ALONE      		0

#define ERROR_CTX_INIT 				-1
#define ERROR_LOAD_DH_PARAMS    		-2
#define ERROR_SSL_ACCEPT       			-3
#define ERROR_READ_PLAINTTEXT_DATA      	-4
#define ERROR_READ_SECURED_DATA         	-4
#define ERROR_SHUTDOWN_PLAINTEXT_SOCKET 	-5
#define ERROR_SHUTDOWN_SECURED_SOCKET   	-6

#define REQUEST_HANDLER_PARAMS struct server_configuration *sc, char *request, SSL *ssl, int socket
#define REQUEST_HANDLER_DEFINITION unsigned short (*request_handler)(REQUEST_HANDLER_PARAMS)

typedef struct server_configuration {
	char server_name[256];
	unsigned short secured;
	unsigned short deamon_mode;
	unsigned int listen_port;
	unsigned short max_receiving_time;
	char listen_address[15];
	char certificate_file[256];
	char certificate_password[256];
	char dh_file[256];
	char ca_list[256];

	REQUEST_HANDLER_DEFINITION;
} server_configuration;

int password_cb(char *, int, int, void *);
SSL_CTX *initialize_ctx(char *, char *, char *); 
void destroy_ctx(SSL_CTX *); 
short load_dh_params(SSL_CTX *, char *);
short generate_eph_rsa_key(SSL_CTX *); 
unsigned short plaintext_server(struct server_configuration *, unsigned int);
unsigned short secured_server(struct server_configuration *, SSL *, unsigned int); 
void set_request_handler(struct server_configuration *, REQUEST_HANDLER_DEFINITION); 
unsigned short server(struct server_configuration *); 

