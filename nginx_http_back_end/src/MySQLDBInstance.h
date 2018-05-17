#include "mysql++/mysql++.h"
#include <string>
#include "MySQLQueryResult.h"

class CMySqlDBInstance 
{
public:
	CMySqlDBInstance();
	~CMySqlDBInstance();

	int connect(const std::string& host, int16_t port, const std::string& userName, const std::string& pwd, const std::string& dbName);
	void disconnect();

	int update(const std::string& sql);

	int query(const std::string& sql, CMySqlQueryResult& result);


private:
	mysqlpp::Connection*		mConn{ nullptr };
};
