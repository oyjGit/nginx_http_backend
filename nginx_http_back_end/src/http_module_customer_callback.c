#include "http_module_customer_callback.h"
#include "customer_backend_typedef.h"
#include "mysql_helper.h"
#include "RedisUtil.h"
#include "ngx_http_back_end_module.h"
#include <stdio.h>


#ifdef __cplusplus
extern "C"{
#endif

ngx_int_t http_preconfiguration(ngx_conf_t *cf)
{
	printf("http_preconfiguration...\n");
	return 0;
}

ngx_int_t http_postconfiguration(ngx_conf_t *cf)
{
	ngx_http_cutomer_module_conf_t* ct = ngx_http_conf_get_module_main_conf(cf, ngx_http_back_end_module);
	mysql_connect_conf_t* mysql = &ct->mysql_info;
	redis_connect_conf_t* redis = &ct->redis_info;
	wx_backend_info_t* wx_info = &ct->wx_backend_info;

	fprintf(stderr, "got mysql info:\n\t\thost:%s\n\t\tport=%d\n\t\tuser_name:%s\n\t\tuser_pwd:%s\n\t\tdb_name:%s\n\t\t\n",
		mysql->host.data, mysql->port, mysql->user_name.data, mysql->user_pwd.data, mysql->db_name.data);

	fprintf(stderr, "got redis info:\n\t\thost:%s\n\t\tport=%d\n\t\tpwd:%s\n", redis->host.data, redis->port, redis->pwd.data);

	if (mysql->host.data == NULL || mysql->user_name.data == NULL
		|| mysql->user_pwd.data == NULL || mysql->db_name.data == NULL)
	{
		return NGX_ERROR;
	}

	if (redis->host.data == NULL || redis->pwd.data == NULL) 
	{
		return NGX_ERROR;
	}

	if (wx_info->app_id.data == NULL || wx_info->secret.data == NULL) 
	{
		return NGX_ERROR;
	}

	ct->mysql_client = connect_mysql_db((char*)(mysql->host.data), mysql->port, (char*)(mysql->user_name.data), 
										(char*)(mysql->user_pwd.data), (char*)(mysql->db_name.data));

	if (NULL == ct->mysql_client)
	{
		ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "connect to mysql server failed");
		return NGX_ERROR;
	}

	ct->redis_client = redis_connect((char*)redis->host.data, redis->port, (char*)redis->pwd.data);
	if (NULL == ct->redis_client) 
	{
		disconnect_mysql_db(ct->mysql_client);
		ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "connect to redis server failed");
		return NGX_ERROR;
	}

	return NGX_OK;
}

void* http_create_main_conf(ngx_conf_t *cf)
{
	fprintf(stderr, "http_create_main_conf...\n");
	ngx_http_cutomer_module_conf_t* main_conf = NULL;
	main_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_cutomer_module_conf_t));
	if (main_conf == NULL)
	{
		return NULL;
	}

	ngx_str_null(&(main_conf->mysql_info.host));
	ngx_str_null(&(main_conf->mysql_info.db_name));
	ngx_str_null(&(main_conf->mysql_info.user_name));
	ngx_str_null(&(main_conf->mysql_info.user_pwd));
	main_conf->mysql_info.port = 3306;

	ngx_str_null(&(main_conf->redis_info.host));
	ngx_str_null(&(main_conf->redis_info.pwd));
	main_conf->redis_info.port = 6379;

	ngx_str_null(&(main_conf->wx_backend_info.app_id));
	ngx_str_null(&(main_conf->wx_backend_info.secret));

	return main_conf;
}

char* http_init_main_conf(ngx_conf_t *cf, void *conf)
{
	printf("http_init_main_conf...\n");
	return NULL;
}

void* http_create_srv_conf(ngx_conf_t *cf)
{
	printf("http_create_srv_conf...\n");
	return NULL;
}

char* http_merge_srv_conf(ngx_conf_t *cf, void *prev, void *conf)
{
	printf("http_merge_srv_conf...\n");
	return NULL;
}

void* http_create_loc_conf(ngx_conf_t *cf)
{
	printf("http_create_loc_conf...\n");
	
	return NULL;
}

char* http_merge_loc_conf(ngx_conf_t *cf, void *prev, void *conf)
{
	printf("http_merge_loc_conf...\n");
	return NULL;
}

#ifdef __cplusplus
};
#endif