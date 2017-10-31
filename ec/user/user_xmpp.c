#include "jabber_config.h"
#include "common.h"
#include "iksemel.h"


/* stuff we keep per session */
struct session {
	iksparser *prs;
	iksid *acc;
	char *pass;
	int features;
	int authorized;
	int counter;
	int set_roster;
	int job_done;
};

/* precious roster we'll deal with */
iks *my_roster;

/* out packet filter */
iksfilter *my_filter;

/* connection time outs if nothing comes for this much seconds */
int opt_timeout = 30;

struct session j_sess;

int opt_log = 0;

struct jabber_config j_config;

static void ICACHE_FLASH_ATTR on_presence();

static void ICACHE_FLASH_ATTR
j_error (char *msg)
{
	ec_log ("iksroster: %s \r\n", msg);
	// exit (2);
}

static int ICACHE_FLASH_ATTR
on_result (struct session *sess, ikspak *pak)
{
	on_presence();
	return IKS_FILTER_EAT;
}


// MARK: 发送消息
static int ICACHE_FLASH_ATTR
ec_make_message(char *subject, char *body, char *id, char *to)
{
	iks *m;
	m = iks_new("message");
	iks_insert_attrib(m, "type", "normal");
	iks_insert_attrib(m, "to", to);
	iks_insert_attrib(m, "id", id);
	iks_insert_cdata(iks_insert(m, "subject"), subject, 0);
	iks_insert_cdata(iks_insert(m, "body"), body, 0);
	return IKS_FILTER_EAT;
}

// MARK: 发送回执消息
#define RECEIPT_M "{\"act\":\"client\",\
\"result\":\"%d\",\
\"reason\":\"%s\",\
\"linkid\":\"%s\"}"

static int ICACHE_FLASH_ATTR
on_receipt(char *to, char *id, char *subject, char *linkid, int result)
{
	char cresult[32] = {0}, body[64] = {0}, ebody[128] = {0};

	if (result)
	{
		os_sprintf(cresult, "成功");
	}
	else
	{
		os_sprintf(cresult, "失败");
	}
	os_sprintf(body, RECEIPT_M, result, cresult, linkid);
	send_codec_encode(body, os_strlen(body), ebody);

	ec_make_message(subject, ebody, id, to);
	return IKS_FILTER_EAT;
}

// MARK: 主动发起数据同步
#define INITINFO_M "{\"act\":\"client\",\
\"power\":\"%d\",\
\"totalPower\":\"%d\",\
\"co2\":\"%d\",\
\"co\":\"%d\",\
\"pm25\":\"%d\",\
\"state\":\"%d\"}"

void ICACHE_FLASH_ATTR
on_infomation(int power, int totalPower, int co2, int co, int pm25, int state)
{

	char id[16] = {0}, body[256] = {0}, ebody[512] = {0};
	// TODO: 看是在这里直接调用接口 还是
	os_sprintf(body, INITINFO_M, power, totalPower, co2, co, pm25, state);
	send_codec_encode(body, os_strlen(body), ebody);
	// TODO: 这里直接填入fullur;
	get_random_string(12, id);
	ec_make_message("initinfo", ebody, id, "to");
}

// MARK: 服务器发起同步数据
#define ASYNCINFO_M "{\"act\":\"client\",\
\"power\":\"%d\",\
\"totalPower\":\"%d\",\
\"co2\":\"%d\",\
\"co\":\"%d\",\
\"pm25\":\"%d\",\
\"state\":\"%d\",\
\"linkid\":\"%s\"}"

void ICACHE_FLASH_ATTR
on_asyncinfomation(int power, int totalPower, int co2, int co, int pm25, int state, char *linkid)
{
	char id[16] = {0}, body[256] = {0}, ebody[512] = {0};
	// TODO: 看是在这里直接调用接口 还是
	os_sprintf(body, ASYNCINFO_M, power, totalPower, co2, co, pm25, state, linkid);
	send_codec_encode(body, os_strlen(body), ebody);
	// TODO: 这里直接填入服务器地址
	get_random_string(12, id);
	ec_make_message("async", ebody, id, "to");
}


