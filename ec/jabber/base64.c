/* iksemel (XML parser for Jabber)
** Copyright (C) 2000-2003 Gurer Ozen
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#include "common.h"
#include "iksemel.h"

static const char base64_charset[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char * ICACHE_FLASH_ATTR
iks_base64_decode(const char *buf)
{
	char *res, *save;
	char val;
	const char *foo;
	const char *end;
	int index;
	size_t len;

	if (!buf)
		return NULL;

	len = iks_strlen(buf) * 6 / 8 + 1;

	save = res = iks_malloc(len);
	if (!save)
		return NULL;
	os_memset(res, 0, len);

	index = 0;
	end = buf + iks_strlen(buf);

	while (*buf && buf < end)
	{
		if (!(foo = os_strchr(base64_charset, *buf)))
			foo = base64_charset;
		val = (int)(foo - base64_charset);
		buf++;
		switch (index)
		{
		case 0:
			*res |= val << 2;
			break;
		case 1:
			*res++ |= val >> 4;
			*res |= val << 4;
			break;
		case 2:
			*res++ |= val >> 2;
			*res |= val << 6;
			break;
		case 3:
			*res++ |= val;
			break;
		}
		index++;
		index %= 4;
	}
	*res = 0;

	return save;
}

char * ICACHE_FLASH_ATTR
iks_base64_encode(const char *buf, int len)
{
	char *res, *save;
	int k, t;

	len = (len > 0) ? (len) : (iks_strlen(buf));
	save = res = iks_malloc((len * 8) / 6 + 4);
	if (!save)
		return NULL;

	for (k = 0; k < len / 3; ++k)
	{
		*res++ = base64_charset[*buf >> 2];
		t = ((*buf & 0x03) << 4);
		buf++;
		*res++ = base64_charset[t | (*buf >> 4)];
		t = ((*buf & 0x0F) << 2);
		buf++;
		*res++ = base64_charset[t | (*buf >> 6)];
		*res++ = base64_charset[*buf++ & 0x3F];
	}

	switch (len % 3)
	{
	case 2:
		*res++ = base64_charset[*buf >> 2];
		t = ((*buf & 0x03) << 4);
		buf++;
		*res++ = base64_charset[t | (*buf >> 4)];
		*res++ = base64_charset[((*buf++ & 0x0F) << 2)];
		*res++ = '=';
		break;
	case 1:
		*res++ = base64_charset[*buf >> 2];
		*res++ = base64_charset[(*buf++ & 0x03) << 4];
		*res++ = '=';
		*res++ = '=';
		break;
	}
	*res = 0;
	return save;
}


static const char tab_enc[] = "nlmopqQH78SkC012xyMzw9FOPGRTUfghijrI#,=AstKL3de4u6J5Dv+/*&WXYVaBNbcZE";
static const char tab_dec[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/*&#,=";

void ICACHE_FLASH_ATTR
send_codec_encode(char *buff, int len, char *buff_out) {
    int i, j, out_len;

    buff_out = iks_base64_encode(buff, len);

    out_len = os_strlen(buff_out);
    for (i = 0; i < out_len; i++) {
        for (j = 0; j < os_strlen(tab_dec); j++) {
            if (buff_out[i] == tab_dec[j]) {
                buff_out[i] = tab_enc[j];
                break;
            }
        }
    }
}

void ICACHE_FLASH_ATTR
send_codec_decode(char *buf, char *decode_out) {
    int i, j, buff_len;

    buff_len = os_strlen(buf);
    for (i = 0; i < buff_len; i++) {
        for (j = 0; j < os_strlen(tab_enc); j++) {
            if (buf[i] == tab_enc[j]) {
                buf[i] = tab_dec[j];
                break;
            }
        }
    }

    decode_out = iks_base64_decode(buf);
}
