#ifndef __CUSTOMER_BACKEND_H__
#define __CUSTOMER_BACKEND_H__

typedef struct mysql_connect_conf_s
{
	uint8_t host[128];
	uint8_t user_name[128];
	uint8_t user_pwd[128];
	uint8_t db_name[128];
	int16_t port;
}mysql_connect_conf_t;

#endif // !__CUSTOMER_BACKEND_H__
