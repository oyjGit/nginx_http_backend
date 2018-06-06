#ifndef __SYS_UTIL_H__
#define __SYS_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int get_random_id(char* buf, int max);

int base64_encode(char* src, int len, char* dst);

int base64_decode(char* src, int len, char* dst);

#ifdef __cplusplus
};
#endif // __cplusplus

#endif
