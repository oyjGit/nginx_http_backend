#include "MySQLDBInstance.h"
#include <stdio.h>

CMySqlDBInstance::CMySqlDBInstance()
{
}

CMySqlDBInstance::~CMySqlDBInstance()
{
	disconnect();
}

int CMySqlDBInstance::connect(const std::string& host, int16_t port, const std::string& userName, const std::string& pwd, const std::string& dbName)
{
	if (nullptr != mConn) 
	{
		return -1;
	}
	if (host.empty() || userName.empty() || pwd.empty() || dbName.empty()) 
	{
		return -2;
	}
	mConn = new mysqlpp::Connection(false);
	// 首先设置字符集   如果你不设置的话到时候存进去的utf8格式会乱码  
	if (!mConn->set_option(new mysqlpp::SetCharsetNameOption("utf8")))
	{
		printf("set db conn charset failed\n");
	}
	// 连接数据库  
	if (mConn->connect(dbName, host, userName, pwd) == false) 
	{
		printf("set db conn connect failed\n");
		return -3;
	}
	return 0;
}

void CMySqlDBInstance::disconnect()
{
	if (nullptr != mConn) 
	{
		mConn->disconnect();
		delete mConn;
		mConn = nullptr;
	}
}

int CMySqlDBInstance::update(const std::string& sql)
{
	if (nullptr == mConn) 
	{
		return -1;
	}
	if (sql.empty()) 
	{
		return -2;
	}
	if (!mConn->query().exec(sql)) 
	{
		return -3;
	}
	return 0;
}

int CMySqlDBInstance::query(const std::string& sql, CMySqlQueryResult& result)
{
	if (nullptr == mConn)
	{
		return -1;
	}
	if (sql.empty())
	{
		return -2;
	}
	mysqlpp::Query query = mConn->query();
	query() << sql;
	result.fillData(query.store());
	return 0;
}
