#include "http_module_callback.h"
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
	printf("http_preconfiguration...\n");
	return 0;
}

void* http_create_main_conf(ngx_conf_t *cf)
{
	printf("http_create_main_conf...\n");
	return NULL;
}

char* http_init_main_conf(ngx_conf_t *cf, void *conf)
{
	printf("http_preconfiguration...\n");
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