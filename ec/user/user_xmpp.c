#include "jabber_config.h"
#include "common.h"
#include "iksemel.h"

// #ifdef HAVE_GETOPT_LONG
// #include <getopt.h>
// #endif

// #ifdef _WIN32
// #include <winsock.h>
// #endif

// #ifdef HAVE_GETOPT_LONG
// static struct option longopts[] = {
// 	{ "backup", required_argument, 0, 'b' },
// 	{ "restore", required_argument, 0, 'r' },
// 	{ "file", required_argument, 0, 'f' },
// 	{ "timeout", required_argument, 0, 't' },
// 	{ "secure", 0, 0, 's' },
// 	{ "sasl", 0, 0, 'a' },
// 	{ "plain", 0, 0, 'p' },
// 	{ "log", 0, 0, 'l' },
// 	{ "help", 0, 0, 'h' },
// 	{ "version", 0, 0, 'V' },
// 	{ 0, 0, 0, 0 }
// };
// #endif

/* stuff we keep per session */
struct session
{
	iksparser *prs;
	iksid *acc;
	char *pass;
	int features;
	int authorized;
	int counter;
	int set_roster;
	int job_done;
};
struct session j_sess;
/* precious roster we'll deal with */
iks *my_roster;

/* out packet filter */
iksfilter *my_filter;

/* connection time outs if nothing comes for this much seconds */
int opt_timeout = 30;

/* connection flags */
int opt_use_tls;
int opt_use_sasl;
int opt_use_plain;
// int opt_log;

struct jabber_config j_config;

void ICACHE_FLASH_ATTR
j_error(char *msg)
{
	// fprintf (stderr, "iksroster: %s\n", msg);
	// exit (2);
}

int ICACHE_FLASH_ATTR
on_result(struct session *sess, ikspak *pak)
{
	// iks *x;

	// if (sess->set_roster == 0) {
	// 	x = iks_make_iq (IKS_TYPE_GET, IKS_NS_ROSTER);
	// 	iks_insert_attrib (x, "id", "roster");
	// 	iks_send (sess->prs, x);
	// 	iks_delete (x);
	// } else {
	// 	iks_insert_attrib (my_roster, "type", "set");
	// 	iks_send (sess->prs, my_roster);
	// }
	on_presence();
	return IKS_FILTER_EAT;
}

int ICACHE_FLASH_ATTR
on_stream(struct session *sess, int type, iks *node)
{
	sess->counter = opt_timeout;

	switch (type)
	{
	case IKS_NODE_START:
		// if (opt_use_tls && !iks_is_secure(sess->prs))
		// {
		// 	iks_start_tls(sess->prs);
		// 	break;
		// }
		// if (!opt_use_sasl)
		// {
		// 	iks *x;
		// 	char *sid = NULL;

		// 	if (!opt_use_plain)
		// 		sid = iks_find_attrib(node, "id");
		// 	x = iks_make_auth(sess->acc, sess->pass, sid);
		// 	iks_insert_attrib(x, "id", "auth");
		// 	iks_send(sess->prs, x);
		// 	iks_delete(x);
		// }
		break;

	case IKS_NODE_NORMAL:
		if (os_strcmp("stream:features", iks_name(node)) == 0)
		{
			sess->features = iks_stream_features(node);

			// if (opt_use_sasl)
			{
				// if (opt_use_tls && !iks_is_secure(sess->prs))
				// 	break;
				if (sess->authorized)
				{
					iks *t;
					if (sess->features & IKS_STREAM_BIND)
					{
						t = iks_make_resource_bind(sess->acc);
						iks_send(sess->prs, t);
						iks_delete(t);
					}
					if (sess->features & IKS_STREAM_SESSION)
					{
						t = iks_make_session();
						iks_insert_attrib(t, "id", "auth");
						iks_send(sess->prs, t);
						iks_delete(t);
					}
				}
				else
				{
					if (sess->features & IKS_STREAM_SASL_MD5)
						iks_start_sasl(sess->prs, IKS_SASL_DIGEST_MD5, sess->acc->user, sess->pass);
					else if (sess->features & IKS_STREAM_SASL_PLAIN)
						iks_start_sasl(sess->prs, IKS_SASL_PLAIN, sess->acc->user, sess->pass);
				}
			}
		}
		else if (os_strcmp("failure", iks_name(node)) == 0)
		{
			j_error("sasl authentication failed");
		}
		else if (os_strcmp("success", iks_name(node)) == 0)
		{
			sess->authorized = 1;
			iks_send_header(sess->prs, sess->acc->server);
		}
		else
		{
			ikspak *pak;

			pak = iks_packet(node);
			iks_filter_packet(my_filter, pak);
			if (sess->job_done == 1)
				return IKS_HOOK;
		}
		break;

	case IKS_NODE_STOP:
		j_error("server disconnected");

	case IKS_NODE_ERROR:
		j_error("stream error");
	}

	if (node)
		iks_delete(node);
	return IKS_OK;
}

