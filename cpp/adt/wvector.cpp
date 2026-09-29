/*
	Wiktor Kopec
	Last Modified 04/27/04
*/

#include "wvector.h"

VectorException::VectorException(const String & message)
{
	this -> message = message;
}

const String & VectorException::getMessage()
{
	return this -> message;
}

void VectorException::setMessage(const String & message)
{
	this -> message = message;
}
