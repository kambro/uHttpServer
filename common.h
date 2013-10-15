#define DEFAULT_LOG_PATH 		"log/main.log"
#define DEFAULT_BUFFER_INIT_SIZE 	1024

#define LOGP_PRINT_MESSAGE		1

#define toslog(a) to_slog(a, __func__, __FILE__, __LINE__)
#define toslogp(a,b) to_slogp(a, b, __func__, __FILE__, __LINE__)

unsigned short _msg_to_slog(char *); 
unsigned short _msg_to_log(char *);
unsigned short to_slog(char *, const char[], const char [], signed long); 
unsigned short _msg_to_slog_p(char *, unsigned short); 
unsigned short _msg_to_log_p(char *, unsigned short); 
unsigned short to_slog_p(char *, unsigned short, const char[], const char [], signed long); 
int is_file_exist(char *);
int send_data(unsigned int, SSL *, char *, int);
short send_file(char *, unsigned int, SSL *);
unsigned long send_executed_command_data(char *, unsigned int, SSL *);
char *read_data_plaintext(unsigned int);
char *read_data_secured(SSL *, unsigned int, unsigned short);
char *read_data(unsigned int, SSL *, unsigned short); 
