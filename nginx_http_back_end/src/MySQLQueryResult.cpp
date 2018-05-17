#include "MySQLQueryResult.h"

CMySqlQueryResult::CMySqlQueryResult()
{

}

CMySqlQueryResult::~CMySqlQueryResult()
{
}

void CMySqlQueryResult::fillData(const mysqlpp::StoreQueryResult& result)
{
	mResult = result;
	mCurRow = 0;
}

uint32_t CMySqlQueryResult::rowNum()
{
	return mResult.num_rows();
}

uint32_t CMySqlQueryResult::columnNum()
{
	return mResult.num_fields();
}

