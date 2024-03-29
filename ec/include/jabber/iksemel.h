/* iksemel (XML parser for Jabber)
** Copyright (C) 2000-2007 Gurer Ozen
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#ifndef IKSEMEL_H
#define IKSEMEL_H 1

#include "os_type.h"
#include "c_types.h"
#define USE_DEFAULT_IO 1
#ifdef __cplusplus
// #include <cstddef>	/* size_t for C++ */
extern "C" {
#else
// #include <stddef.h>	/* size_t for C */
#endif

typedef enum {
	WIFI_INIT,
	WIFI_CONNECTING,
	WIFI_CONNECTING_ERROR,
	WIFI_CONNECTED,
	DNS_RESOLVE,
	TCP_DISCONNECTING,
	TCP_DISCONNECTED,
	TCP_RECONNECT_DISCONNECTING,
	TCP_RECONNECT_REQ,
	TCP_RECONNECT,
	TCP_CONNECTING,
	TCP_CONNECTING_ERROR,
	TCP_CONNECTED,
	IKS_CONNECT_SEND,
	IKS_CONNECT_SENDING,
	IKS_SUBSCIBE_SEND,
	IKS_SUBSCIBE_SENDING,
	IKS_DATA,
	IKS_KEEPALIVE_SEND,
	IKS_PUBLISH_RECV,
	IKS_PUBLISHING,
	IKS_DELETING,
	IKS_DELETED,
} tConnState;

/*****  object stack  *****/



/////

struct ikstack_struct;
typedef struct ikstack_struct ikstack;
// char * ICACHE_FLASH_ATTR  strchr(const char *s, int c);
ikstack * ICACHE_FLASH_ATTR iks_stack_new (size_t meta_chunk, size_t data_chunk);
void * ICACHE_FLASH_ATTR iks_stack_alloc (ikstack *s, size_t size);
char * ICACHE_FLASH_ATTR iks_stack_strdup (ikstack *s, const char *src, size_t len);
char * ICACHE_FLASH_ATTR iks_stack_strcat (ikstack *s, char *old, size_t old_len, const char *src, size_t src_len);
void ICACHE_FLASH_ATTR iks_stack_stat (ikstack *s, size_t *allocated, size_t *used);
void ICACHE_FLASH_ATTR iks_stack_delete (ikstack *s);

/*****  utilities  *****/

void * ICACHE_FLASH_ATTR iks_malloc (size_t size);
void ICACHE_FLASH_ATTR iks_free (void *ptr);
void ICACHE_FLASH_ATTR iks_set_mem_funcs (void *(*malloc_func)(size_t size), void (*free_func)(void *ptr));

char * ICACHE_FLASH_ATTR iks_strdup (const char *src);
char * ICACHE_FLASH_ATTR iks_strcat (char *dest, const char *src);
int ICACHE_FLASH_ATTR iks_strcmp (const char *a, const char *b);
int ICACHE_FLASH_ATTR iks_strcasecmp (const char *a, const char *b);
int ICACHE_FLASH_ATTR iks_strncmp (const char *a, const char *b, size_t n);
int ICACHE_FLASH_ATTR iks_strncasecmp (const char *a, const char *b, size_t n);
size_t ICACHE_FLASH_ATTR iks_strlen (const char *src);
char * ICACHE_FLASH_ATTR iks_escape (ikstack *s, char *src, size_t len);
char * ICACHE_FLASH_ATTR iks_unescape (ikstack *s, char *src, size_t len);

/*****  dom tree  *****/

enum ikstype {
	IKS_NONE = 0,
	IKS_TAG,
	IKS_ATTRIBUTE,
	IKS_CDATA
};

struct iks_struct;
typedef struct iks_struct iks;

iks * ICACHE_FLASH_ATTR iks_new (const char *name);
iks * ICACHE_FLASH_ATTR iks_new_within (const char *name, ikstack *s);
iks * ICACHE_FLASH_ATTR iks_insert (iks *x, const char *name);
iks * ICACHE_FLASH_ATTR iks_insert_cdata (iks *x, const char *data, size_t len);
iks * ICACHE_FLASH_ATTR iks_insert_attrib (iks *x, const char *name, const char *value);
iks * ICACHE_FLASH_ATTR iks_insert_node (iks *x, iks *y);
iks * ICACHE_FLASH_ATTR iks_append (iks *x, const char *name);
iks * ICACHE_FLASH_ATTR iks_prepend (iks *x, const char *name);
iks * ICACHE_FLASH_ATTR iks_append_cdata (iks *x, const char *data, size_t len);
iks * ICACHE_FLASH_ATTR iks_prepend_cdata (iks *x, const char *data, size_t len);
void ICACHE_FLASH_ATTR iks_hide (iks *x);
void ICACHE_FLASH_ATTR iks_delete (iks *x);
iks * ICACHE_FLASH_ATTR iks_next (iks *x);
iks * ICACHE_FLASH_ATTR iks_next_tag (iks *x);
iks * ICACHE_FLASH_ATTR iks_prev (iks *x);
iks * ICACHE_FLASH_ATTR iks_prev_tag (iks *x);
iks * ICACHE_FLASH_ATTR iks_parent (iks *x);
iks * ICACHE_FLASH_ATTR iks_root (iks *x);
iks * ICACHE_FLASH_ATTR iks_child (iks *x);
iks * ICACHE_FLASH_ATTR iks_first_tag (iks *x);
iks * ICACHE_FLASH_ATTR iks_attrib (iks *x);
iks * ICACHE_FLASH_ATTR iks_find (iks *x, const char *name);
char * ICACHE_FLASH_ATTR iks_find_cdata (iks *x, const char *name);
char * ICACHE_FLASH_ATTR iks_find_attrib (iks *x, const char *name);
iks * ICACHE_FLASH_ATTR iks_find_with_attrib (iks *x, const char *tagname, const char *attrname, const char *value);
ikstack * ICACHE_FLASH_ATTR iks_stack (iks *x);
enum ikstype  ICACHE_FLASH_ATTR iks_type (iks *x);
char * ICACHE_FLASH_ATTR iks_name (iks *x);
char * ICACHE_FLASH_ATTR iks_cdata (iks *x);
size_t ICACHE_FLASH_ATTR  iks_cdata_size (iks *x);
int ICACHE_FLASH_ATTR iks_has_children (iks *x);
int ICACHE_FLASH_ATTR iks_has_attribs (iks *x);
char * ICACHE_FLASH_ATTR iks_string (ikstack *s, iks *x);
iks * ICACHE_FLASH_ATTR iks_copy (iks *x);
iks * ICACHE_FLASH_ATTR iks_copy_within (iks *x, ikstack *s);

/*****  sax parser  *****/

enum ikserror {
	IKS_OK = 0,
	IKS_NOMEM,
	IKS_BADXML,
	IKS_HOOK
};

enum ikstagtype {
	IKS_OPEN,
	IKS_CLOSE,
	IKS_SINGLE
};

typedef int (iksTagHook)(void *user_data, char *name, char **atts, int type);
typedef int (iksCDataHook)(void *user_data, char *data, size_t len);
typedef void (iksDeleteHook)(void *user_data);

struct iksparser_struct;
typedef struct iksparser_struct  iksparser;

iksparser * ICACHE_FLASH_ATTR iks_sax_new (void *user_data, iksTagHook *tagHook, iksCDataHook *cdataHook);
iksparser * ICACHE_FLASH_ATTR iks_sax_extend (ikstack *s, void *user_data, iksTagHook *tagHook, iksCDataHook *cdataHook, iksDeleteHook *deleteHook);
ikstack * ICACHE_FLASH_ATTR iks_parser_stack (iksparser *prs);
void * ICACHE_FLASH_ATTR iks_user_data (iksparser *prs);
unsigned long ICACHE_FLASH_ATTR iks_nr_bytes (iksparser *prs);
unsigned long ICACHE_FLASH_ATTR iks_nr_lines (iksparser *prs);
int ICACHE_FLASH_ATTR iks_parse (iksparser *prs, const char *data, size_t len, int finish);
void ICACHE_FLASH_ATTR iks_parser_reset (iksparser *prs);
void ICACHE_FLASH_ATTR iks_parser_delete (iksparser *prs);

/*****  dom parser  *****/

enum iksfileerror {
	IKS_FILE_NOFILE = 4,
	IKS_FILE_NOACCESS,
	IKS_FILE_RWERR
};

iksparser * ICACHE_FLASH_ATTR iks_dom_new (iks **iksptr);
void ICACHE_FLASH_ATTR iks_set_size_hint (iksparser *prs, size_t approx_size);
iks * ICACHE_FLASH_ATTR iks_tree (const char *xml_str, size_t len, int *err);
// int ICACHE_FLASH_ATTR iks_load (const char *fname, iks **xptr);
// int ICACHE_FLASH_ATTR iks_save (const char *fname, iks *x);

/*****  transport layer  *****/

enum iksasyncevents {
	IKS_ASYNC_RESOLVED,
	IKS_ASYNC_CONNECTED,
	IKS_ASYNC_WRITE,
	IKS_ASYNC_WRITTEN,
	IKS_ASYNC_READ,
	IKS_ASYNC_CLOSED,
	IKS_ASYNC_ERROR
};

typedef struct iksasyncevent_struct {
	int event;
	int data0;
	int data1;
} iksasyncevent;

typedef void (iksTClose)(void *socket);
typedef int (iksTConnect)(iksparser *prs, void **socketptr, const char *server, int port);
typedef int (iksTSend)(void *socket, const char *data, size_t len);
typedef int (iksTRecv)(void *socket, char *buffer, size_t buf_len, int timeout);
typedef int (iksAsyncNotify)(void *user_data, iksasyncevent *event);
typedef int (iksTConnectAsync)(iksparser *prs, void **socketptr, const char *server, const char *server_name, int port, void *notify_data, iksAsyncNotify *notify_func);

#define IKS_TRANSPORT_V1 0

typedef const struct ikstransport_struct {
	int abi_version;
	/* basic api, connect can be NULL if only async api is provided */
	iksTConnect *connect;
	iksTSend *send;
	iksTRecv *recv;
	iksTClose *close;
	/* optional async api */
	iksTConnectAsync *connect_async;
} ikstransport;

extern ikstransport iks_default_transport;

/*****  tls layer  *****/

// struct ikstls_data;

// typedef int (iksTlsHandshake)(struct ikstls_data **datap, ikstransport *trans, void *sock);
// typedef int (iksTlsSend)(struct ikstls_data *data, const char *buf, size_t size);
// typedef int (iksTlsRecv)(struct ikstls_data *data, char *buf, size_t size, int timeout);
// typedef void (iksTlsTerminate)(struct ikstls_data *data);

// typedef const struct ikstls_struct {
// 	iksTlsHandshake *handshake;
// 	iksTlsSend *send;
// 	iksTlsRecv *recv;
// 	iksTlsTerminate *terminate;
// } ikstls;

// extern ikstls iks_default_tls;

/*****  stream parser  *****/

enum iksneterror {
	IKS_NET_NODNS = 4,
	IKS_NET_NOSOCK,
	IKS_NET_NOCONN,
	IKS_NET_RWERR,
	IKS_NET_NOTSUPP,
	IKS_NET_TLSFAIL,
	IKS_NET_DROPPED,
	IKS_NET_UNKNOWN
};

enum iksnodetype {
	IKS_NODE_START,
	IKS_NODE_NORMAL,
	IKS_NODE_ERROR,
	IKS_NODE_STOP
};

enum ikssasltype {
	IKS_SASL_PLAIN,
	IKS_SASL_DIGEST_MD5
};

// MARK: JABBER config
#define IKS_JABBER_PORT 5222
#define IKS_JABBER_HOSTNAME "gm.xsxwrd.com"

// MARK:  服务器的域名 资源
#define IKS_JABBER_DOMAIN "xsxwrd.com"
#define IKS_JABBER_RESOURCE "junyan"

typedef int (iksStreamHook)(void *user_data, int type, iks *node);
typedef void (iksLogHook)(void *user_data, const char *data, size_t size, int is_incoming);

iksparser *  ICACHE_FLASH_ATTR iks_stream_new (char *name_space, void *user_data, iksStreamHook *streamHook);
void * ICACHE_FLASH_ATTR iks_stream_user_data (iksparser *prs);
void ICACHE_FLASH_ATTR iks_set_log_hook (iksparser *prs, iksLogHook *logHook);
int ICACHE_FLASH_ATTR iks_connect_tcp (iksparser *prs, const char *server, int port);
int ICACHE_FLASH_ATTR iks_connect_fd (iksparser *prs, int fd);
int ICACHE_FLASH_ATTR iks_connect_via (iksparser *prs, const char *server, int port, const char *server_name);
int ICACHE_FLASH_ATTR iks_connect_with (iksparser *prs, const char *server, int port, const char *server_name, ikstransport *trans);
int ICACHE_FLASH_ATTR iks_connect_async (iksparser *prs, const char *server, int port, void *notify_data, iksAsyncNotify *notify_func);
int ICACHE_FLASH_ATTR iks_connect_async_with (iksparser *prs, const char *server, int port, const char *server_name, ikstransport *trans, void *notify_data, iksAsyncNotify *notify_func);
int ICACHE_FLASH_ATTR iks_fd (iksparser *prs);
// int iks_recv (iksparser *prs, int timeout);
int ICACHE_FLASH_ATTR iks_send_header (iksparser *prs, const char *to);
int ICACHE_FLASH_ATTR iks_send (iksparser *prs, iks *x);
int ICACHE_FLASH_ATTR iks_send_raw (iksparser *prs, const char *xmlstr);
void ICACHE_FLASH_ATTR iks_disconnect (iksparser *prs);
// int ICACHE_FLASH_ATTR iks_has_tls (void);
// int ICACHE_FLASH_ATTR iks_is_secure (iksparser *prs);
// int ICACHE_FLASH_ATTR iks_start_tls (iksparser *prs);
int ICACHE_FLASH_ATTR iks_start_sasl (iksparser *prs, enum ikssasltype type, char *username, char *pass);

/*****  jabber  *****/

#define IKS_NS_CLIENT     "jabber:client"
#define IKS_NS_SERVER     "jabber:server"
#define IKS_NS_AUTH       "jabber:iq:auth"
#define IKS_NS_AUTH_0K    "jabber:iq:auth:0k"
#define IKS_NS_REGISTER   "jabber:iq:register"
#define IKS_NS_ROSTER     "jabber:iq:roster"
#define IKS_NS_XROSTER	  "jabber:x:roster"
#define IKS_NS_OFFLINE    "jabber:x:offline"
#define IKS_NS_AGENT      "jabber:iq:agent"
#define IKS_NS_AGENTS     "jabber:iq:agents"
#define IKS_NS_BROWSE     "jabber:iq:browse"
#define IKS_NS_CONFERENCE "jabber:iq:conference"
#define IKS_NS_DELAY      "jabber:x:delay"
#define IKS_NS_VERSION    "jabber:iq:version"
#define IKS_NS_TIME       "jabber:iq:time"
#define IKS_NS_VCARD      "vcard-temp"
#define IKS_NS_PRIVATE    "jabber:iq:private"
#define IKS_NS_SEARCH     "jabber:iq:search"
#define IKS_NS_OOB        "jabber:iq:oob"
#define IKS_NS_XOOB       "jabber:x:oob"
#define IKS_NS_ADMIN      "jabber:iq:admin"
#define IKS_NS_FILTER     "jabber:iq:filter"
#define IKS_NS_GATEWAY    "jabber:iq:gateway"
#define IKS_NS_LAST       "jabber:iq:last"
#define IKS_NS_SIGNED     "jabber:x:signed"
#define IKS_NS_ENCRYPTED  "jabber:x:encrypted"
#define IKS_NS_ENVELOPE   "jabber:x:envelope"
#define IKS_NS_EVENT      "jabber:x:event"
#define IKS_NS_EXPIRE     "jabber:x:expire"
#define IKS_NS_XHTML      "http://www.w3.org/1999/xhtml"
#define IKS_NS_XMPP_SASL  "urn:ietf:params:xml:ns:xmpp-sasl"
#define IKS_NS_XMPP_BIND  "urn:ietf:params:xml:ns:xmpp-bind"
#define IKS_NS_XMPP_SESSION  "urn:ietf:params:xml:ns:xmpp-session"
#define IKS_NS_XMPP_PING   "urn:xmpp:ping"
#define IKS_NS_XMPP_PONG   "urn:xmpp:pong"

#define IKS_ID_USER 1
#define IKS_ID_SERVER 2
#define IKS_ID_RESOURCE 4
#define IKS_ID_PARTIAL IKS_ID_USER | IKS_ID_SERVER
#define IKS_ID_FULL IKS_ID_USER | IKS_ID_SERVER | IKS_ID_RESOURCE

#define IKS_STREAM_STARTTLS                   1
#define IKS_STREAM_SESSION                    2
#define IKS_STREAM_BIND                       4
#define IKS_STREAM_SASL_PLAIN                 8
#define IKS_STREAM_SASL_MD5                  16

typedef struct iksid_struct {
	char *user;
	char *server;
	char *resource;
	char *partial;
	char *full;
} iksid;

iksid * ICACHE_FLASH_ATTR iks_id_new (ikstack *s, const char *jid);
int ICACHE_FLASH_ATTR iks_id_cmp (iksid *a, iksid *b, int parts);

enum ikspaktype {
	IKS_PAK_NONE = 0,
	IKS_PAK_MESSAGE,
	IKS_PAK_PRESENCE,
	IKS_PAK_IQ,
	IKS_PAK_S10N
};

enum iksubtype {
	IKS_TYPE_NONE = 0,
	IKS_TYPE_ERROR,

	IKS_TYPE_CHAT,
	IKS_TYPE_GROUPCHAT,
	IKS_TYPE_HEADLINE,
    IKS_TYPE_NORMAL,

	IKS_TYPE_GET,
	IKS_TYPE_SET,
	IKS_TYPE_RESULT,

	IKS_TYPE_SUBSCRIBE,
	IKS_TYPE_SUBSCRIBED,
	IKS_TYPE_UNSUBSCRIBE,
	IKS_TYPE_UNSUBSCRIBED,
	IKS_TYPE_PROBE,
	IKS_TYPE_AVAILABLE,
	IKS_TYPE_UNAVAILABLE
};

enum ikshowtype {
	IKS_SHOW_UNAVAILABLE = 0,
	IKS_SHOW_AVAILABLE,
	IKS_SHOW_CHAT,
	IKS_SHOW_AWAY,
	IKS_SHOW_XA,
	IKS_SHOW_DND
};

typedef struct ikspak_struct {
	iks *x;
	iksid *from;
	iks *query;
	char *ns;
	char *id;
	enum ikspaktype type;
	enum iksubtype subtype;
	enum ikshowtype show;
} ikspak;

ikspak * ICACHE_FLASH_ATTR iks_packet (iks *x);

iks * ICACHE_FLASH_ATTR iks_make_auth (iksid *id, const char *pass, const char *sid);
iks * ICACHE_FLASH_ATTR iks_make_msg (enum iksubtype type, const char *to, const char *body);
iks * ICACHE_FLASH_ATTR iks_make_s10n (enum iksubtype type, const char *to, const char *msg);
iks * ICACHE_FLASH_ATTR iks_make_pres (enum ikshowtype show, const char *status);
iks * ICACHE_FLASH_ATTR iks_make_iq (enum iksubtype type, const char *xmlns);
iks * ICACHE_FLASH_ATTR iks_make_resource_bind(iksid *id);
iks * ICACHE_FLASH_ATTR iks_make_session(void);
int ICACHE_FLASH_ATTR iks_stream_features(iks *x);

/*****  jabber packet filter  *****/

#define IKS_RULE_DONE 0
#define IKS_RULE_ID 1
#define IKS_RULE_TYPE 2
#define IKS_RULE_SUBTYPE 4
#define IKS_RULE_FROM 8
#define IKS_RULE_FROM_PARTIAL 16
#define IKS_RULE_NS 32

enum iksfilterret {
	IKS_FILTER_PASS,
	IKS_FILTER_EAT
};

typedef int (iksFilterHook)(void *user_data, ikspak *pak);

struct iksfilter_struct;
typedef struct iksfilter_struct iksfilter;
struct iksrule_struct;
typedef struct iksrule_struct iksrule;

iksfilter * ICACHE_FLASH_ATTR iks_filter_new (void);
iksrule * ICACHE_FLASH_ATTR iks_filter_add_rule (iksfilter *f, iksFilterHook *filterHook, void *user_data, ...);
void ICACHE_FLASH_ATTR iks_filter_remove_rule (iksfilter *f, iksrule *rule);
void ICACHE_FLASH_ATTR iks_filter_remove_hook (iksfilter *f, iksFilterHook *filterHook);
void ICACHE_FLASH_ATTR iks_filter_packet (iksfilter *f, ikspak *pak);
void ICACHE_FLASH_ATTR iks_filter_delete (iksfilter *f);

/*****  sha1  *****/

struct iksha_struct;
typedef struct iksha_struct iksha;

iksha * ICACHE_FLASH_ATTR iks_sha_new (void);
void ICACHE_FLASH_ATTR iks_sha_reset (iksha *sha);
void ICACHE_FLASH_ATTR iks_sha_hash (iksha *sha, const unsigned char *data, size_t len, int finish);
void ICACHE_FLASH_ATTR iks_sha_print (iksha *sha, char *hash);
void ICACHE_FLASH_ATTR iks_sha_delete (iksha *sha);
void ICACHE_FLASH_ATTR iks_sha (const char *data, char *hash);

/*****  md5  *****/

struct ikmd5_struct;
typedef struct iksmd5_struct iksmd5;

iksmd5 * ICACHE_FLASH_ATTR iks_md5_new(void);
void ICACHE_FLASH_ATTR iks_md5_reset(iksmd5 *md5);
void ICACHE_FLASH_ATTR iks_md5_hash(iksmd5 *md5, const unsigned char *data, size_t slen, int finish);
void ICACHE_FLASH_ATTR iks_md5_delete(iksmd5 *md5);
void ICACHE_FLASH_ATTR iks_md5_print(iksmd5 *md5, char *buf);
void ICACHE_FLASH_ATTR iks_md5_digest(iksmd5 *md5, unsigned char *digest);
void ICACHE_FLASH_ATTR iks_md5(const char *data, char *buf);

/*****  base64  *****/

char * ICACHE_FLASH_ATTR iks_base64_decode(const char *buf);
char * ICACHE_FLASH_ATTR iks_base64_encode(const char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif  /* IKSEMEL_H */
