#include "http_module_customer_callback.h"
#include "customer_backend_typedef.h"
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
	printf("http_postconfiguration...\n");
	return 0;
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
	main_conf->mysql_info.port = 0;

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