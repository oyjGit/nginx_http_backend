#ifndef __CUSTOMER_BACKEND_H__
#define __CUSTOMER_BACKEND_H__

#include "ngx_string.h"

typedef struct mysql_connect_conf_s
{
	ngx_str_t host;
	ngx_str_t user_name;
	ngx_str_t user_pwd;
	ngx_str_t db_name;
	int16_t port;
}mysql_connect_conf_t;

typedef struct redis_connect_conf_s 
{
	ngx_str_t host;
	int16_t port;
	ngx_str_t pwd;
}redis_connect_conf_t;

typedef struct wx_backend_info_s 
{
	ngx_str_t	app_id;
	ngx_str_t	secret;
}wx_backend_info_t;

typedef struct cutomer_module_conf_s
{
	mysql_connect_conf_t	mysql_info;
	void*					mysql_client;

	redis_connect_conf_t	redis_info;
	void*					redis_client;

	wx_backend_info_t		wx_backend_info;
}ngx_http_cutomer_module_conf_t;

#endif // !__CUSTOMER_BACKEND_H__
