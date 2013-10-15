#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <openssl/ssl.h>
#include <signal.h>

#include "server.h"
#include "common.h"

/***********************************************/
int password_cb(char *buffer, int size, int rwflag, void *password)
/***********************************************/
{
	static char *localPassword = NULL;

	if (localPassword == NULL) 
	{
		localPassword = password;
		return 0;
	}

	if (buffer == NULL)
		return -1;
	
	strcpy(buffer, (char *)(localPassword));

	return (strlen(buffer));
}

/***********************************************/
SSL_CTX *initialize_ctx(char *keyfile, char *password, char *ca_list)
/***********************************************/
{
	SSL_METHOD *meth;
	SSL_CTX *ctx;
	BIO *bio_err = 0;

	if(!bio_err) 
	{
		SSL_library_init();
		SSL_load_error_strings();

		bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);
	}

	meth = SSLv23_method();
	ctx = SSL_CTX_new(meth);

	if(!(SSL_CTX_use_certificate_chain_file(ctx, keyfile))) 
	{
		toslog("Can't read certificate file");
		return NULL;
	}

	SSL_CTX_set_default_passwd_cb(ctx, password_cb);

	if(!(SSL_CTX_use_PrivateKey_file(ctx, keyfile, SSL_FILETYPE_PEM))) 
	{
		toslog("Can't read key file");
		return NULL;
	}

	if(!(SSL_CTX_load_verify_locations(ctx, ca_list, 0))) 
	{
		toslog("Can't read CA list");

		return NULL;
	}

#if (OPENSSL_VERSION_NUMBER < 0x00905100L)
	SSL_CTX_set_verify_depth(ctx, 1);
#endif

	return ctx;
}

/***********************************************/
void destroy_ctx(SSL_CTX *ctx) 
/***********************************************/
{
	SSL_CTX_free(ctx);
}

/***********************************************/
short load_dh_params(SSL_CTX *ctx, char *file) 
/***********************************************/
{
	DH *ret = 0;
	BIO *bio;

	if ((bio = BIO_new_file(file, "r")) == NULL) 
	{
		toslog("Blad BIO_new_file");
		return -1;
	}

	ret = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);

	BIO_free(bio);

	if (SSL_CTX_set_tmp_dh(ctx, ret) < 0) 
	{
		toslog("Blad SSL_CTX_set_tmp_dh");
		return -2;
	}

	return 0;
}

/***********************************************/
short generate_eph_rsa_key(SSL_CTX *ctx) 
/***********************************************/
{
	RSA *rsa;

	rsa = RSA_generate_key(512, RSA_F4, NULL, NULL);

	if (!SSL_CTX_set_tmp_rsa(ctx, rsa)) 
	{
		toslog("Couldn't set RSA key");
		return -1;
	}

	RSA_free(rsa);

	return 0;
}

/***********************************************/
unsigned short plaintext_server(struct server_configuration *sc, unsigned int socket) 
/***********************************************/
{
	char *received_data;
	int request_handler_result = -1;

	if (shutdown(socket, SHUT_RD) == 0) 
	{
		if ((received_data = read_data_plaintext(socket)) == NULL) 
		{
			toslog("Blad odczytu danych.");

			return ERROR_READ_PLAINTTEXT_DATA;
		}

		request_handler_result = sc->request_handler(sc, received_data, NULL, socket);

		close(socket);

		free(received_data);
	} 
	else 
	{
		toslog("Blad shutdown dla gniazdka.");

		return ERROR_SHUTDOWN_PLAINTEXT_SOCKET;
	}

	return request_handler_result;
}

/***********************************************/
unsigned short secured_server(struct server_configuration *sc, SSL *ssl, unsigned int socket) 
/***********************************************/
{
	char *received_data;
	int request_handler_result = -1;

        if (shutdown(socket, SHUT_RD) == 0)
        {
		if ((received_data = read_data_secured(ssl, socket, sc->max_receiving_time)) == NULL) 
		{
			toslog("Blad odczytu danych.");

			return ERROR_READ_SECURED_DATA;
		}

		request_handler_result = sc->request_handler(sc, received_data, ssl, socket);
	}	
        else
        {
                toslog("Blad shutdown dla gniazdka.");

                return ERROR_SHUTDOWN_SECURED_SOCKET;
        }

	free(received_data);

	close(socket);

	SSL_free(ssl);

	return request_handler_result;
}

/***********************************************/
void set_request_handler(struct server_configuration *sc, REQUEST_HANDLER_DEFINITION) 
/***********************************************/
{
	sc->request_handler = request_handler;
}

/***********************************************/
unsigned short server(struct server_configuration *sc) 
/***********************************************/
{
	pid_t pid;
	unsigned int sock, send_sock;
	unsigned int i;
	int yes = 1;
	struct sockaddr_in addr, info;
	SSL_CTX *ctx;
	SSL *ssl;
	BIO *sbio;

	setsid ();
	umask (0);
	signal (SIGCHLD, SIG_IGN);

	if (sc->secured) 
	{
		password_cb(NULL, -1, -1, sc->certificate_password);

		if ((ctx = initialize_ctx(sc->certificate_file, sc->certificate_password, sc->ca_list)) == NULL)
			return ERROR_CTX_INIT;
	
		if (load_dh_params(ctx, sc->dh_file) < 0)
			return ERROR_LOAD_DH_PARAMS;
	}

	if (sc->deamon_mode && fork()) 
	{
		return 0;
	}

	sock = socket (AF_INET, SOCK_STREAM, 0);

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
	{
		toslog("setsockopt");
		exit(1);
	}

	addr.sin_family = AF_INET;

	if (sc->listen_address[0] == '\0')
		addr.sin_addr.s_addr = htonl (INADDR_ANY);
	else
		inet_aton(sc->listen_address, &addr.sin_addr);

	addr.sin_port = htons(sc->listen_port);
	bind (sock, (struct sockaddr *) &addr, sizeof (addr));
	i = sizeof (info);
	listen (sock, 1);

	for (;;) 
	{
		i = sizeof (info);
		send_sock = accept (sock, (struct sockaddr *) &info, &i);
		pid = fork ();

		if (pid) 
		{
			close (send_sock);
			continue;
		} 
		else 
		{
			if (sc->secured) 
			{
				sbio = BIO_new_socket(send_sock, BIO_NOCLOSE);
				ssl = SSL_new(ctx);
				SSL_set_bio(ssl, sbio, sbio);

				if(SSL_accept(ssl) <= 0) 
				{
					toslog("SSL accept error");
					return ERROR_SSL_ACCEPT;
				}

				return secured_server(sc, ssl, send_sock);
			} else
				return plaintext_server(sc, send_sock);
		}
	}

	return -1;
}
