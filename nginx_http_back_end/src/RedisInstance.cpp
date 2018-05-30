#include "RedisInstance.h"
#include <time.h>

CRedisInstance::CRedisInstance()
{

}

CRedisInstance::~CRedisInstance()
{
	disconnect();
}

int CRedisInstance::connect(const std::string& host, int16_t port, const std::string& userPwd)
{
	if (host.empty() || 0 == port || userPwd.empty()) 
	{
		return -1;
	}
	struct timeval tm = { 3, 500000 };
	mConnect = redisConnectWithTimeout(host.c_str(), port, tm);
	if (nullptr == mConnect) 
	{
		return -2;
	}
	if (mConnect->err) 
	{
		return -3;
	}
	mReply = (redisReply *)redisCommand(mConnect, "auth %s", userPwd.c_str());
	if (mReply->type == REDIS_REPLY_ERROR)
	{
		disconnect();
		return -4;
	}
	freeReplyObject(mReply);
	mReply = nullptr;
	return 0;
}

void CRedisInstance::disconnect()
{
	if (nullptr != mConnect) 
	{
		redisFree(mConnect);
		mConnect = nullptr;
	}
	if (nullptr != mReply) 
	{
		freeReplyObject(mReply);
		mReply = nullptr;
	}
}

int CRedisInstance::setString(const std::string & key, const std::string & value)
{
	if (nullptr == mConnect) 
	{
		return -1;
	}
	if (key.empty())
	{
		return -2;
	}
	//TODO;use nonblock
	if (redisCommand(mConnect, "SET %s %s", key.c_str(), value.c_str()) == NULL) 
	{
		return -3;
	}
	return 0;
}

std::string CRedisInstance::getString(const std::string & key)
{
	mReply = (redisReply*)redisCommand(mConnect, "GET %s", key.c_str());
	std::string str = mReply->str;
	freeReplyObject(mReply);
	return str;
}

