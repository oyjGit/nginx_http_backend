#!/bin/sh
#如果加上openssl连接的时候出现找不到pthreaad函数，需要修改objs/Makefile,将phtread放到最后
./auto/configure --add-module=../nginx_http_back_end --with-debug --with-http_stub_status_module --with-http_ssl_module --with-openssl=../openssl-1.1.1-pre6
