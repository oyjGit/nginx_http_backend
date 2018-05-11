#include "ngx_module.h"
#include "ngx_http.h"
#include "mysql_helper.h"

static char* ngx_http_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

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

static ngx_command_t  ngx_http_commands[] = {
	{ 
		ngx_string("mytest"),
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
		ngx_http_mytest,
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

// 请求的所有信息都存入ngx_http_request_t结构体中  
static ngx_int_t ngx_http_mytest_handler(ngx_http_request_t *r)
{
	// 请求的方法必须为GET或者HEAD
	if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD)))
		return NGX_HTTP_NOT_ALLOWED;

	// 丢弃请求中的包体
	ngx_int_t rc = ngx_http_discard_request_body(r);
	if (rc != NGX_OK)
		return rc;

	printf("start to connect...");
	connect_mysql_server("192.168.6.100", 3306, "root", "Root/123", "iot_db_video");
	printf("end connect...");

	int count = exec_sql("select * from video_plan;");
	printf("end exec sql...");

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


static char* ngx_http_mytest(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t *clcf;

	// 找到mytest配置项所属的配置块  
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	// 设置处理请求的方法，HTTP框架在处理用户请求进行到NGX_HTTP_CONTENT_PHASE阶段时  
	// 如果主机域名、URI和mytest模块所在配置块名称相同，就会调用函数ngx_http_mytest_handler  
	clcf->handler = ngx_http_mytest_handler;

	return NGX_CONF_OK;
}
