/* iksemel (XML parser for Jabber)
** Copyright (C) 2000-2003 Gurer Ozen
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#include "common.h"
#include "iksemel.h"
typedef uint8 u_char;
static u_char charmap[] = 
{
        '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
        '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
        '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
        '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
        '\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
        '\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
        '\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
        '\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
        '\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
        '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
        '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
        '\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
        '\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
        '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
        '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
        '\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
        '\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
        '\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
        '\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
        '\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
        '\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
        '\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
        '\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
        '\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
        '\300', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
        '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
        '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
        '\370', '\371', '\372', '\333', '\334', '\335', '\336', '\337',
        '\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
        '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
        '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
        '\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};
int ICACHE_FLASH_ATTR
os_strcasecmp(s1, s2)
const char *s1, *s2;
{
    register u_char	*cm = charmap,
    *us1 = (u_char *)s1,
    *us2 = (u_char *)s2;
    while (cm[*us1] == cm[*us2++])
    if (*us1++ == '\0') return (0);
	return (cm[*us1] - cm[*--us2]);
}

int ICACHE_FLASH_ATTR
os_strncasecmp(s1, s2, n)
const char *s1, *s2;
register int n;
{
	register u_char	*cm = charmap,
	*us1 = (u_char *)s1,
	*us2 = (u_char *)s2;
	while (--n >= 0 && cm[*us1] == cm[*us2++])
	if (*us1++ == '\0')	return(0);
	return(n < 0 ? 0 : cm[*us1] - cm[*--us2]);
}

char * ICACHE_FLASH_ATTR
os_strdup(char *src)
{
	char *p_src = NULL;
	int count = 0 ;
	char * new_addr = NULL;
    if(NULL == src)
    {
        return NULL;
    }
    
    p_src = (char*)src ;
    
    while(*p_src != '\0')
    {
         count++;
         p_src++;
    }
    new_addr  = (char *)os_malloc(sizeof(char)*(count+1));
    p_src = src;
    while(*p_src  != '\0')
    {
        *new_addr++ = *p_src++;

    }
    *new_addr = '\0';
    return (new_addr -(count));
}
/*****  malloc wrapper  *****/

static void *(*my_malloc_func)(size_t size);
static void (*my_free_func)(void *ptr);

void * ICACHE_FLASH_ATTR
iks_malloc (size_t size)
{
	if (my_malloc_func)
		return my_malloc_func (size);
	else
		return os_malloc (size);
}

void ICACHE_FLASH_ATTR
iks_free (void *ptr)
{
	if (my_free_func)
		my_free_func (ptr);
	else
		os_free (ptr);
}

void ICACHE_FLASH_ATTR
iks_set_mem_funcs (void *(*malloc_func)(size_t size), void (*free_func)(void *ptr))
{
	my_malloc_func = malloc_func;
	my_free_func = free_func;
}

/*****  NULL-safe Functions  *****/


char * ICACHE_FLASH_ATTR
iks_strdup (const char *src)
{
	// if (src) return strdup(src);
	return NULL;
}

char * ICACHE_FLASH_ATTR
iks_strcat (char *dest, const char *src)
{
	// size_t len;

	// if (!src) return dest;

	// len = os_strlen (src);
	// os_memcpy (dest, src, len);
	// dest[len] = '\0';
	return NULL;
}

int ICACHE_FLASH_ATTR
iks_strcmp (const char *a, const char *b)
{
	if (!a || !b) return -1;
	return os_strcmp (a, b);
}

int ICACHE_FLASH_ATTR
iks_strcasecmp (const char *a, const char *b)
{
	if (!a || !b) return -1;
	return os_strcasecmp (a, b);
}

int ICACHE_FLASH_ATTR
iks_strncmp (const char *a, const char *b, size_t n)
{
	return -1;
	// if (!a || !b) return -1;
	// return os_strncmp (a, b, n);
}

int ICACHE_FLASH_ATTR
iks_strncasecmp (const char *a, const char *b, size_t n)
{
	return -1;
	// if (!a || !b) return -1;
	// return os_strncasecmp (a, b, n);
}

size_t ICACHE_FLASH_ATTR
iks_strlen (const char *src)
{
	if (!src) return 0;
	return os_strlen (src);
}

/*****  XML Escaping  *****/

char * ICACHE_FLASH_ATTR
iks_escape (ikstack *s, char *src, size_t len)
{
	char *ret;
	int i, j, nlen;

	if (!src || !s) return NULL;
	if (len == -1) len = os_strlen (src);

	nlen = len;
	for (i=0; i<len; i++) {
		switch (src[i]) {
		case '&': nlen += 4; break;
		case '<': nlen += 3; break;
		case '>': nlen += 3; break;
		case '\'': nlen += 5; break;
		case '"': nlen += 5; break;
		}
	}
	if (len == nlen) return src;

	ret = iks_stack_alloc (s, nlen + 1);
	if (!ret) return NULL;

	for (i=j=0; i<len; i++) {
		switch (src[i]) {
		case '&': os_memcpy (&ret[j], "&amp;", 5); j += 5; break;
		case '\'': os_memcpy (&ret[j], "&apos;", 6); j += 6; break;
		case '"': os_memcpy (&ret[j], "&quot;", 6); j += 6; break;
		case '<': os_memcpy (&ret[j], "&lt;", 4); j += 4; break;
		case '>': os_memcpy (&ret[j], "&gt;", 4); j += 4; break;
		default: ret[j++] = src[i];
		}
	}
	ret[j] = '\0';

	return ret;
}

char * ICACHE_FLASH_ATTR
iks_unescape (ikstack *s, char *src, size_t len)
{
	int i,j;
	char *ret;

	if (!s || !src) return NULL;
	if (!os_strchr (src, '&')) return src;
	if (len == -1) len = os_strlen (src);

	ret = iks_stack_alloc (s, len + 1);
	if (!ret) return NULL;

	for (i=j=0; i<len; i++) {
		if (src[i] == '&') {
			i++;
			if (os_strncmp (&src[i], "amp;", 4) == 0) {
				ret[j] = '&';
				i += 3;
			} else if (os_strncmp (&src[i], "quot;", 5) == 0) {
				ret[j] = '"';
				i += 4;
			} else if (os_strncmp (&src[i], "apos;", 5) == 0) {
				ret[j] = '\'';
				i += 4;
			} else if (os_strncmp (&src[i], "lt;", 3) == 0) {
				ret[j] = '<';
				i += 2;
			} else if (os_strncmp (&src[i], "gt;", 3) == 0) {
				ret[j] = '>';
				i += 2;
			} else {
				ret[j] = src[--i];
			}
		} else {
			ret[j] = src[i];
		}
		j++;
	}
	ret[j] = '\0';

	return ret;
}
