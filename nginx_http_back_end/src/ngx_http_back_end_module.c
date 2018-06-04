#include "ngx_module.h"
#include "ngx_http.h"
#include "mysql_helper.h"
#include "RedisUtil.h"
#include "customer_backend_typedef.h"
#include "http_module_customer_callback.h"
#include "Login.h"


static char* ngx_http_get_mysql_conf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char* ngx_http_get_mysql_host(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char* ngx_http_get_mysql_port(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char* ngx_http_get_mysql_user_name(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char* ngx_http_get_mysql_user_pwd(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char* ngx_http_get_mysql_db_name(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static char* ngx_http_get_redis_host(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char* ngx_http_get_redis_pwd(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char* ngx_http_get_redis_port(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static char* ngx_http_set_login_call_back(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char* ngx_http_set_wx_login_call_back(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_http_module_t ngx_http_module_ctx =
{
	NULL,//http_preconfiguration,
	http_postconfiguration,
	http_create_main_conf,
	NULL,//http_init_main_conf,
	NULL,//http_create_srv_conf,
	NULL,//http_merge_srv_conf,
	NULL,//http_create_loc_conf,
	NULL,//http_merge_loc_conf,
};

static ngx_command_t  ngx_http_commands[] = {

	{
		ngx_string("mysql"),
		NGX_HTTP_MAIN_CONF | NGX_CONF_BLOCK | NGX_CONF_NOARGS,
		ngx_http_get_mysql_conf,
		NGX_HTTP_MAIN_CONF_OFFSET,
		offsetof(ngx_http_cutomer_module_conf_t, mysql_info),
		NULL
	},

	{
		ngx_string("mysql_host"),
		NGX_HTTP_MAIN_CONF | NGX_CONF_ANY,
		//NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_ANY | NGX_CONF_TAKE1,
		ngx_http_get_mysql_host,
		NGX_HTTP_MAIN_CONF_OFFSET,
		offsetof(ngx_http_cutomer_module_conf_t, mysql_info),
		NULL
	},

	{
		ngx_string("mysql_port"),
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_ANY,
		//NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_ANY | NGX_CONF_TAKE1,
		ngx_http_get_mysql_port,
		NGX_HTTP_MAIN_CONF_OFFSET,
		offsetof(ngx_http_cutomer_module_conf_t, mysql_info),
		NULL
	},

	{
		ngx_string("mysql_user_name"),
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_ANY,
		//NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_ANY | NGX_CONF_TAKE1,
		ngx_http_get_mysql_user_name,
		NGX_HTTP_MAIN_CONF_OFFSET,
		offsetof(ngx_http_cutomer_module_conf_t, mysql_info),
		NULL
	},

	{
		ngx_string("mysql_user_pwd"),
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_ANY,
		//NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_ANY | NGX_CONF_TAKE1,
		ngx_http_get_mysql_user_pwd,
		NGX_HTTP_MAIN_CONF_OFFSET,
		offsetof(ngx_http_cutomer_module_conf_t, mysql_info),
		NULL
	},

	{
		ngx_string("mysql_db_name"),
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_ANY,
		//NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_ANY | NGX_CONF_TAKE1,
		ngx_http_get_mysql_db_name,
		NGX_HTTP_MAIN_CONF_OFFSET,
		offsetof(ngx_http_cutomer_module_conf_t, mysql_info),
		NULL
	},

	{
		ngx_string("redis_host"),
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_ANY,
		//NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_ANY | NGX_CONF_TAKE1,
		ngx_http_get_redis_host,
		NGX_HTTP_MAIN_CONF_OFFSET,
		offsetof(ngx_http_cutomer_module_conf_t, redis_info),
		NULL
	},

	{
		ngx_string("redis_port"),
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_ANY,
		//NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_ANY | NGX_CONF_TAKE1,
		ngx_http_get_redis_port,
		NGX_HTTP_MAIN_CONF_OFFSET,
		offsetof(ngx_http_cutomer_module_conf_t, redis_info),
		NULL
	},

	{
		ngx_string("redis_pwd"),
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_ANY,
		//NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_CONF_ANY | NGX_CONF_TAKE1,
		ngx_http_get_redis_pwd,
		NGX_HTTP_MAIN_CONF_OFFSET,
		offsetof(ngx_http_cutomer_module_conf_t, redis_info),
		NULL
	},

	{
		ngx_string("login"),
		//配置项只能出现在location块中并且配置参数数量为1个
		//NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
		ngx_http_set_login_call_back,
		0,
		0,
		NULL
	},

	{
		ngx_string("wx_bp_login"),
		//配置项只能出现在location块中并且配置参数数量为1个
		//NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
		ngx_http_set_wx_login_call_back,
		0,
		0,
		NULL
	},

	ngx_null_command
};

ngx_module_t ngx_http_back_end_module =
{
	NGX_MODULE_V1,					// 0,0,0,0,0,0,1  
	&ngx_http_module_ctx,
	ngx_http_commands,
	NGX_HTTP_MODULE,				//定义模块类型

	/* Nginx在启动和退出时会调用下面7个回调方法 */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NGX_MODULE_V1_PADDING,  // 0,0,0,0,0,0,0,0,保留字段
};

/*子请求结束时的回调函数*/
static ngx_int_t wx_login_subrequest_post_handler(ngx_http_request_t *r, void *data, ngx_int_t rc)
{
	printf("start to subrequest\n");
	return 0;
}

static void ngx_http_customer_manager_login_handler(ngx_http_request_t *r)
{
	//ngx_http_cutomer_module_conf_t* cf = ngx_http_get_module_main_conf(r, ngx_http_back_end_module);
	ngx_http_request_body_t* body = r->request_body;

	size_t body_len = body->bufs->buf->last - body->bufs->buf->pos;
	ngx_str_t body_content = { body_len, body->bufs->buf->pos };

	if (0 == body_len)
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "get login body size is zero");
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

    /*子请求的回调方法将在此结构体中设置*/
    ngx_http_post_subrequest_t *psr=ngx_palloc(r->pool, sizeof(ngx_http_post_subrequest_t));
    if (psr == NULL)
    {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "alloc ngx_http_post_subrequest_t struct failed");
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
    }
	psr->handler = wx_login_subrequest_post_handler;
	/*构造子请求*/
	/*sina服务器要求*/
	ngx_str_t sub_prefix = ngx_string("/wxMiniBuyPhone/login");
	/*URL构造=前缀+参数*/
	ngx_str_t sub_location;
	sub_location.len = sub_prefix.len + r->args.len;
	sub_location.data = ngx_palloc(r->pool, sub_location.len);
	ngx_snprintf(sub_location.data, sub_location.len,
		"%V%V", &sub_prefix, &r->args);

	/*sr即为子请求*/
	ngx_http_request_t *sr;
	/*创建子请求*/
	/*参数分别是：*/
	//1.父请求 2.请求URI 3.子请求的URI参数 4.返回创建好的子请求
	//5.子请求结束的回调
	//6.subrequest_in_memory 标识
	ngx_int_t rc = ngx_http_subrequest(r, &sub_location, NULL, &sr, psr, NGX_HTTP_SUBREQUEST_IN_MEMORY);

	if (rc != NGX_OK)
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "alloc ngx_http_subrequest failed");
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	/*父请求不会被销毁，而是等待再次被激活*/
	return;
	
	// 设置响应的HTTP头部
	ngx_str_t response = { 5, (u_char*)"hello" };
	r->headers_out.status = NGX_HTTP_OK;           // 返回的响应码
	r->headers_out.content_length_n = response.len;    // 响应包体长度
	ngx_str_set(&r->headers_out.content_type, "text/json");
	// 如果响应不包含包体，则在此处可以直接返回rc  
	rc = ngx_http_send_header(r); // 发送HTTP头部
	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
	{
		ngx_http_finalize_request(r, rc);
		return;
	}
	
	// 分配响应包体空间，因为是异步发送，所以不要从栈中获得空间
	ngx_buf_t *b = ngx_create_temp_buf(r->pool, response.len);
	if (b == NULL)
	{
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	ngx_memcpy(b->pos, response.data, response.len);
	b->last = b->pos + response.len;  // 指向数据末尾
	b->last_buf = 1;                    // 声明这是最后一块缓冲区

	ngx_chain_t out;
	out.buf = b;
	out.next = NULL;
	rc = ngx_http_output_filter(r, &out);   // 向用户发送响应包  

	//必须调用ngx_http_finalize_request
	ngx_http_finalize_request(r, rc);
}

static ngx_int_t ngx_http_customer_manager_wx_login(ngx_http_request_t *r)
{
	printf("ngx_http_customer_manager_wx_login...\n");
	return NGX_OK;
}

static char* ngx_http_set_login_call_back(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t *clcf;
	// 找到customer_manager配置项所属的配置块  
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	// 设置处理请求的方法，HTTP框架在处理用户请求进行到NGX_HTTP_CONTENT_PHASE阶段时  
	// 如果主机域名、URI和mytest模块所在配置块名称相同，就会调用函数ngx_http_mytest_handler 
	clcf->handler = ngx_http_customer_manager_login;

	return NGX_CONF_OK;
}

static char* ngx_http_set_wx_login_call_back(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t *clcf;
	// 找到customer_manager配置项所属的配置块  
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	// 设置处理请求的方法，HTTP框架在处理用户请求进行到NGX_HTTP_CONTENT_PHASE阶段时  
	// 如果主机域名、URI和mytest模块所在配置块名称相同，就会调用函数ngx_http_mytest_handler 
	clcf->handler = ngx_http_customer_manager_wx_login;

	return NGX_CONF_OK;
}

static char* ngx_http_get_mysql_conf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
	ngx_conf_log_error(NGX_LOG_DEBUG, cf, 0, "find mysql conf block");
	return NGX_CONF_OK;
}

static char* ngx_http_get_mysql_host(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	char  *p = conf;
	ngx_str_t        *field, *value;
	field = &(((mysql_connect_conf_t *)(p + cmd->offset))->host);
	value = cf->args->elts;
	*field = value[1];
	return NGX_CONF_OK;
}

static char* ngx_http_get_mysql_port(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
	char  *p = conf;
	ngx_str_t        *value;
	int16_t* field = &(((mysql_connect_conf_t *)(p + cmd->offset))->port);
	value = cf->args->elts;
	*field = ngx_atoi(value[1].data, value[1].len);
	return NGX_CONF_OK;
}

static char* ngx_http_get_mysql_user_name(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
	char  *p = conf;
	ngx_str_t        *field, *value;
	field = &(((mysql_connect_conf_t *)(p + cmd->offset))->user_name);
	value = cf->args->elts;
	*field = value[1];
	return NGX_CONF_OK;
}

static char* ngx_http_get_mysql_user_pwd(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
	char  *p = conf;
	ngx_str_t        *field, *value;
	field = &(((mysql_connect_conf_t *)(p + cmd->offset))->user_pwd);
	value = cf->args->elts;
	*field = value[1];
	return NGX_CONF_OK;
}

static char* ngx_http_get_mysql_db_name(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
	char  *p = conf;
	ngx_str_t        *field, *value;
	mysql_connect_conf_t* conn = (mysql_connect_conf_t *)(p + cmd->offset);
	field = &(conn->db_name);
	value = cf->args->elts;
	*field = value[1];
	return NGX_CONF_OK;
}

static char* ngx_http_get_redis_host(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
	char  *p = conf;
	ngx_str_t        *field, *value;
	redis_connect_conf_t* conn = (redis_connect_conf_t *)(p + cmd->offset);
	field = &(conn->host);
	value = cf->args->elts;
	*field = value[1];
	return NGX_CONF_OK;
}

static char* ngx_http_get_redis_pwd(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	char  *p = conf;
	ngx_str_t        *field, *value;
	redis_connect_conf_t* conn = (redis_connect_conf_t *)(p + cmd->offset);
	field = &(conn->pwd);
	value = cf->args->elts;
	*field = value[1];
	return NGX_CONF_OK;
}

static char* ngx_http_get_redis_port(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
	char  *p = conf;
	ngx_str_t        *value;
	int16_t* field = &(((redis_connect_conf_t *)(p + cmd->offset))->port);
	value = cf->args->elts;
	*field = ngx_atoi(value[1].data, value[1].len);
	return NGX_CONF_OK;
}