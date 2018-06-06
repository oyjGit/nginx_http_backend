#!/bin/sh
#如果加上openssl连接的时候出现找不到pthreaad函数，需要修改objs/Makefile,将phtread放到最后
#openssl 源码编译
#./auto/configure --add-module=../nginx_http_back_end --with-debug --with-http_stub_status_module --with-http_ssl_module --with-openssl=../openssl-1.1.1-pre6

#openssl链接系统中安装的openssl,ubuntu 需要apt-get install libssl-dev
./auto/configure --add-module=../nginx_http_back_end --with-debug --with-http_stub_status_module --with-http_ssl_module --add-dynamic-module=../redis2-nginx-module-master --add-dynamic-module=../ngx_devel_kit --add-dynamic-module=../set-misc-nginx-module-master
