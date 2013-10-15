#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <openssl/ssl.h>
#include <sys/stat.h>

#include "common.h"
#include "string_tools.h"

/***********************************************/
unsigned short _msg_to_slog(char *message_to_log) 
/***********************************************/
{
	FILE *file;
	char prefix[64] = "";

	if ((file = fopen(DEFAULT_LOG_PATH, "a+")) == NULL)
		return -1;

	if (chmod(DEFAULT_LOG_PATH, 0666) == -1)
		return -1;

	fprintf(file, "%s: %s\n", prefix, message_to_log);

	fclose(file);

	return 0;
}

/***********************************************/
unsigned short to_slog(char *message_to_log, const char func_name[], const char file_name[], signed long line_no) 
/***********************************************/
{
	char *message = sprintfalloc("[%s:%s][%ld]: %s", file_name, func_name, line_no, message_to_log);

	if (message == NULL)
		return -1;

	_msg_to_slog(message);

	free(message);
	return 0;
}

/***********************************************/
unsigned short _msg_to_slog_p(char *message_to_log, unsigned short log_option) 
/***********************************************/
{
	FILE *file;
	char prefix[64] = "";

	if (chmod(DEFAULT_LOG_PATH, 0666) == -1)
		return -1;

	if ((file = fopen(DEFAULT_LOG_PATH, "a+")) == NULL)
		return -1;

	if (chmod(DEFAULT_LOG_PATH, 0666) == -1)
		return -1;

	if (log_option & LOGP_PRINT_MESSAGE)
	{
		printf("%s: %s\n", prefix, message_to_log);
	}

	fprintf(file, "%s: %s\n", prefix, message_to_log);

	fclose(file);

	return 0;
}

/***********************************************/
unsigned short to_slog_p(char *message_to_log, unsigned short log_option, const char func_name[], const char file_name[], signed long line_no) 
/***********************************************/
{
	char *message = sprintfalloc("[%s:%s][%ld]: %s", file_name, func_name, line_no, message_to_log);

	if (message == NULL)
		return -1;

	_msg_to_slog_p(message, log_option);

	free(message);
	return 0;
}

/***********************************************/
int is_file_exist(char *fname) 
/***********************************************/
{
	FILE *f;

	f = fopen(fname, "r");

	if (f == NULL) 
		return(-1);

	fclose(f);

	return 1;
}

/***********************************************/
int send_data(unsigned int socket, SSL *ssl, char *buf, int slen) 
/***********************************************/
{
	size_t total = 0;
	size_t bytesleft = slen;
	size_t len = slen;
	size_t n, result;
	BIO *io, *ssl_bio;

	if (ssl != NULL) {
		io = BIO_new(BIO_f_buffer());
		ssl_bio = BIO_new(BIO_f_ssl());
		BIO_set_ssl(ssl_bio, ssl, BIO_CLOSE);
		BIO_push(io, ssl_bio);

		if ((result = BIO_puts(io, buf)) <= 0)
			return result;

		if ((result = BIO_flush(io)) <= 0)
			return result;

		return 0;
	}

	while(total < len) 
	{
		n = send(socket, buf + total, bytesleft, 0);

		if (n == -1)
			break;

		total += n;
		bytesleft -= n;
	}

	len = total;
	return n == -1 ? -1 : 0;
} 

/***********************************************/
short send_file(char *fname, unsigned int socket, SSL *ssl) 
/***********************************************/
{
	FILE *f;
	char bufr[512];
	size_t rsize;

	if ((f = fopen(fname, "r")) == NULL)
		return -1;

	do 
	{
		rsize = fread(bufr, 1, 512, f);
		send_data(socket, ssl, bufr, rsize);
	} while (rsize > 0);

	fclose(f);

	return 0;
}

/***********************************************/
size_t send_executed_command_data(char *command, unsigned int socket, SSL *ssl)
/***********************************************/
{
	FILE *po;
	char data[512];
	size_t readed_bytes;
	size_t sum_of_readed_bytes = 0;

	if ((po = popen(command, "r")) == NULL)
		return -1;

	while ((readed_bytes = fread(data, sizeof(char), sizeof(data), po)) != 0) 
	{
		data[readed_bytes] = '\0';
		sum_of_readed_bytes += readed_bytes;
		send_data(socket, ssl, (char *)data, readed_bytes);
		toslog(data);
		memset(data, 0, sizeof(data));
	};

	if (pclose(po) == -1) 
		return -2;

	return sum_of_readed_bytes;
}

/***********************************************/
char *read_data_plaintext(unsigned int socket) 
/***********************************************/
{
	char *received_data = calloc(DEFAULT_BUFFER_INIT_SIZE, sizeof(char));
	char temp_buffer[DEFAULT_BUFFER_INIT_SIZE] = "";
	size_t sum_of_bytes = 0, bytes_received;

	while ((bytes_received = recv(socket, &temp_buffer, sizeof(char) * DEFAULT_BUFFER_INIT_SIZE, 0)) > 0) 
	{
		sum_of_bytes += bytes_received;
		received_data = realloc(received_data, sum_of_bytes + 1);
		strcat(received_data, temp_buffer);
		received_data[sum_of_bytes] = '\0';
	}

	return received_data;
}

/***********************************************/
char *read_data_secured(SSL *ssl, unsigned int socket, unsigned short max_repeat_no) 
/***********************************************/
{
	char *received_data = calloc(DEFAULT_BUFFER_INIT_SIZE, sizeof(char));
	char temp_buffer[DEFAULT_BUFFER_INIT_SIZE] = "";
	size_t bytes_received, sum_of_bytes = 0;
	BIO *io, *ssl_bio;
	static unsigned short func_level = 0;
	
	if (func_level >= max_repeat_no)
		return NULL;
	else if (func_level)
		sleep(1);

	func_level++;

	io = BIO_new(BIO_f_buffer());
	ssl_bio = BIO_new(BIO_f_ssl());
	BIO_set_ssl(ssl_bio, ssl, BIO_CLOSE);
	BIO_push(io, ssl_bio);

	while (1) 
	{
		bytes_received = BIO_gets(io, temp_buffer, sizeof (char) * DEFAULT_BUFFER_INIT_SIZE);

		if (SSL_get_error(ssl, bytes_received) == SSL_ERROR_NONE)
			sum_of_bytes += bytes_received;
		else 
		{
			if (shutdown(socket, SHUT_RD) == 0 && sum_of_bytes == 0) {
				return read_data_secured(ssl, socket, max_repeat_no);
			} else return received_data;
		}

		received_data = realloc(received_data, sum_of_bytes + 1);
		strcat(received_data, temp_buffer);
		received_data[sum_of_bytes] = '\0';
	}

	return received_data;
}

/***********************************************/
char *read_data(unsigned int socket, SSL *ssl, unsigned short max_repeat_no) 
/***********************************************/
{
	if (ssl != NULL)
		return read_data_secured(ssl, socket, max_repeat_no);
	else 
		return read_data_plaintext(socket); 
}
