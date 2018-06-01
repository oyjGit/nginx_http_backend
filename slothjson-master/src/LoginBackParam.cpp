#include "LoginBackParam.h"
#include <iostream>

using std::cout;

CLoginBackParam::CLoginBackParam()
{
}

CLoginBackParam::~CLoginBackParam()
{
}

void CLoginBackParam::print()
{
	cout << "code:" << mCode << ", desc:" << mDesc << ", mData:" << mData;
}
