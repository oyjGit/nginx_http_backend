#ifndef __MYSQL_HELPER__
#define __MYSQL_HELPER__

//ubuntu ��Ҫ��װlibmysqlclient-dev ���� libmysql++ - dev

int connect_mysql_server(const char* host, int16_t port, char* user_name, const char* pwd, const char* db);

int disconnect_mysql_server();

int exec_sql(const char* sql);

#endif
