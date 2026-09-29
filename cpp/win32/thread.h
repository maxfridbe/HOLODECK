#ifndef THREAD_H
#define THREAD_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

template<typename T>
class Thread
{
	public:
		typedef void (T::*ThreadFunc)(void *);
	public:
		Thread(ThreadFunc func, T * caller, void * args);
		void Run();

	private:
		Thread();
		static DWORD WINAPI ThreadProc(void * arg);
		ThreadFunc m_proc;
		void * m_args;
		HANDLE m_handle;
		DWORD m_id;
		T * m_caller;
};

template<typename T>
Thread<T>::Thread(ThreadFunc func, T * caller, void * args) : m_proc(func), m_args(args), m_caller(caller) 
{
	m_handle = CreateThread(NULL, 0, ThreadProc, this, CREATE_SUSPENDED, &m_id);
}

template<typename T>
void Thread<T>::Run()
{
	ResumeThread(m_handle);
}

template<typename T>
DWORD WINAPI Thread<T>::ThreadProc(void * arg)
{
	Thread<T> * thread = (Thread<T> *)arg;
	((thread -> m_caller) ->* (thread -> m_proc))(thread -> m_args);
	return 0L;
}

#endif
