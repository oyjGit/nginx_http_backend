#ifndef __LOGIN_H__
#define __LOGIN_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "ngx_http.h"

ngx_int_t ngx_http_customer_manager_login(ngx_http_request_t* r);

ngx_int_t ngx_http_customer_manager_wx_login(ngx_http_request_t *r);

#ifdef __cplusplus
}
#endif

#endif // !__LOGIN_H__
