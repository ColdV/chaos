#pragma once

#include "../stdafx.h"
#include "../template/Single.h"

class Package : public Single<Package>
{
public:
	Package() {}
	virtual ~Package() {}

	//装包(头两个自己表示包大小)
	//@param return 返回装包后的大小 
	//如果返回值大于pkgSize，说明传入的pkg空间不足
	//0表示装包失败
	uint32 Pack(const char* msg, uint32 msgSize, char* pkg, uint32 pkgSize);

	//解包
	//@param return 返回已解包的大小 
	//如果返回值大于pkgSize，说明传入的pkg空间不足
	//如果返回值小于msgSize，装包成功并且剩下的数据是下一个包
	//0表示解包失败
	uint32 Unpack(const char* msg, uint32 msgSize, char* pkg, uint32 pkgSize);
};

//#define Pack(msg, msgSize, pkg, pkgSize) \
//Package::Instance().Pack(msg, msgSize, pkg, pkgSize);
//
//#define Unpack(msg, msgSize, pkg, pkgSize) \
//Package::Instance().Unpack(msg, msgSize, pkg, pkgSize);