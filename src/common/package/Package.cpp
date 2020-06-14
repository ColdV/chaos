#include "Package.h"


uint32 Package::Pack(const char* msg, uint32 msgSize, char* pkg, uint32 pkgSize)
{
	if (!msg || !pkg)
		return 0;

	uint16* bodySize = (uint16*)pkg;
	*bodySize = msgSize;

	uint32 headSize = sizeof(uint16);

	if (pkgSize < msgSize + headSize)
		return msgSize + headSize;

	memcpy(pkg + sizeof(uint16), msg, msgSize);

	return msgSize + headSize;
}


uint32 Package::Unpack(const char* msg, uint32 msgSize, char* pkg, uint32 pkgSize)
{
	if (!msg || !pkg)
		return 0;

	uint16 bodySize = *(uint16*)msg;

	if (pkgSize < bodySize)
		return bodySize;

	memcpy(pkg, msg + sizeof(uint16), bodySize);

	return bodySize + sizeof(uint16);
}