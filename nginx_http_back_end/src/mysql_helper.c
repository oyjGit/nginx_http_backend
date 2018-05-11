#include <stdio.h>
#include <mysql/mysql.h>

static MYSQL* connection = NULL;

int connect_mysql_server(const char* host, int16_t port, char* user_name, const char* pwd, const char* db) 
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
	if (connection == NULL)
	{
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
	if (mysql_query(connection, sql))
	{
		return -1;
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
		}
		mysql_free_result(result);
		return i;
	}
	return 0;
}
