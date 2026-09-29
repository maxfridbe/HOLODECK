#ifndef SOCKET_H
#define SOCKET_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#endif

class Socket
{
	public: 
		enum SocketType
		{
			Stream = SOCK_STREAM,
		};

		enum AddressFamily
		{
			InterNetwork = AF_INET,
		};

		enum ProtocolType
		{
			Tcp = IPPROTO_TCP,
		};
	public:
		Socket();
		Socket(AddressFamily addressFamily, SocketType socketType, ProtocolType protocolType);
		int Connect(const char * ip, unsigned short port);
		int Send(char * buffer, int amount, int offset = 0);
		int Receive(char * buffer, int amount, int offset = 0);
		int Close();

		int Listen(int backlog);
		int Bind(const char * ip, unsigned short port);
		Socket Accept();

		static int Init();
		static int Release();
	private:
		Socket(SOCKET descriptor);
		SOCKET m_socket;
};

inline Socket::Socket() : m_socket(~0U) {}

inline Socket::Socket(Socket::AddressFamily addressFamily, Socket::SocketType socketType, Socket::ProtocolType protocolType)
{
	m_socket = socket(addressFamily, socketType, protocolType);
}

inline Socket::Socket(SOCKET descriptor) : m_socket(descriptor) {}

inline int Socket::Send(char * buffer, int amount, int offset)
{
	return send(m_socket, &buffer[offset], amount, 0);
}

inline int Socket::Receive(char * buffer, int amount, int offset)
{
	return recv(m_socket, &buffer[offset], amount, 0);
}

inline int Socket::Close()
{
	return closesocket(m_socket);
}

inline int Socket::Init()
{
	WSADATA wsadata;
	return WSAStartup(MAKEWORD(1, 1), &wsadata);
}

inline int Socket::Release()
{
	return WSACleanup();
}

inline int Socket::Listen(int backlog)
{
	return listen(m_socket, backlog);
}

inline Socket Socket::Accept()
{
	return Socket(accept(m_socket, NULL, NULL));
}


#endif
