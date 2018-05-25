#include "ngx_module.h"
#include "ngx_http.h"
#include "mysql_helper.h"
#include "customer_backend_typedef.h"
#include "http_module_customer_callback.h"

static char* ngx_http_conf_set_callback(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char* ngx_http_get_mysql_conf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char* ngx_http_get_single_conf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_http_module_t ngx_http_module_ctx =
{
	NULL,//http_preconfiguration,
	NULL,//http_postconfiguration,
	http_create_main_conf,
	NULL,//http_init_main_conf,
	NULL,//http_create_srv_conf,
	NULL,//http_merge_srv_conf,
	NULL,//http_create_loc_conf,
	NULL,//http_merge_loc_conf,
};

static ngx_command_t  ngx_http_commands[] = {
	
	{ 
		ngx_string("customer_manager"),
		//配置项只能出现在location块中并且配置参数数量为1个
		//NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
		ngx_http_conf_set_callback,
		0,
		0,
		NULL 
	},

	{
		ngx_string("test"),
		//配置项只能出现在location块中并且配置参数数量为1个
		//NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_ANY,
		ngx_http_get_single_conf,
		0,
		0,
		NULL
	},

	{
		ngx_string("mysql"),
		//配置项只能出现在location块中并且配置参数数量为1个
		//NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
		NGX_HTTP_MAIN_CONF | NGX_CONF_BLOCK | NGX_CONF_NOARGS,
		ngx_http_get_mysql_conf,
		NGX_HTTP_MAIN_CONF_OFFSET,
		offsetof(ngx_http_cutomer_module_conf_t, mysql_info),
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

// 请求的所有信息都存入ngx_http_request_t结构体中  
static ngx_int_t ngx_http_customer_manager_handler(ngx_http_request_t *r)
{
	// 请求的方法必须为GET或者HEAD
	if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD)))
		return NGX_HTTP_NOT_ALLOWED;

	// 丢弃请求中的包体
	ngx_int_t rc = ngx_http_discard_request_body(r);
	if (rc != NGX_OK)
		return rc;

	ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log , 0, "%s", "start to connect...\n");
	int ret = connect_mysql_server("localhost", 3306, "root", "123456", "mytest");
	ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "end to connect..., ret=%d\n", ret);

	int count = exec_sql("select * from test_table;");
	ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "%s", "end exec sql...\n");

	char res[1024] = { 0 };
	snprintf(res, 1024, "{\"data\":\"hello world, result=%d\"}", count);

	//ngx_str_t type = ngx_string("text/plain");
	//ngx_str_t response = ngx_string("{\"data\":\"hello world\"}");    // 包体内容
	ngx_str_t type = ngx_string("text/json");
	ngx_str_t response = { strlen(res), (u_char*)res };

														// 设置响应的HTTP头部
	r->headers_out.status = NGX_HTTP_OK;           // 返回的响应码
	r->headers_out.content_length_n = response.len;    // 响应包体长度
	r->headers_out.content_type = type;                // Content-Type  

	rc = ngx_http_send_header(r); // 发送HTTP头部
	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
		return rc;

	// 如果响应不包含包体，则在此处可以直接返回rc  

	// 分配响应包体空间，因为是异步发送，所以不要从栈中获得空间
	ngx_buf_t *b = ngx_create_temp_buf(r->pool, response.len);

	if (b == NULL)
		return NGX_HTTP_INTERNAL_SERVER_ERROR;

	ngx_memcpy(b->pos, response.data, response.len);
	b->last = b->pos + response.len;  // 指向数据末尾
	b->last_buf = 1;                    // 声明这是最后一块缓冲区

	ngx_chain_t out;
	out.buf = b;
	out.next = NULL;

	return ngx_http_output_filter(r, &out);   // 向用户发送响应包  
}


static char* ngx_http_conf_set_callback(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t *clcf;

	printf("ngx_http_conf_set_callback...\n");

	// 找到customer_manager配置项所属的配置块  
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	// 设置处理请求的方法，HTTP框架在处理用户请求进行到NGX_HTTP_CONTENT_PHASE阶段时  
	// 如果主机域名、URI和mytest模块所在配置块名称相同，就会调用函数ngx_http_mytest_handler  
	clcf->handler = ngx_http_customer_manager_handler;

	return NGX_CONF_OK;
}

static char* ngx_http_get_mysql_conf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
	//ngx_http_cutomer_module_conf_t* local_conf = conf;
	//char* rv = ngx_conf_set_str_slot(cf, cmd, conf);
		//ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "host:%s", local_conf->hello_string.data);
	
	/* cf->args是1个ngx_array_t队列，它的成员都是ngx_str_t结构。我们用value指向ngx_array_t的elts内容，其中
	value[1]就是第1个参数，同理，value[2]是第2个参数
	*/

	ngx_str_t *value = cf->args->elts;
	printf("got mysql conf, args num=%lu,name=%s,value[0]=%s\n", cf->args->nelts, cf->name, value[3].data);
	//// ngx_array_t的nelts表示参数的个数
	//if (cf->args->nelts > 1)
	//{
	//	// 直接赋值即可，ngx_str_t结构只是指针的传递
	//	mycf->my_config_str = value[1];
	//}

	//if (cf->args->nelts > 2)
	//{
	//	// 将字符串形式的第2个参数转为整型
	//	mycf->my_config_num = ngx_atoi(value[2].data, value[2].len);
	//	/*如果字符串转化整型失败，将报“invalid number”错误，Nginx启动失败*/
	//	if (mycf->my_config_num == NGX_ERROR) {
	//		return "invalid number";
	//	}
	//}

	return NGX_CONF_OK;
	//return rv;
}

static char* ngx_http_get_single_conf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	printf("read a sinle file\n");
	return NGX_CONF_OK;
}