static int ICACHE_FLASH_ATTR
on_stream (struct session *sess, int type, iks *node)
{
	sess->counter = opt_timeout;

	switch (type) {
		case IKS_NODE_START:
			break;

		case IKS_NODE_NORMAL:
			if (os_strcmp ("stream:features", iks_name (node)) == 0) {

				sess->features = iks_stream_features (node);

				if (sess->authorized) {
					iks *t;
					// ec_log("IKS_NODE_NORMAL authorized == %d features == %d",sess->authorized,sess->features);
					if (sess->features & IKS_STREAM_BIND) {
						// ec_log("---000====");
						t = iks_make_resource_bind (sess->acc);
						// ec_log("---111====");
						iks_send (sess->prs, t);
						// ec_log("---222====");
						iks_delete (t);
					}
					if (sess->features & IKS_STREAM_SESSION) {
						// ec_log("IKS_NODE_NORMAL sess->features & IKS_STREAM_SESSION");
						t = iks_make_session ();
						iks_insert_attrib (t, "id", "auth");
						iks_send (sess->prs, t);
						iks_delete (t);
					}
				} else {
					// ec_log("IKS_NODE_NORMAL sess->authorized == 0");
					if (sess->features & IKS_STREAM_SASL_MD5)
					{
						iks_start_sasl (sess->prs, IKS_SASL_DIGEST_MD5, sess->acc->user, sess->pass);
					}
					else if (sess->features & IKS_STREAM_SASL_PLAIN)
					{
						// ec_log("IKS_NODE_NORMAL sess->authorized == 0 IKS_STREAM_SASL_PLAIN");
						iks_start_sasl (sess->prs, IKS_SASL_PLAIN, sess->acc->user, sess->pass);
					}
				}
			} else if (os_strcmp ("failure", iks_name (node)) == 0) {
				j_error ("sasl authentication failed");
			} else if (os_strcmp ("success", iks_name (node)) == 0) {
				sess->authorized = 1;
				iks_send_header (sess->prs, sess->acc->server);
			} else {
				ikspak *pak;
				pak = iks_packet (node);
				iks_filter_packet (my_filter, pak);
				if (sess->job_done == 1) return IKS_HOOK;
			}
			break;
		case IKS_NODE_STOP:
			j_error ("server disconnected");
		case IKS_NODE_ERROR:
			j_error ("stream error");
	}
	if (node) iks_delete (node);
	return IKS_OK;
}

static int ICACHE_FLASH_ATTR
on_error (void *user_data, ikspak *pak)
{
	j_error ("authorization failed");
	return IKS_FILTER_EAT;
}

static int ICACHE_FLASH_ATTR
on_roster (struct session *sess, ikspak *pak)
{
	// ec_log("---- on_roster --- \r\n");
	my_roster = pak->x;
	sess->job_done = 1;
	return IKS_FILTER_EAT;
}

static void ICACHE_FLASH_ATTR
ec_keepalive(void *arg)
{
	iks *t;
	char id[16] = { 0 };
	get_random_string(12, id);
	t = iks_new("iq");
	iks_insert_attrib(t, "type", "get");
	iks_insert_attrib(t, "id", id);
	iks_insert_attrib(t, "from", j_sess.acc->full);
	iks_insert_attrib(t, "to", j_sess.acc->server);
	iks_insert_attrib(iks_insert(t, "ping"), "xmlns", IKS_NS_XMPP_PING);
	iks_send(j_sess.prs, t);
	iks_delete(t);

}

static int ICACHE_FLASH_ATTR
on_ping(struct session *sess, ikspak *pak)
{
	iks *t;
	char *id;
	id = pak->id;
	t = iks_new("iq");
	iks_insert_attrib(t, "type", "result");
	iks_insert_attrib(t, "id", id);
	iks_insert_attrib(t, "from", j_sess.acc->full);
	iks_insert_attrib(t, "to", j_sess.acc->server);
	iks_send(sess->prs, t);
	iks_delete(t);
	return IKS_FILTER_EAT;
}

