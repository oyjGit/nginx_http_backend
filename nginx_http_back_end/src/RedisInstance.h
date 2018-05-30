#ifndef __REDIS_INSTANCE_H__
#define __REDIS_INSTANCE_H__

#include <string>
#include "hiredis/hiredis.h"

//ubuntu 安装 hiredis git地址:git clone https://github.com/redis/hiredis.git
//下载后直接make -j 4;sudo make install;

class CRedisInstance 
{
public:
	CRedisInstance();
	~CRedisInstance();

	int connect(const std::string& host, int16_t port, const std::string& userPwd);
	void disconnect();

	int setString(const std::string& key, const std::string& value);

	std::string getString(const std::string& key);

private:
	std::string		mHost;
	int16_t			mPort{ 6379 };
	std::string		mUserPwd;

	redisContext*	mConnect{ nullptr };
	redisReply*		mReply{ nullptr };
};

#endif