#ifndef __HTTP_MODULE_CALLBACK_H__
#define __HTTP_MODULE_CALLBACK_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "ngx_core.h"

ngx_int_t http_preconfiguration(ngx_conf_t *cf);

ngx_int_t http_postconfiguration(ngx_conf_t *cf);

void* http_create_main_conf(ngx_conf_t *cf);

char* http_init_main_conf(ngx_conf_t *cf, void *conf);

void* http_create_srv_conf(ngx_conf_t *cf);

char* http_merge_srv_conf(ngx_conf_t *cf, void *prev, void *conf);

void* http_create_loc_conf(ngx_conf_t *cf);

char* http_merge_loc_conf(ngx_conf_t *cf, void *prev, void *conf);

#ifdef __cplusplus
};
#endif

#endif