#ifndef __MYSQL_HELPER__
#define __MYSQL_HELPER__

//ubuntu 需要安装libmysqlclient-dev 或者 libmysql++ - dev

#ifdef __cplusplus
extern "C"{
#endif

int connect_mysql_server(const char* host, int16_t port, const char* user_name, const char* pwd, const char* db);

int disconnect_mysql_server();

int exec_sql(const char* sql);

void* connect(const char* host, int16_t port, const char* user_name, const char* pwd, const char* db);

void disconnect(void* handle);

#ifdef __cplusplus
};
#endif

#endif
