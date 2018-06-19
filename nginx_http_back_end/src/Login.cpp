#include "Login.h"
#include "LoginParam.h"
#include "LoginBackParam.h"
#include "WXSessionInfo.h"
#include "ngx_http_back_end_module.h"
#include "ngx_string.h"
#include "sys_util.h"

/*������*/
#define WX_RESPONSE 0
#define REDIS_RESPONSE 1

typedef struct ngx_http_login_ctx_s {
	ngx_str_t response[2];
}ngx_http_login_ctx_t;

#ifdef __cplusplus
extern "C"{
#endif

static void login_done(ngx_http_request_t * r) 
{
	printf("login done, send back\n");
	/*���û�з���200��ֱ�ӰѴ����뷢���û�*/
	if (r->headers_out.status != NGX_HTTP_OK)
	{
		ngx_http_finalize_request(r, r->headers_out.status);
		return;
	}

	ngx_http_login_ctx_t *ctx = (ngx_http_login_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_back_end_module);

	WXSessionInfo info;
	std::string wxBackJson((const char*)ctx->response[WX_RESPONSE].data, ctx->response[WX_RESPONSE].len);
	if (!decode<WXSessionInfo>(wxBackJson, info))
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "decode json to WXSessionInfo object failed");
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	LoginBackParam backParam;
	backParam.openid = info.openid;
	backParam.session_id = info.openid;
	std::string backJson;
	if (!encode<true, LoginBackParam>(backParam, backJson))
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "encode json to WXSessionInfo object failed");
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	// ������Ӧ��HTTPͷ��
	r->headers_out.status = NGX_HTTP_OK;           // ���ص���Ӧ��
	r->headers_out.content_length_n = backJson.size();    // ��Ӧ���峤��
	ngx_str_set(&r->headers_out.content_type, "text/json");
	// �����Ӧ���������壬���ڴ˴�����ֱ�ӷ���rc  
	ngx_int_t rc = ngx_http_send_header(r); // ����HTTPͷ��
	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "login send header failed, rc=%d", rc);
		ngx_http_finalize_request(r, rc);
		return;
	}

	// ������Ӧ����ռ䣬��Ϊ���첽���ͣ����Բ�Ҫ��ջ�л�ÿռ�
	ngx_buf_t *b = ngx_create_temp_buf(r->pool, backJson.size());
	if (b == NULL)
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "create login response body failed");
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	ngx_memcpy(b->pos, backJson.data(), backJson.size());
	b->last = b->pos + backJson.size();  // ָ������ĩβ
	b->last_buf = 1;                    // �����������һ�黺����

	ngx_chain_t out;
	out.buf = b;
	out.next = NULL;
	rc = ngx_http_output_filter(r, &out);   // ���û�������Ӧ��  

	//�������ngx_http_finalize_request
	ngx_http_finalize_request(r, rc);
}

static ngx_int_t wx_login_set_redis_subrequest_done(ngx_http_request_t *r, void *data, ngx_int_t rc)
{
	printf("redis done\n");
	/*��ǰ������������*/
	ngx_http_request_t          *pr = r->parent;
	pr->headers_out.status = r->headers_out.status;
	/*���ʷ������ɹ�����ʼ��������*/
	if (NGX_HTTP_OK == r->headers_out.status)
	{
		ngx_str_t bak = { (size_t)(r->upstream->buffer.last - r->upstream->buffer.pos), r->upstream->buffer.pos };
		ngx_http_login_ctx_t* ptr = (ngx_http_login_ctx_t*)data;
		ptr->response[REDIS_RESPONSE].data = bak.data;
		ptr->response[REDIS_RESPONSE].len = bak.len;
		static std::string redis_ok_str = "+OK\n+OK\n";
		static std::string redis_ok_str2 = "+OK\r\n+OK\r\n";
		std::string redis_back((const char*)bak.data, bak.len);
		if ((redis_back.compare(redis_ok_str) != 0) && (redis_back.compare(redis_ok_str2) != 0)) 
		{
			ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "login set redis failed, response=%s", redis_back.c_str());
			pr->headers_out.status = NGX_HTTP_INTERNAL_SERVER_ERROR;
		}
		/*WXSessionInfo info;
		std::string backStr((const char*)bak.data, bak.len);
		if (!decode<WXSessionInfo>(backStr, info))
		{
			ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "get data from wx success, but parse json failed, json value=%s\n", backStr.c_str());
		}
		else*/
		{
			//char sessionID[SESSION_ID_LEN]{0};
			//if (0 != get_random_id(sessionID, SESSION_ID_LEN))
			//{
			//	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, errno, "generate session id failed\n");
			//	goto LOGIN_FAILED;
			//}

			//char sessionIDBase64[SESSION_ID_BASE64_LEN + 1]{0};
			//if (base64_encode(sessionID, SESSION_ID_LEN, sessionIDBase64) < 0)
			//{
			//	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, errno, "base64 session id failed\n");
			//	goto LOGIN_FAILED;
			//}
		}
	}
	/*���ø�����Ļص�����*/
	pr->write_event_handler = login_done;
	//��������������
	return NGX_OK;
}

	/*�������ص�*/
