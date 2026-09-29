/*
	Wiktor Kopec
	Last Modified 04/27/04
*/

#include "wstring.h"

String::String() : str(0), size(0) {}

/*-----------------------------------------------------------*/

String::String(const String & source) : str(0), size(0)
{
	if (source.str)
	{
		this -> str = new char[source.size + 1];
		strcpy(this -> str, source.str);
		this -> size = source.size;
	}
}

/*-----------------------------------------------------------*/

String::String(int initSize, char initValue) : str(0), size(initSize)
{
	if (initSize > 0)
	{
		this -> str = new char[initSize + 1];
		memset(this -> str, initValue, initSize);
		(this -> str)[initSize] = 0;
	}
	else
	{
		this -> size = 0;
	}
}

/*-----------------------------------------------------------*/

String::String(const char * initValue) : str(0), size(0)
{
	if (initValue)		
	{
		if ((this -> size = static_cast<int>(strlen(initValue))) > 0)
		{			
			this -> str = new char[this -> size + 1];
			strcpy(this -> str, initValue);
		}
	}
}

/*-----------------------------------------------------------*/

String::~String()
{
	if (this -> str)
	{
		delete [] this -> str;
	}
}

/*-----------------------------------------------------------*/

const String & String::operator = (const String & rhs)
{
	if (this == &rhs)
	{
		return (*this);
	}

	if (this -> str)
	{
		delete [] this -> str;
		this -> str = 0;
		this -> size = 0;
	}
	
	if (rhs.str)
	{
		this -> str = new char[rhs.size + 1];		
		strcpy(this -> str, rhs.str);
		this -> size = rhs.size;		
	}
	
	return (*this);
}

/*-----------------------------------------------------------*/

char & String::operator [](int index) const
{
	if (index >= this -> size)
	{
		throw StringException("Invalid index access.");
	}
	else
	{
		return (this -> str)[index];
	}
}

/*-----------------------------------------------------------*/

String String::operator + (const String & rhs) const
{
	String returnValue;	
	
	if (this -> str || rhs.str)
	{
		returnValue.size = this -> size + rhs.size;
		returnValue.str = new char[returnValue.size + 1];
		memset(returnValue.str, 0, returnValue.size + 1);
		strncat(returnValue.str, this -> str, this -> size);
		strncat(returnValue.str, rhs.str, rhs.size);
	}	

	return returnValue;
}

/*-----------------------------------------------------------*/

bool String::operator == (const String & rhs) const
{
	if ((!(this -> str)) && (!(rhs.str)))
	{
		return true;
	}

    if ((!(this -> str)) || (!(rhs.str)))
	{
		return false;
	}
	
	if (strcmp(this -> str, rhs.str) == 0)
	{
		return true;
	}

	return false;
}

/*-----------------------------------------------------------*/

bool String::operator != (const String & rhs) const
{
	if ((!(this -> str)) && (!(rhs.str)))
	{
		return false;
	}

    if ((!(this -> str)) || (!(rhs.str)))
	{
		return true;
	}
	
	if (strcmp(this -> str, rhs.str) == 0)
	{
		return false;
	}

	return true;
}

/*-----------------------------------------------------------*/

const String & String::operator += (const String & rhs)
{
	if (rhs.str)
	{
		char * newStr = new char[this -> size + rhs.size + 1];
		memset(newStr, 0, size + rhs.size + 1);
		strncat(newStr, this -> str, this -> size);
		strncat(newStr, rhs.str, rhs.size);
		this -> size += rhs.size;
		delete [] str;
		str = newStr;
	}
	return (*this);
}

/*-----------------------------------------------------------*/

const String & String::operator += (char c)
{
	char * newStr = new char[this -> size + 2];
	memset(newStr, 0, size + 2);
	strncat(newStr, this -> str, this -> size);
	newStr[size] = c;
	size++;
	delete [] str;
	str = newStr;
	return (*this);
}

/*-----------------------------------------------------------*/

int String::FindChar(char c) const
{
	for (int i = 0; i < size; i++)
	{
		if (str[i] == c)
		{
			return i;
		}
	}
	return -1;
}

/*-----------------------------------------------------------*/

String String::Substring(int first, int last) const
{
	if ( (first < 0) || (last >= size) || (first > last) )
	{
		throw StringException("Invalid parameters.");
	}

	String returnValue(last - first + 1, '\0');
	for (int i = first; i <= last; i++)
	{
		returnValue[i - first] = (*this)[i];
	}

	return returnValue;
}

/*-----------------------------------------------------------*/

void String::TrimEnd(int amount)
{
	if ( (amount > this -> size) || (amount <= 0) )
	{
		throw StringException("Invalid paramter.");
	}

	if (amount != this -> size)
	{
		char * newStr = new char[size - amount + 1];	
		strncpy(newStr, str, size - amount);
		newStr[size - amount] = '\0';
		delete [] str;
		str = newStr;
		size -= amount;
	}
	else
	{
		delete [] str;
		str = 0;
		size = 0;	
	}
}

/*-----------------------------------------------------------*/

void String::TrimFront(int amount)
{
	if ( (amount > this -> size) || (amount <= 0) )
	{
		throw StringException("Invalid paramter.");
	}

	if (amount != this -> size)
	{
		char * newStr = new char[size - amount + 1];	
		strncpy(newStr, &str[amount], size - amount);
		newStr[size - amount] = '\0';
		delete [] str;
		str = newStr;
		size -= amount;
	}
	else
	{
		delete [] str;
		str = 0;
		size = 0;	
	}
}

/*-----------------------------------------------------------*/

String String::ToString(int value)
{
	char * buffer = new char[16];
	sprintf(buffer, "%d", value);
	String returnValue(buffer);
	delete [] buffer;
	return returnValue;
}

/*-----------------------------------------------------------*/

String String::ToString(float value)
{
	char * buffer = new char[16];
	sprintf(buffer, "%.3f", value);
	String returnValue(buffer);
	delete [] buffer;
	return returnValue;
}

/*-----------------------------------------------------------*/

String String::ToString(double value)
{
	char * buffer = new char[16];
	sprintf(buffer, "%.3f", value);
	String returnValue(buffer);
	delete [] buffer;
	return returnValue;
}

/*-----------------------------------------------------------*/

int String::ToInt() const
{
	return atoi(str);
}

/*-----------------------------------------------------------*/

float String::ToFloat() const
{
	return static_cast<float>(atof(str));
}

/*-----------------------------------------------------------*/

double String::ToDouble() const
{
	return atof(str);
}

/*-----------------------------------------------------------*/

int String::getLength() const
{
	return this -> size;
}

/*-----------------------------------------------------------*/

const char * String::getCString() const
{
	return this -> str;
}

/*-----------------------------------------------------------*/

StringException::StringException(const String & message)
{
	this -> message = message;
}

const String & StringException::getMessage()
{
	return this -> message;
}

void StringException::setMessage(const String & message)
{
	this -> message = message;
}
