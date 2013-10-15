struct config_section {
	char *section_name;
	char *error_description;
} typedef config_section;

struct config_option {
//	unsigned short expected_value_type;
	char *option_name;
	char *option_value;
	char *error_description;
} typedef config_option;

char *read_config(const char []);
short check_value_type(char *, unsigned short);
short is_config_section(const char [], config_section *); 
unsigned short is_config_option(const char [], struct config_option *); 
