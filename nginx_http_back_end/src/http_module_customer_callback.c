#include "http_module_customer_callback.h"
#include "customer_backend_typedef.h"
#include "ngx_module.h"
#include "ngx_http.h"
#include "mysql_helper.h"
#include <stdio.h>

extern ngx_module_t ngx_http_back_end_module;

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
	mysql_connect_conf_t* conn = &ct->mysql_info;

	fprintf(stderr, "got mysql info:\n\t\thost:%s\n\t\tport=%d\n\t\tuser_name:%s\n\t\tuser_pwd:%s\n\t\tdb_name:%s\n\t\t\n",
		conn->host.data, conn->port, conn->user_name.data, conn->user_pwd.data, conn->db_name.data);

	if (conn->host.data == NULL || conn->user_name.data == NULL
		|| conn->user_pwd.data == NULL || conn->db_name.data == NULL)
	{
		return NGX_ERROR;
	}
	ct->mysql_client = connect_mysql_db((char*)(conn->host.data), conn->port, (char*)(conn->user_name.data), 
										(char*)(conn->user_pwd.data), (char*)(conn->db_name.data));

	if (NULL == ct->mysql_client)
	{
		ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "connect to mysql server failed");
		return NGX_ERROR;
	}
	return NGX_OK;
}

void* http_create_main_conf(ngx_conf_t *cf)
{
	printf("http_create_main_conf...\n");
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