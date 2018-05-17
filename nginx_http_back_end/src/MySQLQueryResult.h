#include "mysql++/mysql++.h"

class CMySqlQueryResult 
{
public:
	CMySqlQueryResult();
	~CMySqlQueryResult();
	void fillData(const mysqlpp::StoreQueryResult& result);
	uint32_t rowNum();
	uint32_t columnNum();
	

private:
	uint32_t mCurRow{ 0 };
	mysqlpp::StoreQueryResult mResult;
};