#include "Login.h"
#include "LoginParam.h"
#include "LoginBackParam.h"
#include "WXSessionInfo.h"
#include "ngx_http_back_end_module.h"
#include "ngx_string.h"
#include "sys_util.h"

/*上下文*/
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
	/*如果没有返回200则直接把错误码发回用户*/
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

	// 设置响应的HTTP头部
	r->headers_out.status = NGX_HTTP_OK;           // 返回的响应码
	r->headers_out.content_length_n = backJson.size();    // 响应包体长度
	ngx_str_set(&r->headers_out.content_type, "text/json");
	// 如果响应不包含包体，则在此处可以直接返回rc  
	ngx_int_t rc = ngx_http_send_header(r); // 发送HTTP头部
	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "login send header failed, rc=%d", rc);
		ngx_http_finalize_request(r, rc);
		return;
	}

	// 分配响应包体空间，因为是异步发送，所以不要从栈中获得空间
	ngx_buf_t *b = ngx_create_temp_buf(r->pool, backJson.size());
	if (b == NULL)
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "create login response body failed");
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	ngx_memcpy(b->pos, backJson.data(), backJson.size());
	b->last = b->pos + backJson.size();  // 指向数据末尾
	b->last_buf = 1;                    // 声明这是最后一块缓冲区

	ngx_chain_t out;
	out.buf = b;
	out.next = NULL;
	rc = ngx_http_output_filter(r, &out);   // 向用户发送响应包  

	//必须调用ngx_http_finalize_request
	ngx_http_finalize_request(r, rc);
}

static ngx_int_t wx_login_set_redis_subrequest_done(ngx_http_request_t *r, void *data, ngx_int_t rc)
{
	printf("redis done\n");
	/*当前请求是子请求*/
	ngx_http_request_t          *pr = r->parent;
	pr->headers_out.status = r->headers_out.status;
	/*访问服务器成功，开始解析包体*/
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
	/*设置父请求的回调方法*/
	pr->write_event_handler = login_done;
	//结束本次子请求
	return NGX_OK;
}

	/*激活父请求回调*/
static void wx_login_subrequest_post_father_handler(ngx_http_request_t * r)
{
	printf("wx done, start redis\n");
	/*如果没有返回200则直接把错误码发回用户*/
	if (r->headers_out.status != NGX_HTTP_OK)
	{
		ngx_http_finalize_request(r, r->headers_out.status);
		return;
	}

	/*子请求的回调方法将在此结构体中设置*/
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
	/*构造子请求*/
	/*URL构造=前缀+参数*/
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

	/*sr即为子请求*/
	ngx_http_request_t *sr;
	/*创建子请求*/
	/*参数分别是：*/
	//1.父请求 2.请求URI 3.子请求的URI参数 4.返回创建好的子请求
	//5.子请求结束的回调
	//6.subrequest_in_memory 标识
	ngx_int_t rc = ngx_http_subrequest(r, &sub_location, &sub_args, &sr, psr, NGX_HTTP_SUBREQUEST_IN_MEMORY);
	if (rc != NGX_OK)
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "start set redis ngx_http_subrequest failed, rc=%d", rc);
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}
}

/*子请求结束时的回调函数*/
static ngx_int_t wx_login_subrequest_post_handler(ngx_http_request_t *r, void *data, ngx_int_t rc)
{
	printf("wx done, status=%d\n", (int)r->headers_out.status);
	/*当前请求是子请求*/
	ngx_http_request_t          *pr = r->parent;
	pr->headers_out.status = r->headers_out.status;
	/*访问服务器成功，开始解析包体*/
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
	/*设置父请求的回调方法*/
	pr->write_event_handler = wx_login_subrequest_post_father_handler;
	//结束本次子请求
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

	/*子请求的回调方法将在此结构体中设置*/
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
		//必须在内存池中申请对象, 这样才能保证在一个请求结束和这些资源都被释放  
		ctx = (ngx_http_login_ctx_t*)ngx_palloc(r->pool, sizeof(ngx_http_login_ctx_t));
		if (ctx == NULL) 
		{
			ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "alloc login ctx failed");
			ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
			return;
		}
		//把上下文结构体存起来  
		ngx_http_set_ctx(r, ctx, ngx_http_back_end_module);
	}

	psr->handler = wx_login_subrequest_post_handler;
	psr->data = ctx;
	/*构造子请求*/
	/*URL构造=前缀+参数*/
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

	/*sr即为子请求*/
	ngx_http_request_t *sr;
	/*创建子请求*/
	/*参数分别是：*/
	//1.父请求 2.请求URI 3.子请求的URI参数 4.返回创建好的子请求
	//5.子请求结束的回调
	//6.subrequest_in_memory 标识
	//默认子请求的输出是直接返回给客户端的，如果不想返回输出，需要设置NGX_HTTP_SUBREQUEST_IN_MEMORY标志。注意，不是所有Handler模块都支持此标志，比如fastcgi模块不支持，proxy模块支持
	ngx_int_t rc = ngx_http_subrequest(r, &sub_location, &sub_args, &sr, psr, NGX_HTTP_SUBREQUEST_IN_MEMORY);

	if (rc != NGX_OK)
	{
		ngx_log_error(NGX_LOG_ERR, r->connection->log, errno, "alloc ngx_http_subrequest failed");
		ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
		return;
	}

	/*父请求不会被销毁，而是等待再次被激活*/
	return;
}

//ngx_int_t ngx_http_customer_manager_wx_login(ngx_http_request_t *r)
//{
//	printf("start to subrequest\n");
//	return NGX_DONE;
//}

ngx_int_t ngx_http_customer_manager_login(ngx_http_request_t* r)
{
	// 请求的方法必须为POST
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
	//ngx_str_t response = ngx_string("{\"data\":\"hello world\"}");    // 包体内容
	//ngx_str_t type = ngx_string("text/json");
	//ngx_str_t response = { strlen(res), (u_char*)res };

	//													// 设置响应的HTTP头部
	//r->headers_out.status = NGX_HTTP_OK;           // 返回的响应码
	//r->headers_out.content_length_n = response.len;    // 响应包体长度
	//r->headers_out.content_type = type;                // Content-Type  

	//rc = ngx_http_send_header(r); // 发送HTTP头部
	//if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
	//	return rc;

	//// 如果响应不包含包体，则在此处可以直接返回rc  

	//// 分配响应包体空间，因为是异步发送，所以不要从栈中获得空间
	//ngx_buf_t *b = ngx_create_temp_buf(r->pool, response.len);

	//if (b == NULL)
	//	return NGX_HTTP_INTERNAL_SERVER_ERROR;

	//ngx_memcpy(b->pos, response.data, response.len);
	//b->last = b->pos + response.len;  // 指向数据末尾
	//b->last_buf = 1;                    // 声明这是最后一块缓冲区

	//ngx_chain_t out;
	//out.buf = b;
	//out.next = NULL;

	//return ngx_http_output_filter(r, &out);   // 向用户发送响应包  
}

#ifdef __cplusplus
};
#endif

