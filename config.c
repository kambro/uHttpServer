#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/ssl.h>

#include "config.h"
#include "string_tools.h"
#include "common.h"

char *read_config(const char file_name[]) 
{
	char *data = calloc(DEFAULT_BUFFER_INIT_SIZE, sizeof(char));
	char temp_buffer[DEFAULT_BUFFER_INIT_SIZE] = "";
	size_t readed_bytes = 0;
	size_t sum_of_bytes = 0;
	FILE *file;

	if ((file = fopen(file_name, "r")) == NULL)
		return NULL;
	
	while (!feof(file) && (readed_bytes = fread(temp_buffer, sizeof(char), sizeof(char) * DEFAULT_BUFFER_INIT_SIZE, file)) > 0) 
	{
		sum_of_bytes += readed_bytes;
		strcat(data, temp_buffer);
		data[sum_of_bytes] = '\0';
	}

	fclose(file);

	return data;
}


short is_config_section(const char section_line[], config_section *cs) 
{
	char *temp_section_name;
	short result = 0; 
	char *section, *before_section, *after_section;

	long start = in_array('[', section_line);
	long stop = in_array(']', section_line);
	unsigned long counter;
	short stop_flag = 1;

	if (start < 0 || stop < 0)
		return -1;
	temp_section_name = strdup(section_line);

	temp_section_name[start] = temp_section_name[stop] = '\0';
	before_section = trim(&temp_section_name[0]);
	section = trim(&temp_section_name[start + 1]);
	after_section = trim(&temp_section_name[stop + 1]);

	counter = 0;
	while (before_section[counter] != '\0' &&
	       (stop_flag = (in_array(before_section[counter], " \t") != -1)))
		counter++;

	if (!stop_flag)
	{
		if (cs->error_description != NULL)
			cs->error_description = strdup(before_section);

		result = -2;
	}

	counter = 0;
	while (after_section[counter] != '\0' &&
	       (stop_flag = (in_array(after_section[counter], " \t") != -1)))
		counter++;

	if (!stop_flag)
	{
		if (cs->error_description != NULL)
			cs->error_description = strdup(after_section);

		result = -3;
	}

	stop_flag = counter = 0;
	while (section[counter] != '\0' &&
	       (stop_flag = (isalnum(section[counter]) || section[counter] == '_')))
		counter++;

	if (!stop_flag)
	{
		if (cs->error_description != NULL)
			cs->error_description = strdup(section);

		result = -4;
	} 
	
	if (result == 0) 
	{
		cs->section_name = strdup(section);
		cs->error_description = NULL;
	} else 
		cs->section_name = NULL;

	free(temp_section_name);

	return result;
}

unsigned short is_config_option(const char option_line[], struct config_option *co) 
{
	char *temp_option_line;
	char *option_name, *option_value;
	long option_value_sposition = in_array('=', option_line);
	unsigned long counter = 0;
	short stop_flag = 1;

	co->option_name = co->option_value = co->error_description = NULL;

	if (option_value_sposition < 0)
		return -1;

	temp_option_line = strdup(option_line);

	if (temp_option_line == NULL)
		return -2;
	
	temp_option_line[option_value_sposition] = '\0';
	
	option_name = trim(&temp_option_line[0]);
	option_value = trim(&temp_option_line[option_value_sposition + 1]);

	while (option_name[counter] != '\0' &&
	       ((stop_flag = (isalnum(option_name[counter]) || option_name[counter] == '_')) != 0))
		counter++;

	if (!stop_flag) 
	{
		co->error_description = strdup(option_name);
		free(temp_option_line);

		return -3;
	}

	co->option_name = strdup(option_name);
	co->option_value = strdup(option_value);

	free(temp_option_line);

	return 0;
}

