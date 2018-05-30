#ifndef __REDIS_UTIL_H__
#define __REDIS_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

void* redis_connect(const char* host, int16_t port, const char* pwd);

void redis_disconnect(void* instance);

int redis_set_string(void* redis, const char* key, const char* value);

int redis_get_string(void* redis, const char* key, char* value, int max_len);

#ifdef __cplusplus
};
#endif

#endif