static void wx_login_subrequest_post_father_handler(ngx_http_request_t * r)
{
	printf("wx done, start redis\n");
	/*���û�з���200��ֱ�ӰѴ����뷢���û�*/
	if (r->headers_out.status != NGX_HTTP_OK)
	{
		ngx_http_finalize_request(r, r->headers_out.status);
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
	ngx_http_cutomer_module_conf_t* cf = (ngx_http_cutomer_module_conf_t*)ngx_http_get_module_main_conf(r, ngx_http_back_end_module);
	ngx_http_login_ctx_t *ctx = (ngx_http_login_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_back_end_module);
	psr->handler = wx_login_set_redis_subrequest_done;
	psr->data = ctx;
	WXSessionInfo info;
	std::string backStr((const char*)ctx->response[WX_RESPONSE].data, ctx->response[WX_RESPONSE].len);
	if (!decode<WXSessionInfo>(backStr, info))
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "get data from wx success, but parse json failed, json value=%s\n", backStr.c_str());
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}
	/*����������*/
	/*URL����=ǰ׺+����*/
	static ngx_str_t sub_location = ngx_string("/redis_set");
	static ngx_str_t key_str = ngx_string("key=");
	static ngx_str_t value_str = ngx_string("&value=");
	static ngx_str_t pwd_str = ngx_string("&pwd=");
	ngx_str_t key = { info.openid.size(), (u_char*)info.openid.data() };
	ngx_str_t value = { info.session_key.size(), (u_char*)info.session_key.data() };
	ngx_str_t sub_args;
	sub_args.len = key_str.len + value_str.len + pwd_str.len + info.openid.size() +
		info.session_key.size() + cf->redis_info.pwd.len;
	sub_args.data = (u_char *)ngx_palloc(r->pool, sub_args.len);
	ngx_snprintf(sub_args.data, sub_args.len, "%V%V%V%V%V%V", &key_str, &key,
		&value_str, &value, &pwd_str, &(cf->redis_info.pwd));

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
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "start set redis ngx_http_subrequest failed, rc=%d", rc);
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}
}

/*���������ʱ�Ļص�����*/
static ngx_int_t wx_login_subrequest_post_handler(ngx_http_request_t *r, void *data, ngx_int_t rc)
{
	printf("wx done, status=%d\n", (int)r->headers_out.status);
	/*��ǰ������������*/
	ngx_http_request_t          *pr = r->parent;
	pr->headers_out.status = r->headers_out.status;
	/*���ʷ������ɹ�����ʼ��������*/
	if (NGX_HTTP_OK == r->headers_out.status)
	{
		ngx_str_t bak = { (size_t)(r->upstream->buffer.last - r->upstream->buffer.pos), r->upstream->buffer.pos };
		ngx_http_login_ctx_t* ptr = (ngx_http_login_ctx_t*)data;
		ptr->response[WX_RESPONSE].data = bak.data;
		ptr->response[WX_RESPONSE].len = bak.len;
#if 0
		WXSessionInfo info;
		std::string backStr((const char*)bak.data, bak.len);
		if (!decode<WXSessionInfo>(backStr, info))
		{
			ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "get data from wx success, but parse json failed, json value=%s\n", backStr.c_str());
		}
		else
		{
			//char sessionID[SESSION_ID_LEN]{0};
			//if (0 != get_random_id(sessionID, SESSION_ID_LEN))
			//{
			//	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, errno, "generate session id failed\n");
			//	goto LOGIN_FAILED;
			//}

			//char sessionIDBase64[SESSION_ID_BASE64_LEN + 1]{0};
			//if (base64_encode(sessionID, SESSION_ID_LEN, sessionIDBase64) < 0)
			//{
			//	ngx_log_error(NGX_LOG_DEBUG, r->connection->log, errno, "base64 session id failed\n");
			//	goto LOGIN_FAILED;
			//}
		}
#endif
	}
	/*���ø�����Ļص�����*/
	pr->write_event_handler = wx_login_subrequest_post_father_handler;
	//��������������
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
	std::string codeJson((const char*)body_content.data, body_len);
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

	ngx_http_login_ctx_t* ctx = (ngx_http_login_ctx_t*)ngx_http_get_module_ctx(r, ngx_http_back_end_module);
	if (ctx == NULL)
	{
		//�������ڴ�����������, �������ܱ�֤��һ�������������Щ��Դ�����ͷ�  
		ctx = (ngx_http_login_ctx_t*)ngx_palloc(r->pool, sizeof(ngx_http_login_ctx_t));
		if (ctx == NULL) 
		{
			ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "alloc login ctx failed");
			ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
			return;
		}
		//�������Ľṹ�������  
		ngx_http_set_ctx(r, ctx, ngx_http_back_end_module);
	}

	psr->handler = wx_login_subrequest_post_handler;
	psr->data = ctx;
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
	//Ĭ��������������ֱ�ӷ��ظ��ͻ��˵ģ�������뷵���������Ҫ����NGX_HTTP_SUBREQUEST_IN_MEMORY��־��ע�⣬��������Handlerģ�鶼֧�ִ˱�־������fastcgiģ�鲻֧�֣�proxyģ��֧��
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

