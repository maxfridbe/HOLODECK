#include "socket.h"

int Socket::Connect(const char * ip, unsigned short port)
{
	sockaddr_in addr;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	return connect(m_socket, (const sockaddr *)&addr, sizeof(sockaddr_in));
}

inline int Socket::Bind(const char * ip, unsigned short port)
{
	sockaddr_in addr;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	memset(addr.sin_zero, 0, sizeof(addr.sin_zero));

	return bind(m_socket, (const sockaddr *)&addr, sizeof(sockaddr_in));
}
