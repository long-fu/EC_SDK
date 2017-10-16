#ifndef e_http_h
#define e_http_h

typedef void (* http_failure_callback)(int error);
typedef void (* http_success_callback)(char *data, int len);

int
http_request(const char *url, 
    int type, // type 表示请求类型
    const char *body, 
    http_success_callback success_handl, 
    http_failure_callback failure_handl);
   
#endif