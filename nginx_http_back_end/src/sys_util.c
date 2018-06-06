#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

int get_random_id(char* buf, int max) 
{
	if (NULL == buf || 0 > max) 
	{
		return -1;
	}
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0)
	{
		return -2;
	}
	else
	{
		int result = read(fd, buf, max);
		if (result < 0)
		{
			return -3;
		}
		close(fd);
		return 0;
	}
}

int base64_encode(char* src, int len, char* dst)
{
	BIO *b64, *bio;
	BUF_MEM *bptr = NULL;
	size_t size = 0;
	if (NULL == src || NULL == dst)
		return -1;
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_new(BIO_s_mem());
	bio = BIO_push(b64, bio);

	BIO_write(bio, src, len);
	BIO_flush(bio);

	BIO_get_mem_ptr(bio, &bptr);
	memcpy(dst, bptr->data, bptr->length);
	dst[bptr->length - 1] = '\0';
	size = bptr->length;

	BIO_free_all(bio);
	return size;
}

int base64_decode(char* src, int len, char* dst)
{
	BIO *b64, *bio;
	int size = 0;

	if (src == NULL || dst == NULL)
		return -1;

	b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

	bio = BIO_new_mem_buf(src, len);
	bio = BIO_push(b64, bio);

	size = BIO_read(bio, dst, len);
	dst[size] = '\0';

	BIO_free_all(bio);
	return size;
}

#ifdef __cplusplus
};
#endif // __cplusplus