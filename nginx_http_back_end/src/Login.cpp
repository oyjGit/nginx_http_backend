#include "Login.h"
#include "LoginParam.h"
#include "WXSessionInfo.h"
#include "ngx_http_back_end_module.h"

#ifdef __cplusplus
extern "C"{
#endif

/*�������ص�*/
static void wx_login_subrequest_post_father_handler(ngx_http_request_t * r)
{
	/*���û�з���200��ֱ�ӰѴ����뷢���û�*/
	if (r->headers_out.status != NGX_HTTP_OK)
	{
		ngx_http_finalize_request(r, r->headers_out.status);
		return;
	}

	// ������Ӧ��HTTPͷ��
	ngx_str_t response = { 5, (u_char*)"hello2" };
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

/*���������ʱ�Ļص�����*/
static ngx_int_t wx_login_subrequest_post_handler(ngx_http_request_t *r, void *data, ngx_int_t rc)
{
	/*��ǰ������������*/
	ngx_http_request_t          *pr = r->parent;
	pr->headers_out.status = r->headers_out.status;
	/*���ʷ������ɹ�����ʼ��������*/
	if (NGX_HTTP_OK == r->headers_out.status)
	{
		ngx_str_t bak = { (size_t)(r->upstream->buffer.last - r->upstream->buffer.pos), r->upstream->buffer.pos };
		WXSessionInfo info;
		std::string backStr = (const char*)bak.data;
		if (!decode<WXSessionInfo>(backStr, info))
		{
			ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "get data from wx success, but parse json failed, json value=%s\n", backStr.c_str());
			pr->headers_out.status = NGX_HTTP_INTERNAL_SERVER_ERROR;
		}
		ngx_log_error(NGX_LOG_DEBUG, r->connection->log, errno, "get data from wx success, id=%s, session_key=%s\n", info.openid.c_str(), info.session_key.c_str());
	}
	/*���ø�����Ļص�����*/
	pr->write_event_handler = wx_login_subrequest_post_father_handler;
	return NGX_OK;
}

static void ngx_http_customer_manager_login_handler(ngx_http_request_t *r)
{
	ngx_http_cutomer_module_conf_t* cf = (ngx_http_cutomer_module_conf_t*)ngx_http_get_module_main_conf(r, ngx_http_back_end_module);
	ngx_http_request_body_t* body = r->request_body;

	size_t body_len = body->bufs->buf->last - body->bufs->buf->pos;
	ngx_str_t body_content = { body_len, body->bufs->buf->pos };
	if (0 == body_len)
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "get login body size is zero");
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	LoginParam codeObj;
	std::string codeJson = (const char*)body_content.data;
	if (!decode<LoginParam>(codeJson, codeObj))
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "parse login param json failed, data=%s", (const char*)codeJson.c_str());
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	if (codeObj.code.empty())
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "parse login param json got code is empty");
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	/*������Ļص��������ڴ˽ṹ��������*/
	ngx_http_post_subrequest_t *psr = (ngx_http_post_subrequest_t*)ngx_palloc(r->pool, sizeof(ngx_http_post_subrequest_t));
	if (psr == NULL)
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "alloc ngx_http_post_subrequest_t struct failed");
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}
	psr->handler = wx_login_subrequest_post_handler;
	/*����������*/
	/*URL����=ǰ׺+����*/
	static ngx_str_t sub_location = ngx_string("/wxMiniBuyPhone/login");
	static ngx_str_t app_id_str = ngx_string("appid=");
	static ngx_str_t app_secret_str = ngx_string("&secret=");
	static ngx_str_t app_code_str = ngx_string("&js_code=");
	static ngx_str_t app_type_str = ngx_string("&grant_type=authorization_code");
	ngx_str_t code_str = { codeObj.code.size(), (u_char*)codeObj.code.c_str() };
	ngx_str_t sub_args;
	sub_args.len = app_id_str.len + app_secret_str.len + app_code_str.len + app_type_str.len 
					+ cf->wx_backend_info.app_id.len + cf->wx_backend_info.secret.len + codeObj.code.size();
	sub_args.data = (u_char *)ngx_palloc(r->pool, sub_args.len);
	ngx_snprintf(sub_args.data, sub_args.len, "%V%V%V%V%V%V%V", &app_id_str, &(cf->wx_backend_info.app_id),
		&app_secret_str, &(cf->wx_backend_info.secret), &app_code_str, &code_str, &app_type_str);

	/*sr��Ϊ������*/
	ngx_http_request_t *sr;
	/*����������*/
	/*�����ֱ��ǣ�*/
	//1.������ 2.����URI 3.�������URI���� 4.���ش����õ�������
	//5.����������Ļص�
	//6.subrequest_in_memory ��ʶ
	ngx_int_t rc = ngx_http_subrequest(r, &sub_location, &sub_args, &sr, psr, NGX_HTTP_SUBREQUEST_IN_MEMORY);

	if (rc != NGX_OK)
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "alloc ngx_http_subrequest failed");
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	/*�����󲻻ᱻ���٣����ǵȴ��ٴα�����*/
	return;
}

//ngx_int_t ngx_http_customer_manager_wx_login(ngx_http_request_t *r)
//{
//	printf("start to subrequest\n");
//	return NGX_DONE;
//}

ngx_int_t ngx_http_customer_manager_login(ngx_http_request_t* r)
{
	// ����ķ�������ΪPOST
	if (!(r->method & (NGX_HTTP_POST)))
		return NGX_HTTP_NOT_ALLOWED;

	ngx_log_debug(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "request uri:%s", r->uri.data);

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

#ifdef __cplusplus
};
#endif