int ICACHE_FLASH_ATTR
on_error(void *user_data, ikspak *pak)
{
	j_error("authorization failed");
	return IKS_FILTER_EAT;
}

int ICACHE_FLASH_ATTR
on_ping(struct session *sess, ikspak *pak)
{
	iks *t;
	char *id;
	// ec_log("---- on_ping --- \r\n");
	id = pak->id;
	t = iks_new("iq");
	iks_insert_attrib(t, "type", "set");
	iks_insert_attrib(t, "id", id);
	iks_insert_attrib(iks_insert(t, "pong"), "xmlns", IKS_NS_XMPP_PONG);
	iks_send(sess->prs, t);
	iks_delete(t);
	return IKS_FILTER_EAT;
}

int ICACHE_FLASH_ATTR
on_presence()
{
	iks *t;
	t = iks_make_pres(IKS_SHOW_AVAILABLE, "ONLINE");
	iks_send(j_sess.prs, t);
	iks_delete(t);
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
int ICACHE_FLASH_ATTR
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

int ICACHE_FLASH_ATTR
on_bing(struct session *sess, ikspak *pak)
{
	iks *t;
	char sid[16] = {0};
	// ec_log("---- on_bing --- \r\n");
	// MARK: 创建一个新的session
	get_random_string(12, sid);
	t = iks_make_session();
	iks_insert_attrib(t, "id", sid);
	iks_send(sess->prs, t);
	iks_delete(t);
	// on_presence();
	return IKS_FILTER_EAT;
}

int ICACHE_FLASH_ATTR
on_message(struct session *sess, ikspak *pak)
{
	char *id, *body, *subject;
	subject = iks_find_cdata(pak->x, "subject");
	body = iks_find_cdata(pak->x, "body");
	ec_log("\r\n ==== on_message ===== \r\n");
	// TODO: 进行数据解析
	// TODO: 进行对应操作
}

int ICACHE_FLASH_ATTR
on_roster(struct session *sess, ikspak *pak)
{
	my_roster = pak->x;
	sess->job_done = 1;
	return IKS_FILTER_EAT;
}

void ICACHE_FLASH_ATTR
on_log(struct session *sess, const char *data, size_t size, int is_incoming)
{
	// if (iks_is_secure (sess->prs)) fprintf (stderr, "Sec");
	// if (is_incoming) fprintf (stderr, "RECV"); else fprintf (stderr, "SEND");
	// fprintf (stderr, "[%s]\n", data);
}

void ICACHE_FLASH_ATTR
j_setup_filter(struct session *sess)
{
	if (my_filter)
		iks_filter_delete(my_filter);
	my_filter = iks_filter_new();
	iks_filter_add_rule(my_filter, (iksFilterHook *)on_result, sess,
						IKS_RULE_TYPE, IKS_PAK_IQ,
						IKS_RULE_SUBTYPE, IKS_TYPE_RESULT,
						IKS_RULE_ID, "auth",
						IKS_RULE_DONE);
	iks_filter_add_rule(my_filter, on_error, sess,
						IKS_RULE_TYPE, IKS_PAK_IQ,
						IKS_RULE_SUBTYPE, IKS_TYPE_ERROR,
						IKS_RULE_ID, "auth",
						IKS_RULE_DONE);
	iks_filter_add_rule(my_filter, (iksFilterHook *)on_roster, sess,
						IKS_RULE_TYPE, IKS_PAK_IQ,
						IKS_RULE_SUBTYPE, IKS_TYPE_RESULT,
						IKS_RULE_ID, "roster",
						IKS_RULE_DONE);

	iks_filter_add_rule(my_filter, (iksFilterHook *)on_bing, sess,
						IKS_RULE_TYPE, IKS_PAK_IQ,
						IKS_RULE_SUBTYPE, IKS_TYPE_RESULT,
						IKS_RULE_NS, IKS_NS_XMPP_BIND,
						IKS_RULE_DONE);

	iks_filter_add_rule(my_filter, (iksFilterHook *)on_ping, sess,
						IKS_RULE_TYPE, IKS_PAK_IQ,
						IKS_RULE_SUBTYPE, IKS_TYPE_GET,
						IKS_RULE_NS, IKS_NS_XMPP_PING,
						IKS_RULE_DONE);

	iks_filter_add_rule(my_filter, (iksFilterHook *)on_message, sess,
						IKS_RULE_TYPE, IKS_PAK_MESSAGE,
						IKS_RULE_DONE);
}

void ICACHE_FLASH_ATTR
j_connect(char *jabber_id, char *pass, int set_roster, char *username, char *host_name)
{

	ec_log("jabber connect \r\n");
	os_memset (&j_sess, 0, sizeof (j_sess));
	j_sess.authorized = 0;
	j_sess.prs = iks_stream_new (IKS_NS_CLIENT, &j_sess, (iksStreamHook *) on_stream);
	if (1) iks_set_log_hook (j_sess.prs, (iksLogHook *) on_log);
	j_sess.acc = iks_id_new (iks_parser_stack (j_sess.prs), jabber_id);

	if (NULL == j_sess.acc->resource)
	{
		/* user gave no resource name, use the default */
		// char *tmp;
		// tmp = iks_malloc (strlen (j_sess.acc->user) + strlen (j_sess.acc->server) + 9 + 3);
		// sprintf (tmp, "%s@%s/%s", j_sess.acc->user, j_sess.acc->server, "iksroster");
		// j_sess.acc = iks_id_new (iks_parser_stack (j_sess.prs), tmp);
		// iks_free (tmp);
		char tmp[32] = {0};
		os_sprintf(tmp, "%s@%s/%s", username, j_config.domain, EC_VERSION);
		j_sess.acc = iks_id_new(iks_parser_stack(j_sess.prs), tmp);
	}
	j_sess.pass = pass;
	j_sess.set_roster = set_roster;
	j_setup_filter(&j_sess);

	iks_connect_tcp(j_sess.prs, host_name, j_config.port);
	// switch (e) {
	// 	case IKS_OK:
	// 		break;
	// 	case IKS_NET_NODNS:
	// 		j_error ("hostname lookup failed");
	// 	case IKS_NET_NOCONN:
	// 		j_error ("connection failed");
	// 	default:
	// 		j_error ("io error");
	// }

	// sess.counter = opt_timeout;
	// while (1) {
	// 	e = iks_recv (sess.prs, 1);
	// 	if (IKS_HOOK == e) break;
	// 	if (IKS_NET_TLSFAIL == e) j_error ("tls handshake failed");
	// 	if (IKS_OK != e) j_error ("io error");
	// 	sess.counter--;
	// 	if (sess.counter == 0) j_error ("network timeout");
	// }
	// iks_parser_delete (sess.prs);
}

void ICACHE_FLASH_ATTR
xmpp_init(struct jabber_config *config)
{
	ec_log("xmpp_init \r\n");
	if (config != NULL)
	{
		char full_jid[32] = {0};
		// MARK: XMPP配置是读取的全局配置信息 所有这里不用进行设置
		// os_memset(&j_config, 0x0, sizeof(&j_config));
		// MARK: 不能直接拷贝
		// os_memcpy(&j_config, config, sizeof(&j_config));
		{

			// ec_log("j_config ip %d \r\n", j_config.ip.addr);
			// ec_log("j_config port %d \r\n", config->port);
			// ec_log("j_config resources %s \r\n", j_config.resources);
			// ec_log("j_config username %s \r\n", j_config.username);
			// ec_log("j_config password %s \r\n", j_config.password);
			// ec_log("j_config app_username %s \r\n", j_config.app_username);
			// ec_log("j_config domain %s \r\n", j_config.domain);
			// ec_log("j_config host_name %s\r\n", j_config.host_name);

			// 单个初始化
			j_config.ip.addr = config->ip.addr;
			j_config.port = config->port;

			os_memcpy(j_config.resources, EC_VERSION, os_strlen(EC_VERSION));
			os_memcpy(j_config.username, config->username, sizeof(config->username));
			os_memcpy(j_config.password, config->password, sizeof(config->password));
			os_memcpy(j_config.app_username, config->app_username, sizeof(config->app_username));
			os_memcpy(j_config.domain, config->domain, sizeof(config->domain));
			os_memcpy(j_config.host_name, config->host_name, sizeof(config->host_name));

			// ec_log("j_config ip %d \r\n", j_config.ip.addr);
			// ec_log("j_config port %d \r\n", j_config.port);
			// ec_log("j_config resources %s \r\n", j_config.resources);
			// ec_log("j_config username %s \r\n", j_config.username);
			// ec_log("j_config password %s \r\n", j_config.password);
			// ec_log("j_config app_username %s \r\n", j_config.app_username);
			// ec_log("j_config domain %s \r\n", j_config.domain);
			// ec_log("j_config host_name %s\r\n", j_config.host_name);
		}
		// ec_log(" config is \r\n");
		os_sprintf(full_jid, "%s@%s/%s", config->username, config->domain, EC_VERSION);

		// NOTE: 这里由于host不可能是null 必须要加上长度判断
		if (os_strlen(config->host_name) > 3)
		{
			j_connect(full_jid, config->password, 0, config->username, config->host_name);
		}
		else
		{
			j_connect(full_jid, config->password, 0, config->username, NULL);
		}

		return;
	}
	else if (jabber_get_config(&j_config))
	{
		char full_jid[32] = {0};
		ec_log("read config\r\n");
		os_sprintf(full_jid, "%s@%s/%s", j_config.username, j_config.domain, j_config.resources);
		j_connect(full_jid, j_config.password, 0, j_config.username, j_config.host_name);
	}
	else
	{
		// FIXME: 系统致命错误
		ec_log("sys error \r\n");
	}
}