#include <stdio.h>
#include <mysql/mysql.h>
#include "MySQLDBInstance.h"

static MYSQL* connection = NULL;


extern "C" {


int connect_mysql_server(const char* host, int16_t port, const char* user_name, const char* pwd, const char* db)
{
	if (NULL == host || NULL == user_name || NULL == pwd) 
	{
		return -1;
	}
	if (NULL != connection)
	{
		return -2;
	}
	connection = mysql_init(NULL);
	if (NULL == connection)
	{
		return -3;
	}
	connection = mysql_real_connect(connection, host, user_name, pwd, db, 0, NULL, 0);
	if (NULL == connection)
	{
		printf("connect host=%s, user name=%s, pwd=%s, db=%s failed, error=%d\n", host, user_name, pwd, db, mysql_errno(connection));
		return -4;
	}
	return 0;
}

int disconnect_mysql_server() 
{
	if (NULL == connection)
	{
		return -1;
	}
	else 
	{
		mysql_close(connection);
		connection = NULL;
		return 0;
	}
}

int exec_sql(const char* sql)
{
	if (NULL == connection) 
	{
		return -1;
	}
	if (mysql_query(connection, sql))
	{
		return -2;
	}
	else
	{
		MYSQL_RES *result = mysql_use_result(connection);
		unsigned int i = 0;
		for (; i < mysql_field_count(connection); ++i)
		{
			MYSQL_ROW row = mysql_fetch_row(result);
			if (row == NULL)
			{
				break;
			}
			else 
			{
				printf("%s\n", row[1]);
			}
		}
		mysql_free_result(result);
		return i;
	}
	return 0;
}

void* connect_mysql_db(const char* host, int16_t port, const char* user_name, const char* pwd, const char* db)
{
	CMySqlDBInstance* ins = new CMySqlDBInstance;
	std::string hostStr(host);
	std::string name(user_name);
	std::string passwd(pwd);
	std::string dbName(db);
	if (ins->connect(hostStr, port, name, passwd, dbName) != 0) 
	{
		delete ins;
		return nullptr;
	}
	return ins;
}

void disconnect_mysql_db(void* handle)
{
	if (nullptr != handle) 
	{
		CMySqlDBInstance* ins = (CMySqlDBInstance*)handle;
		delete ins;
	}
}

}