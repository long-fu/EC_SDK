/* iksemel (XML parser for Jabber)
** Copyright (C) 2000-2003 Gurer Ozen
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#include "common.h"
#include "iksemel.h"

/*****  malloc wrapper  *****/

static void *(*my_malloc_func)(size_t size);
static void (*my_free_func)(void *ptr);

void * ICACHE_FLASH_ATTR
iks_malloc(size_t size)
{
	// if (my_malloc_func)
	// 	return my_malloc_func(size);
	// else
	// 	return malloc(size);
}

void ICACHE_FLASH_ATTR
iks_free(void *ptr)
{
	// if (my_free_func)
	// 	my_free_func(ptr);
	// else
	// 	free(ptr);
}

void ICACHE_FLASH_ATTR
iks_set_mem_funcs(void *(*malloc_func)(size_t size), void (*free_func)(void *ptr))
{
	my_malloc_func = malloc_func;
	my_free_func = free_func;
}

/*****  NULL-safe Functions  *****/

char * ICACHE_FLASH_ATTR
iks_strdup(const char *src)
{
	if (src)
		return strdup(src);
	return NULL;
}

char *ICACHE_FLASH_ATTR
iks_strcat(char *dest, const char *src)
{
	size_t len;

	if (!src)
		return dest;

	len = os_strlen(src);
	os_memcpy(dest, src, len);
	dest[len] = '\0';
	return dest + len;
}

int ICACHE_FLASH_ATTR
iks_strcmp(const char *a, const char *b)
{
	if (!a || !b)
		return -1;
	return os_strcmp(a, b);
}

int ICACHE_FLASH_ATTR
iks_strcasecmp(const char *a, const char *b)
{
	if (!a || !b)
		return -1;
	return strcasecmp(a, b);
}

int ICACHE_FLASH_ATTR
iks_strncmp(const char *a, const char *b, size_t n)
{
	if (!a || !b)
		return -1;
	return os_strncmp(a, b, n);
}

int ICACHE_FLASH_ATTR
iks_strncasecmp(const char *a, const char *b, size_t n)
{
	if (!a || !b)
		return -1;
	return strncasecmp(a, b, n);
}

size_t ICACHE_FLASH_ATTR
iks_strlen(const char *src)
{
	if (!src)
		return 0;
	return os_strlen(src);
}

/*****  XML Escaping  *****/

char * ICACHE_FLASH_ATTR
iks_escape(ikstack *s, char *src, size_t len)
{
	char *ret;
	int i, j, nlen;

	if (!src || !s)
		return NULL;
	if (len == -1)
		len = os_strlen(src);

	nlen = len;
	for (i = 0; i < len; i++)
	{
		switch (src[i])
		{
		case '&':
			nlen += 4;
			break;
		case '<':
			nlen += 3;
			break;
		case '>':
			nlen += 3;
			break;
		case '\'':
			nlen += 5;
			break;
		case '"':
			nlen += 5;
			break;
		}
	}
	if (len == nlen)
		return src;

	ret = iks_stack_alloc(s, nlen + 1);
	if (!ret)
		return NULL;

	for (i = j = 0; i < len; i++)
	{
		switch (src[i])
		{
		case '&':
			os_memcpy(&ret[j], "&amp;", 5);
			j += 5;
			break;
		case '\'':
			os_memcpy(&ret[j], "&apos;", 6);
			j += 6;
			break;
		case '"':
			os_memcpy(&ret[j], "&quot;", 6);
			j += 6;
			break;
		case '<':
			os_memcpy(&ret[j], "&lt;", 4);
			j += 4;
			break;
		case '>':
			os_memcpy(&ret[j], "&gt;", 4);
			j += 4;
			break;
		default:
			ret[j++] = src[i];
		}
	}
	ret[j] = '\0';

	return ret;
}

char * ICACHE_FLASH_ATTR
iks_unescape(ikstack *s, char *src, size_t len)
{
	int i, j;
	char *ret;

	if (!s || !src)
		return NULL;
	if (!strchr(src, '&'))
		return src;
	if (len == -1)
		len = os_strlen(src);

	ret = iks_stack_alloc(s, len + 1);
	if (!ret)
		return NULL;

	for (i = j = 0; i < len; i++)
	{
		if (src[i] == '&')
		{
			i++;
			if (os_strncmp(&src[i], "amp;", 4) == 0)
			{
				ret[j] = '&';
				i += 3;
			}
			else if (os_strncmp(&src[i], "quot;", 5) == 0)
			{
				ret[j] = '"';
				i += 4;
			}
			else if (os_strncmp(&src[i], "apos;", 5) == 0)
			{
				ret[j] = '\'';
				i += 4;
			}
			else if (os_strncmp(&src[i], "lt;", 3) == 0)
			{
				ret[j] = '<';
				i += 2;
			}
			else if (os_strncmp(&src[i], "gt;", 3) == 0)
			{
				ret[j] = '>';
				i += 2;
			}
			else
			{
				ret[j] = src[--i];
			}
		}
		else
		{
			ret[j] = src[i];
		}
		j++;
	}
	ret[j] = '\0';

	return ret;
}
