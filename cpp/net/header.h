#ifndef HEADER_H
#define HEADER_H

struct Header
{	
	enum RequestType
	{
		None,
		Connect,
		Disconnect,
		Error,
		Position,
	};
		
	enum RequestSpecificType
	{										
		TooManyClients = 1,
	};	

	RequestType request : 2;
	RequestSpecificType specific : 2;
	int size;	
};

#endif
