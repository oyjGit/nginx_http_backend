#include "Login.h"

extern "C"{

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
};

