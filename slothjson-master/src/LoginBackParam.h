#ifndef __LOGIN_BACK_PARAM_H__
#define __LOGIN_BACK_PARAM_H__

#include <string>

class CLoginBackParam 
{
public:
	CLoginBackParam();
	~CLoginBackParam();

	void print();

private:
	int				mCode;
	std::string		mDesc;
	std::string		mData;

};

#endif