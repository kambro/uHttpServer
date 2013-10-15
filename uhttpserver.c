#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include "server.h"
#include "uhttpserver.h"
#include "common.h"
#include "config.h"
#include "string_tools.h"

#define CONFIG_FILE "config/config.ini"
#define NTHREADS 32
static server_configuration *servers_configuration[NTHREADS];
static unsigned int configured_servers_number = 0;

/***********************************************/
static void cb_exit(void) 
/***********************************************/
{
	unsigned int counter;
	for (counter = 0; counter < configured_servers_number; counter++) {
		free(servers_configuration[counter]);
	}

	printf("SIGNAL HANDLER: cb_exit();\n\n");
	toslog("SIGNAL HANDLER: cb_exit();");

	exit(0);
}

/***********************************************/
static void cb_sigpipe(int x) 
/***********************************************/
{
}

/***********************************************/
unsigned short request_handler(REQUEST_HANDLER_PARAMS) 
/***********************************************/
{
	char *bootstrap_command;
	bootstrap_command = calloc((strlen(request) + strlen(DEFAULT_BOOTSTRAP_SCRIPT)) + 64, sizeof(char));

	sprintf(bootstrap_command, "%s request='%s'", DEFAULT_BOOTSTRAP_SCRIPT, request);
	toslog(bootstrap_command);

	if (sc->secured) 
	{
		toslog("secured request_handler");
		toslog(request);

		return send_executed_command_data(bootstrap_command, socket, ssl);
	}
	else 
	{  
		toslog("plainttext request_handler");
		toslog(request);

		return send_executed_command_data(bootstrap_command, socket, NULL);
	}

	free(bootstrap_command);

	return -1;
}

/***********************************************/
void *run_server(void *ptr)
/***********************************************/
{
	struct server_configuration *sc = ptr;
	server(sc);	
}

/***********************************************/
int read_config_file(const char filename[]) 
/***********************************************/
{
	char *config = read_config(CONFIG_FILE);
	char *err, *section_name, *line;
	config_section cs;
	config_option po;
	unsigned long config_file_lines_number, counter;
	unsigned int config_section_number = 0;
	struct server_configuration *sc;
	short result = 0;
	static pthread_t thread_id[NTHREADS];

	if (config == NULL) {
		printf("nie moge otworzyc pliku");

		return -1;
	}
	
	config_file_lines_number = count_lines(config);

	for (counter = 0; counter < config_file_lines_number; counter++) {
		line = getLineByIndex(counter, config);

		if (line != NULL && !is_empty(line)) {
			if (is_config_section(line, &cs) == 0) {
				sc = servers_configuration[config_section_number] = calloc(1, sizeof(server_configuration));
				config_section_number++;
				sc->listen_port = DEFAULT_PORT;
				strcpy(sc->listen_address, DEFAULT_ADDR);
				strcpy(sc->ca_list, DEFAULT_CA_LIST);
				strcpy(sc->certificate_file, DEFAULT_CERTIFICATE_FILE);
				strcpy(sc->certificate_password, DEFAULT_CERTIFICATE_PASSWORD);
				strcpy(sc->dh_file, DEFAULT_DH_FILE);
				sc->secured = SERVER_PLAINTEXT;
				sc->max_receiving_time = DEFAULT_MAX_DATA_RECEIVED_TIME;
				sc->deamon_mode = SERVER_STAND_ALONE;
				strcpy(sc->server_name, "uHTTPServer");
				set_request_handler(sc, request_handler);
			}
			else if (is_config_option(line, &po) == 0 && config_section_number > 0)
			{
				if (strcmp("address", po.option_name) == 0) {
					strcpy(sc->listen_address, po.option_value);
				}
				else if (strcmp("port", po.option_name) == 0)
				{
					sc->listen_port = atoi(po.option_value);
				}
				else if (strcmp("secured", po.option_name) == 0)
				{
					if (strcmp("true", po.option_value)) {
						sc->secured = SERVER_SECURED;
					} else {
						sc->secured = SERVER_PLAINTEXT;
					}
				}
				else if (strcmp("bootstrap_script_path", po.option_name) == 0)
				{
				} 
				else if (strcmp("ca_file_path", po.option_name) == 0)
				{
					strcpy(sc->ca_list, po.option_value);
				}
				else if (strcmp("certificate_file", po.option_name) == 0)
				{
					strcpy(sc->certificate_file, po.option_value);
				}
				else if (strcmp("certificate_password", po.option_name) == 0)
				{
					strcpy(sc->certificate_password, po.option_value);
				}
				else if (strcmp("dh_file", po.option_name) == 0)
				{
					strcpy(sc->dh_file, po.option_value);
				}
			} else {
				result = -1;
				break;
			}
		}
	}

	for (counter = 0; counter < config_section_number; counter++) {
		pthread_create(&thread_id[counter], NULL, run_server, (void *) servers_configuration[counter]);
	}

	for (counter = 0; counter < config_section_number; counter++) {
		pthread_join(thread_id[counter], NULL);
	}

	configured_servers_number = config_section_number;

	free(config);
	return result;
}

/***********************************************/
int main (void) 
/***********************************************/
{
	signal(SIGINT, (void *) cb_exit);   /* ctrl + c 								    */
	signal(SIGTERM, (void *) cb_exit);  /* kill								    	    */
	signal(SIGHUP, (void *) cb_exit);   /* hup								    	    */
	signal(SIGPIPE, cb_sigpipe);
	read_config_file(CONFIG_FILE);

	return 0;	
}
