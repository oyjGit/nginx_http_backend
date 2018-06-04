#include "Login.h"

extern "C"{

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
};

