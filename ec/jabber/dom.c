/* iksemel (XML parser for Jabber)
** Copyright (C) 2000-2003 Gurer Ozen
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#include "common.h"
#include "iksemel.h"

struct dom_data {
	iks **iksptr;
	iks *current;
	size_t chunk_size;
};

static int ICACHE_FLASH_ATTR
tagHook (struct dom_data *data, char *name, char **atts, int type)
{
	iks *x;

	if (IKS_OPEN == type || IKS_SINGLE == type) {
		if (data->current) {
			x = iks_insert (data->current, name);
		} else {
			ikstack *s;
			s = iks_stack_new (data->chunk_size, data->chunk_size);
			x = iks_new_within (name, s);
		}
		if (atts) {
			int i=0;
			while (atts[i]) {
				iks_insert_attrib (x, atts[i], atts[i+1]);
				i += 2;
			}
		}
		data->current = x;
	}
	if (IKS_CLOSE == type || IKS_SINGLE == type) {
		x = iks_parent (data->current);
		if (iks_strcmp(iks_name(data->current), name) != 0)
			return IKS_BADXML;
		if (x)
			data->current = x;
		else {
			*(data->iksptr) = data->current;
			data->current = NULL;
		}
	}
	return IKS_OK;
}

static int ICACHE_FLASH_ATTR
cdataHook (struct dom_data *data, char *cdata, size_t len)
{
	if (data->current) iks_insert_cdata (data->current, cdata, len);
	return IKS_OK;
}

static void ICACHE_FLASH_ATTR
deleteHook (struct dom_data *data)
{
	if (data->current) iks_delete (data->current);
	data->current = NULL;
}

iksparser * ICACHE_FLASH_ATTR
iks_dom_new (iks **iksptr)
{
	ikstack *s;
	struct dom_data *data;

	*iksptr = NULL;
	s = iks_stack_new (DEFAULT_DOM_CHUNK_SIZE, 0);
	if (!s) return NULL;
	data = iks_stack_alloc (s, sizeof (struct dom_data));
	data->iksptr = iksptr;
	data->current = NULL;
	data->chunk_size = DEFAULT_DOM_IKS_CHUNK_SIZE;
	return iks_sax_extend (s, data, (iksTagHook *) tagHook, (iksCDataHook *) cdataHook, (iksDeleteHook *) deleteHook);
}

void ICACHE_FLASH_ATTR
iks_set_size_hint (iksparser *prs, size_t approx_size)
{
	size_t cs;
	struct dom_data *data = iks_user_data (prs);

	cs = approx_size / 10;
	if (cs < DEFAULT_DOM_IKS_CHUNK_SIZE) cs = DEFAULT_DOM_IKS_CHUNK_SIZE;
	data->chunk_size = cs;
}

iks * ICACHE_FLASH_ATTR
iks_tree (const char *xml_str, size_t len, int *err)
{
	iksparser *prs;
	iks *x;
	int e;

	if (0 == len) len = os_strlen (xml_str);
	prs = iks_dom_new (&x);
	if (!prs) {
		if (err) *err = IKS_NOMEM;
		return NULL;
	}
	e = iks_parse (prs, xml_str, len, 1);
	if (err) *err = e;
	iks_parser_delete (prs);
	return x;
}