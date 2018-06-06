#ifndef __NGX_HTTP_BACK_END_MODULE_H__
#define __NGX_HTTP_BACK_END_MODULE_H__

#include "ngx_module.h"
#include "ngx_http.h"
#include "customer_backend_typedef.h"

#define SESSION_ID_LEN 24
#define SESSION_ID_BASE64_LEN 32

extern ngx_module_t ngx_http_back_end_module;

#endif