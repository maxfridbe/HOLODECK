#ifndef CLIENT_H
#define CLIENT_H

#include "socket.h"
#include "header.h"
#include "wlist.h"

class Client
{
	public:
		static const unsigned short Port = 1307;
	public:
		struct ClientState
		{
			char * buffer;
			Header currentHeader;
			int total;
			int processed;
		};
	public:
		Client();			
		int Connect(const char * ip, unsigned short port);		
		void SendHandler(void * args);
		void ReceiveHandler(void * args);
		void EnqueueSendData(ClientState * entry);
		void EnqueueRecvData(ClientState * entry);
		ClientState * DequeueData();
		static void Init();

		bool & Ready();

	private:
		void SyncRecv(ClientState * state);
		void SyncSend(ClientState * state);

		List<ClientState *> m_recvQueue;
		List<ClientState *> m_sendQueue;
		List<ClientState *> m_readyQueue;

		bool m_ready;
		Socket m_socket;

};


inline Client::Client() : m_ready(false), m_socket(Socket::InterNetwork, Socket::Stream, Socket::Tcp) {}

inline int Client::Connect(const char * ip, unsigned short port)
{
	return m_socket.Connect(ip, port);	
}

inline bool & Client::Ready()
{
	return m_ready;
}

#endif
