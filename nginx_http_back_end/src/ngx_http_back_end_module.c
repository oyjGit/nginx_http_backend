#include "ngx_module.h"
#include "ngx_http.h"
#include "mysql_helper.h"
#include "RedisUtil.h"
#include "customer_backend_typedef.h"
#include "http_module_customer_callback.h"


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
		//������ֻ�ܳ�����location���в������ò�������Ϊ1��
		//NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
		ngx_http_set_login_call_back,
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
	NGX_HTTP_MODULE,				//����ģ������

	/* Nginx���������˳�ʱ���������7���ص����� */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NGX_MODULE_V1_PADDING,  // 0,0,0,0,0,0,0,0,�����ֶ�
};

static void ngx_http_customer_manager_login_handler(ngx_http_request_t *r)
{
	//ngx_http_cutomer_module_conf_t* cf = ngx_http_get_module_main_conf(r, ngx_http_back_end_module);
	//ngx_http_request_body_t* body = r->request_body;
	
	// ������Ӧ��HTTPͷ��
	ngx_str_t response = { 5, (u_char*)"hello" };
	r->headers_out.status = NGX_HTTP_OK;           // ���ص���Ӧ��
	r->headers_out.content_length_n = response.len;    // ��Ӧ���峤��
	ngx_str_set(&r->headers_out.content_type, "text/json");
	// �����Ӧ���������壬���ڴ˴�����ֱ�ӷ���rc  
	ngx_int_t rc = ngx_http_send_header(r); // ����HTTPͷ��
	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
	{
		ngx_http_finalize_request(r, rc);
		return;
	}
	
	// ������Ӧ����ռ䣬��Ϊ���첽���ͣ����Բ�Ҫ��ջ�л�ÿռ�
	ngx_buf_t *b = ngx_create_temp_buf(r->pool, response.len);
	if (b == NULL)
	{
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	ngx_memcpy(b->pos, response.data, response.len);
	b->last = b->pos + response.len;  // ָ������ĩβ
	b->last_buf = 1;                    // �����������һ�黺����

	ngx_chain_t out;
	out.buf = b;
	out.next = NULL;
	rc = ngx_http_output_filter(r, &out);   // ���û�������Ӧ��  

	//�������ngx_http_finalize_request
	ngx_http_finalize_request(r, rc);
}

// �����������Ϣ������ngx_http_request_t�ṹ����  
static ngx_int_t ngx_http_customer_manager_login(ngx_http_request_t *r)
{
	// ����ķ�������ΪPOST
	if (!(r->method & (NGX_HTTP_POST)))
		return NGX_HTTP_NOT_ALLOWED;

	ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log , 0, "request uri:%s", r->uri.data);
	
	ngx_int_t rc = ngx_http_read_client_request_body(r, ngx_http_customer_manager_login_handler);

	if (rc >= NGX_HTTP_SPECIAL_RESPONSE) 
	{
		return rc;
	}

	return NGX_DONE;

	//ngx_str_t type = ngx_string("text/plain");
	//ngx_str_t response = ngx_string("{\"data\":\"hello world\"}");    // ��������
	//ngx_str_t type = ngx_string("text/json");
	//ngx_str_t response = { strlen(res), (u_char*)res };

	//													// ������Ӧ��HTTPͷ��
	//r->headers_out.status = NGX_HTTP_OK;           // ���ص���Ӧ��
	//r->headers_out.content_length_n = response.len;    // ��Ӧ���峤��
	//r->headers_out.content_type = type;                // Content-Type  

	//rc = ngx_http_send_header(r); // ����HTTPͷ��
	//if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
	//	return rc;

	//// �����Ӧ���������壬���ڴ˴�����ֱ�ӷ���rc  

	//// ������Ӧ����ռ䣬��Ϊ���첽���ͣ����Բ�Ҫ��ջ�л�ÿռ�
	//ngx_buf_t *b = ngx_create_temp_buf(r->pool, response.len);

	//if (b == NULL)
	//	return NGX_HTTP_INTERNAL_SERVER_ERROR;

	//ngx_memcpy(b->pos, response.data, response.len);
	//b->last = b->pos + response.len;  // ָ������ĩβ
	//b->last_buf = 1;                    // �����������һ�黺����

	//ngx_chain_t out;
	//out.buf = b;
	//out.next = NULL;

	//return ngx_http_output_filter(r, &out);   // ���û�������Ӧ��  
}


static char* ngx_http_set_login_call_back(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t *clcf;
	// �ҵ�customer_manager���������������ÿ�  
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	// ���ô�������ķ�����HTTP����ڴ����û�������е�NGX_HTTP_CONTENT_PHASE�׶�ʱ  
	// �������������URI��mytestģ���������ÿ�������ͬ���ͻ���ú���ngx_http_mytest_handler 
	clcf->handler = ngx_http_customer_manager_login;

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