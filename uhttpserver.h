#define DEFAULT_ADDR 				"127.0.0.1"
#define DEFAULT_PORT 				8080
#define DEFAULT_BOOTSTRAP_SCRIPT 		"scripts/bootstrap.sh"

#define DEFAULT_CA_LIST 			"certificates/root.pem"
#define DEFAULT_CERTIFICATE_FILE 		"certificates/server.pem"
#define DEFAULT_CERTIFICATE_PASSWORD 		"password"
#define DEFAULT_DH_FILE 			"certificates/dh1024.pem"

#define DEFAULT_MAX_DATA_RECEIVED_TIME  	5

static void cb_exit(void);
unsigned short request_handler(REQUEST_HANDLER_PARAMS); 
void *run_server(void *);
int read_config_file(const char []);
