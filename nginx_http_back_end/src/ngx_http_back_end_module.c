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
		//������ֻ�ܳ�����location���в������ò�������Ϊ1��
		//NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,
		ngx_http_conf_set_callback,
		0,
		0,
		NULL 
	},

	{
		ngx_string("test"),
		//������ֻ�ܳ�����location���в������ò�������Ϊ1��
		//NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
		NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_ANY,
		ngx_http_get_single_conf,
		0,
		0,
		NULL
	},

	{
		ngx_string("mysql"),
		//������ֻ�ܳ�����location���в������ò�������Ϊ1��
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

// �����������Ϣ������ngx_http_request_t�ṹ����  
static ngx_int_t ngx_http_customer_manager_handler(ngx_http_request_t *r)
{
	// ����ķ�������ΪGET����HEAD
	if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD)))
		return NGX_HTTP_NOT_ALLOWED;

	// ���������еİ���
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
	//ngx_str_t response = ngx_string("{\"data\":\"hello world\"}");    // ��������
	ngx_str_t type = ngx_string("text/json");
	ngx_str_t response = { strlen(res), (u_char*)res };

														// ������Ӧ��HTTPͷ��
	r->headers_out.status = NGX_HTTP_OK;           // ���ص���Ӧ��
	r->headers_out.content_length_n = response.len;    // ��Ӧ���峤��
	r->headers_out.content_type = type;                // Content-Type  

	rc = ngx_http_send_header(r); // ����HTTPͷ��
	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
		return rc;

	// �����Ӧ���������壬���ڴ˴�����ֱ�ӷ���rc  

	// ������Ӧ����ռ䣬��Ϊ���첽���ͣ����Բ�Ҫ��ջ�л�ÿռ�
	ngx_buf_t *b = ngx_create_temp_buf(r->pool, response.len);

	if (b == NULL)
		return NGX_HTTP_INTERNAL_SERVER_ERROR;

	ngx_memcpy(b->pos, response.data, response.len);
	b->last = b->pos + response.len;  // ָ������ĩβ
	b->last_buf = 1;                    // �����������һ�黺����

	ngx_chain_t out;
	out.buf = b;
	out.next = NULL;

	return ngx_http_output_filter(r, &out);   // ���û�������Ӧ��  
}


static char* ngx_http_conf_set_callback(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t *clcf;

	printf("ngx_http_conf_set_callback...\n");

	// �ҵ�customer_manager���������������ÿ�  
	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	// ���ô�������ķ�����HTTP����ڴ����û�������е�NGX_HTTP_CONTENT_PHASE�׶�ʱ  
	// �������������URI��mytestģ���������ÿ�������ͬ���ͻ���ú���ngx_http_mytest_handler  
	clcf->handler = ngx_http_customer_manager_handler;

	return NGX_CONF_OK;
}

static char* ngx_http_get_mysql_conf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) 
{
	//ngx_http_cutomer_module_conf_t* local_conf = conf;
	//char* rv = ngx_conf_set_str_slot(cf, cmd, conf);
		//ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "host:%s", local_conf->hello_string.data);
	
	/* cf->args��1��ngx_array_t���У����ĳ�Ա����ngx_str_t�ṹ��������valueָ��ngx_array_t��elts���ݣ�����
	value[1]���ǵ�1��������ͬ��value[2]�ǵ�2������
	*/

	ngx_str_t *value = cf->args->elts;
	printf("got mysql conf, args num=%lu,name=%s,value[0]=%s\n", cf->args->nelts, cf->name, value[3].data);
	//// ngx_array_t��nelts��ʾ�����ĸ���
	//if (cf->args->nelts > 1)
	//{
	//	// ֱ�Ӹ�ֵ���ɣ�ngx_str_t�ṹֻ��ָ��Ĵ���
	//	mycf->my_config_str = value[1];
	//}

	//if (cf->args->nelts > 2)
	//{
	//	// ���ַ�����ʽ�ĵ�2������תΪ����
	//	mycf->my_config_num = ngx_atoi(value[2].data, value[2].len);
	//	/*����ַ���ת������ʧ�ܣ�������invalid number������Nginx����ʧ��*/
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