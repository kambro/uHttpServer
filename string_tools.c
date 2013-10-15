#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <openssl/ssl.h>

#include "string_tools.h"

/***********************************************/
char *sprintfalloc(const char *format, ...) 
/***********************************************/
{
    int ret;
    size_t size = 100;
    char *str = malloc(size);

    if (!str)
	return NULL;

    for(;;) {
	va_list ap;
	char *tmp;

	va_start(ap, format);
	ret = vsnprintf(str, size, format, ap);
	va_end(ap);

	if (ret < size)
	    break;

	tmp = realloc(str, (size_t) ret + 1);

	if (!tmp) {
	    ret = -1;
	    break;
	} else {
	    str = tmp;
	    size = (size_t) ret + 1;
	}
    }

    if (ret < 0) {
	free(str);
	str = 0;
    } else if (size - 1 > ret) {
	char *tmp = realloc(str, (size_t) ret + 1);

	if (tmp)
	    str = tmp;
    }

    return str;
}

/***********************************************/
char *trim(char *b)
/***********************************************/
{
	char *e = strrchr(b, '\0');

	while (b < e && *b == ' ')
		++b;

	while (e > b && *(e - 1) == ' ')
		--e; 

	*e = '\0';

	return b;
}

/***********************************************/
long in_array(char c, const char text[]) 
/***********************************************/
{
	unsigned long counter = 0;

	if (!strlen(text))
		return -1;

	while (text[counter] != '\0') 
	{
		if (c == text[counter])
			return counter;

		counter++;
	}

	return -1;
}

/***********************************************/
long count_words(char *text, char *separators) 
/***********************************************/
{ 
	unsigned long countedParts = 0;
	short incCounter = 0;
	int counter = 0;

	if (!strlen(text))
		return -1;

	while (text[counter] != '\0') 
	{
		if (in_array(text[counter], separators) > -1 ||
		    (in_array(text[counter + 1], separators) > -1 && in_array(text[counter], separators) > -1)
		   ) {
		    incCounter = 1;
		} else if (incCounter) {
		    countedParts++;
		    incCounter = 0;
		}

		counter++;
	}

	return countedParts;
}

/***********************************************/
unsigned long count_lines(char *text) 
/***********************************************/
{
	unsigned long counter = 0;

	while (*text != '\0')
	{
		if (*text == '\n' || 
		    strncmp(text, "\r\n", 2) == 0)
			counter++;

		text++;
	}

	return counter == 0 ? 0 : counter + 1;
} 

/***********************************************/
char *strnduplicate(char *text, unsigned long length)
/***********************************************/
{
	char *result = calloc(length + 1, sizeof(char));
	
	strncpy(result, text, length);
	result[length] = '\0';

	return result;
}
/***********************************************/
char *getLineByIndex(unsigned long index, char *text)
/***********************************************/
{
	unsigned long prevPosition = 0; 
	unsigned long currPosition = 0; 
	unsigned long counter = 0;
	short eos_flag = 0;

	while (index >= 0)
	{ 
		if (eos_flag && currPosition == prevPosition)
			return NULL; 

		if (index + 1 == counter)
			return strndup(&text[prevPosition], (currPosition - prevPosition));

		if (text[currPosition] == '\n' || 
		    (eos_flag = (text[currPosition] == '\0')) || 
		    strncmp(&text[currPosition], "\r\n", 2) == 0)
		{
			counter++;

			if (counter <= index)
				prevPosition = currPosition + 1;
			else
				continue;
		}
 
		currPosition++;
	}

	return NULL;
}

/***********************************************/
short get_word_by_index(unsigned long index, char *text, char *word, char *separators) 
/***********************************************/
{
	int countedParts = 0;
	short incCounter = 0;
	int startPosition = -1;
	int counter = 0;
	int length;
	char *tempText = text;

	if (!strlen(text))
		return -1;

	if (index < 0)
		return -2;

	do {
		if (text[counter] == '\0' || in_array(text[counter], separators) > -1 ||
		    (in_array(text[counter + 1], separators) > -1 && in_array(text[counter], separators) > -1)
		   ) {
			if (startPosition != -1 && index == countedParts) {
				length =  counter - startPosition;
				
				word = calloc(length + 1, sizeof(char));
				memcpy(word, &text[startPosition], length);

				word[length] = '\0';

				return startPosition;
			}

		incCounter = 1;
		startPosition = -1;
		} else {
			if (startPosition == -1)
				startPosition = counter;

			if (incCounter) {
				countedParts++;
				incCounter = 0;
			}
		} 
	} while (tempText[counter++] != '\0'); 

	return -3;
}

/***********************************************/
int get_word_index(char *word, char *query, char *separators) 
/***********************************************/
{
	int countedWords = count_words(query, separators);
	int index;
	char *temp;

	for (index = 0; index < countedWords; index++) 
	{
		get_word_by_index(index, query, temp, separators);

		if (strcmp(word, temp) == 0) {
			free(word);
			return index;
		}
	}

	return -1;
}

/***********************************************/
char *get_section(char *query, char *sWord, char *eWord) 
/***********************************************/
{
	char *sIndexStr, *eIndexStr;
	char *temp;
	int startPosition, endPosition;
	int selectLength;
	int sWordIndex, sWordLength, eWordIndex;

	sWordIndex = get_word_index(sWord, query, " ");
	eWordIndex = get_word_index(eWord, query, " ");
	sWordLength = strlen(sWord);

	if (sWordIndex == -1 || eWordIndex == -1)
		return NULL;
 
	endPosition = get_word_by_index(eWordIndex, query, sIndexStr, " ");
	free(sWord);

	startPosition = get_word_by_index(sWordIndex, query, eIndexStr, " ");
	free(eWord);

	selectLength = endPosition - startPosition - sWordLength;

	if (endPosition != -1 && endPosition != -1) 
	{
		temp = (char *) calloc(selectLength, sizeof(char));
		memcpy(temp, &query[startPosition + sWordLength], selectLength);
		temp[selectLength - 1] = '\0';	

		return temp;
	}

    return NULL;
}

/***********************************************/
short isalnum_str(const char text[])
/***********************************************/
{
	unsigned long counter = 0;
	short flag = 0;

	while (text[counter] != '\0' && (flag = isalnum(text[counter])))
		if (flag)
			return -1;

	return 0;
}

/***********************************************/
short is_empty(const char text[])
/***********************************************/
{
	return strlen(text) == 0;
}