static int ICACHE_FLASH_ATTR
on_message(struct session *sess, ikspak *pak)
{
	char *id, *from,*body, *subject;
	id = pak->id;
	subject = iks_find_cdata(pak->x, "subject");
	body = iks_find_cdata(pak->x, "body");

	ec_log("\r\n ==== on_message ===== \r\n");

    if (os_strcmp(subject,"switch") == 0)
    {
    	// TODO: 服务器下发开关
    	int ret, t;
        char linkid[32] = { 0 };
    	ret = json_parse_switch(body, linkid, &t);
    	if(t == 1)
    	{
    		// TODO: 发送回执
    		on_receipt(from, id, "switch", linkid, ret);
    	}

    }
    else if (os_strcmp(subject,"async") == 0)
    {
    	// TODO: 服务器请求同步数据
    	int ret;
    	char linkid[32] = { 0 };
    	json_parse_async(body);
    }
    else if (os_strcmp(subject,"commamd") == 0)
    {
    	int ret;
        char linkid[32] = { 0 };
    	ret = json_parse_commamd(body, linkid);
    	// MARK: 发送回执 返回是否执行成工 需要发送linkid
    	on_receipt(from, id, "commamd", linkid, ret);
    }
    else if (os_strcmp(subject,"initinfo") == 0)
    {
    	// TODO: 服务器请求同步 数据信息 直接调用数据上传 不会带上任何回执消息
    	int ret;
    	ec_get_info();
    }
    return IKS_FILTER_EAT;
}

static void ICACHE_FLASH_ATTR
on_presence() {
	iks *t;
	t = iks_make_pres(IKS_SHOW_AVAILABLE,"ONLINE");
	iks_send (j_sess.prs, t);
	iks_delete (t);
}

static int ICACHE_FLASH_ATTR
on_bing (struct session *sess, ikspak *pak)
{
	iks *t;
	t = iks_make_session ();
	iks_insert_attrib (t, "id", pak->id);
	iks_send (sess->prs, t);
	iks_delete (t);

	on_presence();
	return IKS_FILTER_EAT;
}

static void ICACHE_FLASH_ATTR
on_log (struct session *sess, const char *data, size_t size, int is_incoming)
{
}

void ICACHE_FLASH_ATTR
j_setup_filter (struct session *sess)
{
	if (my_filter) iks_filter_delete (my_filter);
	my_filter = iks_filter_new ();

	iks_filter_add_rule (my_filter, (iksFilterHook *) on_result, sess,
		IKS_RULE_TYPE, IKS_PAK_IQ,
		IKS_RULE_SUBTYPE, IKS_TYPE_RESULT,
		IKS_RULE_ID, "auth",
		IKS_RULE_DONE);

	iks_filter_add_rule (my_filter, on_error, sess,
		IKS_RULE_TYPE, IKS_PAK_IQ,
		IKS_RULE_SUBTYPE, IKS_TYPE_ERROR,
		IKS_RULE_ID, "auth",
		IKS_RULE_DONE);

	iks_filter_add_rule (my_filter, (iksFilterHook *) on_roster, sess,
		IKS_RULE_TYPE, IKS_PAK_IQ,
		IKS_RULE_SUBTYPE, IKS_TYPE_RESULT,
		IKS_RULE_ID, "roster",
		IKS_RULE_DONE);

	iks_filter_add_rule (my_filter, (iksFilterHook *) on_bing, sess,
		IKS_RULE_TYPE, IKS_PAK_IQ,
		IKS_RULE_SUBTYPE, IKS_TYPE_RESULT,
		IKS_RULE_NS, IKS_NS_XMPP_BIND,
		IKS_RULE_DONE);

	iks_filter_add_rule (my_filter, (iksFilterHook *) on_ping, sess,
		IKS_RULE_TYPE, IKS_PAK_IQ,
		IKS_RULE_SUBTYPE, IKS_TYPE_GET,
		IKS_RULE_NS, IKS_NS_XMPP_PING,
		IKS_RULE_DONE);

    iks_filter_add_rule(my_filter, (iksFilterHook *)on_message, sess,
		IKS_RULE_TYPE, IKS_PAK_MESSAGE,
		IKS_RULE_DONE);
}



