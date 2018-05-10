#include <stdio.h>
#include "ngx_module.h"
#include "ngx_http_config.h"

static ngx_command_t  ngx_http_commands[] = {
	{ 
		ngx_string("http_back_end"),
		NGX_MAIN_CONF | NGX_CONF_BLOCK | NGX_CONF_NOARGS,
		NULL,
		0,
		0,
		NULL 
	},

	ngx_null_command
};

static ngx_http_module_t ngx_http_module_ctx = 
{
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

ngx_module_t ngx_http_back_end_module =
{
	
	NULL,


};


