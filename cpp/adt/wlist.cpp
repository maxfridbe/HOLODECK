/*
	Wiktor Kopec
	Last Modified 04/27/04
*/

#include "wlist.h"

/*-----------------------------------------------------------*/

ListException::ListException(const String & message)
{
	this -> message = message;
}

/*-----------------------------------------------------------*/

const String & ListException::getMessage()
{
	return this -> message;
}

/*-----------------------------------------------------------*/

void ListException::setMessage(const String & message)
{
	this -> message = message;
}

/*-----------------------------------------------------------*/

IteratorException::IteratorException(const String & message)
{
	this -> message = message;
}

/*-----------------------------------------------------------*/

const String & IteratorException::getMessage()
{
	return this -> message;
}

/*-----------------------------------------------------------*/

void IteratorException::setMessage(const String & message)
{
	this -> message = message;
}