void ICACHE_FLASH_ATTR
j_connect (char *jabber_id, 
	char *pass, 
	int set_roster, 
	char *username, 
	char *host_name)
{

	ec_log("j_connect %s\r\n",jabber_id);

	os_memset (&j_sess, 0, sizeof (j_sess));
	j_sess.authorized = 0;

	j_sess.prs = iks_stream_new (IKS_NS_CLIENT, &j_sess, (iksStreamHook *) on_stream);

	if (opt_log) iks_set_log_hook (j_sess.prs, (iksLogHook *) on_log);

	j_sess.acc = iks_id_new (iks_parser_stack (j_sess.prs), jabber_id);

	j_sess.acc->user = username;
	j_sess.acc->server = j_config.domain;

	if (NULL == j_sess.acc->resource) {
		/* user gave no resource name, use the default */
		char *tmp;
		tmp = iks_malloc (os_strlen (j_sess.acc->user) + os_strlen (j_sess.acc->server) + 9 + 3);

		os_sprintf (tmp, "%s@%s/%s", j_sess.acc->user, j_sess.acc->server, EC_VERSION);
		j_sess.acc = iks_id_new (iks_parser_stack (j_sess.prs), tmp);
		iks_free (tmp);
	}
	j_sess.pass = pass;

	j_sess.set_roster = set_roster;

	j_setup_filter (&j_sess);

	iks_connect_tcp (j_sess.prs, host_name, j_config.port);

	j_sess.counter = opt_timeout;
}


void ICACHE_FLASH_ATTR
xmpp_init(struct jabber_config *config)
{
	if (config != NULL)
	{
		char full_jid[64] = { 0 };
		os_sprintf(full_jid, "%s@%s/%s", config->username, config->domain, EC_VERSION);

		{
			// 测试代码
			os_memset(&j_config,0x0,sizeof(j_config));
			// 这里必须配置一次
			j_config.port = 5222;
			// os_memcpy(j_config.ip, config->ip, os_strlen(config->ip));
			// os_memcpy(j_config.resources, EC_VERSION, os_strlen(EC_VERSION));
			os_memcpy(j_config.username, "18682435851", os_strlen("18682435851"));
			os_memcpy(j_config.password, "18682435851", os_strlen("18682435851"));
			// os_memcpy(j_config.app_username, config->app_username, os_strlen(config->app_username));
			os_memcpy(j_config.domain, "xsxwrd.com", os_strlen("xsxwrd.com"));
			os_memcpy(j_config.host_name, "gm.xsxwrd.com", os_strlen("gm.xsxwrd.com"));
		}
		ec_log("j_config ip %s \r\n", j_config.ip);
		ec_log("j_config port %d \r\n", j_config.port);
		ec_log("j_config resources %s \r\n", j_config.resources);
		ec_log("j_config username %s \r\n", j_config.username);
		ec_log("j_config password %s \r\n", j_config.password);
		ec_log("j_config app_username %s \r\n", j_config.app_username);
		ec_log("j_config domain %s \r\n", j_config.domain);
		ec_log("j_config host_name %s\r\n", j_config.host_name);

		// NOTE: 这里由于host不可能是null 必须要加上长度判断
		if (os_strlen(config->host_name) > 3)
		{
			j_connect(full_jid, j_config.password, 0, j_config.username, j_config.host_name);
		}
		else
		{
			j_connect(full_jid, j_config.password, 0,j_config.username, j_config.ip);
		}

		return;
	}
	else if (jabber_get_config(&j_config))
	{
		char full_jid[32] = {0};
		os_sprintf(full_jid, "%s@%s/%s", j_config.username, j_config.domain, j_config.resources);
		j_connect(full_jid, j_config.password, 0, j_config.username, j_config.host_name);

		if (os_strlen(j_config.host_name) > 3)
		{
			j_connect(full_jid, j_config.password, 0, j_config.username, j_config.host_name);
		}
		else
		{
			j_connect(full_jid, j_config.password, 0, j_config.username, j_config.ip);
		}
	}
	else
	{
		// FIXME: 系统致命错误
		ec_log("sys error \r\n");
	}
}