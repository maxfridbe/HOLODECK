/*
	Wiktor Kopec
	Last Modified 04/27/04
*/

#include "wfunctions.h"

void Split(String str, Vector<String> & v, char delimiter)
{		
	if (str.getLength() <= 0)
	{
		return;
	}
	
	int limit(8);
	int tokenCount(0);
	char * token = NULL;
	char * baseString = const_cast<char *>(str.getCString());
	char * dString = new char[2];
	dString[0] = delimiter;
	dString[1] = '\0';
	
	v = Vector<String>(limit);
	
	token = strtok(baseString, dString);
	
	while (token)
	{
		v[tokenCount++] = String(token);
		if (tokenCount >= limit)
		{
			v.Resize(limit *= 2);
		}
		token = strtok(NULL, dString);
	}

	v.Resize(tokenCount);
	delete [] dString;
}
