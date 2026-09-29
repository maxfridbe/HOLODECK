#include "client.h"
#include "wstring.h"

void Client::Init()
{
	Socket::Init();
}

void Client::SyncSend(ClientState * state)
{
	while (state -> processed < state -> total)
	{
		int sent = m_socket.Send(state -> buffer, state -> total - state -> processed, state -> processed);
		
		if (sent > 0)
		{
			state -> processed += sent;
		}
		else
		{
			//TODO handle erroneous send
		}
	}	
}

void Client::SyncRecv(ClientState * state)
{
	while (state -> processed < state -> total)
	{
		int received = m_socket.Receive(state -> buffer, state -> total - state -> processed, state -> processed);
		
		if (received > 0)
		{
			state -> processed += received;
		}
		else
		{
			//TODO handle erroneous recv
		}
	}	
}

void Client::ReceiveHandler(void * args)
{
	//TODO implement a sync mechanism to prevent these sort of loops
	//TODO current impl. requires new buffers to be created on every transfer...this is highly inefficient
	while (true)
	{
		if ( (m_recvQueue.getSize() > 0) && (m_ready) )
		{
			ClientState * state = NULL;
			m_recvQueue.PopFront(state);

			SyncRecv(state);
			//TODO handle error conditions
			m_readyQueue.PushBack(state);
		}
	}
}

void Client::SendHandler(void * args)
{
	while (true)
	{
		if ( (m_sendQueue.getSize() > 0) && (m_ready) )
		{
			ClientState * state = NULL;
			m_sendQueue.PopFront(state);

			SyncSend(state);
			//TODO handle error conditions
		}
	}
}

void Client::EnqueueRecvData(ClientState * entry)
{
	this -> m_recvQueue.PushBack(entry);
}

void Client::EnqueueSendData(ClientState * entry)
{
	this -> m_sendQueue.PushBack(entry);
}

Client::ClientState * Client::DequeueData()
{
	ClientState * clientState;
	m_readyQueue.PopFront(clientState);
	return clientState;
}