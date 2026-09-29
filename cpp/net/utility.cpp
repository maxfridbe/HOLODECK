#include "utility.h"
#include "header.h"
#include <memory.h>

int Utility::Serialize(const Header & header, char * buffer)
{
	((short *)buffer)[0] = header.request;
	((short *)buffer)[1] = header.specific;
	((int *)buffer)[1] = header.size;
	
	return sizeof(Header);
}

int Utility::Deserialize(Header & header, const char * buffer)
{
	header.request = (Header::RequestType)((short *)buffer)[0];
	header.specific = (Header::RequestSpecificType)((short *)buffer)[1];
	header.size = ((int *)buffer)[1];
	return sizeof(Header);
}

int Utility::Serialize(const float * array, int count, char * buffer)
{
	memcpy(buffer, array, sizeof(float) * count);
	return sizeof(float) * count;
}

Utility::Deserialize(float * array, int count, const char * buffer)
{
	memcpy(array, buffer, sizeof(float) * count);
	return sizeof(float) * count;
}