/*
	Wiktor Kopec
	Last Modified 04/27/04
*/

#include "whashtable.h"
#include "wstring.h"

HashtableException::HashtableException(const String & message)
{
	this -> message = message;
}

/*-----------------------------------------------------------*/

const String & HashtableException::getMessage()
{
	return this -> message;
}

/*-----------------------------------------------------------*/

void HashtableException::setMessage(const String & message)
{
	this -> message = message;
}
