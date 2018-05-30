#include "RedisInstance.h"
#include <string.h>

#ifdef __cplusplus
extern "C"{
#endif

void* redis_connect(const char* host, int16_t port, const char* pwd)
{
	if (NULL == host || NULL == pwd || 0 == port) 
	{
		return NULL;
	}
	CRedisInstance* ins = new CRedisInstance;
	std::string hostStr(host);
	std::string passwd(pwd);
	if (0 != ins->connect(hostStr, port, passwd)) 
	{
		delete ins;
		return NULL;
	}
	return ins;
}

void redis_disconnect(void* instance)
{
	if (NULL != instance) 
	{
		CRedisInstance* ins = (CRedisInstance*)instance;
		delete ins;
	}
}

int redis_set_string(void* redis, const char* key, const char* value)
{
	CRedisInstance* ins = (CRedisInstance*)redis;
	std::string keyStr(key);
	std::string valueStr(value);
	return ins->setString(keyStr, valueStr);
}

int redis_get_string(void* redis, const char* key, char* value, int max_len)
{
	CRedisInstance* ins = (CRedisInstance*)redis;
	std::string keyStr(key);
	std::string valueStr = ins->getString(keyStr);
	if (!valueStr.empty()) 
	{
		int cpLen = max_len < (int)valueStr.size() ? max_len : valueStr.size();
		strncpy(value, valueStr.c_str(), cpLen);
	}
	return valueStr.size();
}

#ifdef __cplusplus
};
#endif