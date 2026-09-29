/*
	Wiktor Kopec
	Last Modified 04/25/04
*/

#ifndef CONTROLLER_H
#define CONTROLLER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "keydelegate.h"
#include "mousedelegate.h"
#include "keyboarddelegate.h"

#include "wlist.h"

//<summary>Allows the user to register handlers to handle specific input events.
//Supports Windows based message loop events, as well as Direct Input events.</summary>
//<remarks>Currently the controller only supports handling one window.  The template argument 
//must be a class, whose methods will be used to handle events.  This class cannot be
//instantiated.</remarks>
template<class T>
class Controller
{
	public:
/*		Controller<T>() : hwnd(NULL) 
		{
			memset(&wc, 0, sizeof(wc));
		}

		Controller<T>(HINSTANCE hInstance, const char * name)
		{
			memset(&wc, 0, sizeof(wc));
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.hInstance = hInstance;
			wc.lpszClassName = name;
			wc.lpfnWndProc = WinProc;
			wc.style = CS_OWNDC;

			RegisterClassEx(&wc);
			hwnd = CreateWindow(wc.lpszClassName, "", WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, wc.hInstance, NULL);
		}
		
		Controller<T>(const Controller<T> & source) : hwnd(source.hwnd) {}
*/
		
/*		
		~Controller<T>()
*/
				
		//<summary>Initializes the controller</summary>
		//<remarks>The name must be a unique Window class name. Typically the default value, but future implementations may allow for
		//multiple controllers, which may require unqiue names.</remarks>
		//<param name='hInstance'>Handle to the application instance.  This can be obtained
		//from WinMain</param>
		//<param name='name'>Unique name identifying this controller.  See remarks</param>
		//<returns>Returns true if function succeeds. Otherwise returns false</returns>
		static bool Init(HINSTANCE hInstance, const char * name = "Controller")
		{
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.hInstance = hInstance;
			wc.lpszClassName = name;
			wc.lpfnWndProc = WinProc;
			wc.style = CS_OWNDC;

			bool succeeded(true);
			
			if (RegisterClassEx(&wc))
			{
				hwnd = CreateWindow(wc.lpszClassName, "", WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, wc.hInstance, NULL);
				if (hwnd == NULL)
				{					
					succeeded = false;
				}
			}
			else
			{
				succeeded = false;
			}					

			return succeeded;
		}

		//<summary>Releases all resources held by the controller class</summary>
		static void Release()
		{
			for (List<Delegate<T> *>::Iterator it = handlerList.Begin(); !it.isNull(); it++)
			{
				delete (*it);
			}
			UnregisterClass(wc.lpszClassName, wc.hInstance);
			DestroyWindow(hwnd);
			hwnd = NULL;
		}

		//<summary>Returns the instance to the application</summary>
		//<returns>The instance handle initially passed in on initialization</returns>
		static HINSTANCE getInstance() 
		{
			return wc.hInstance;
		}

		//<summary>Returns the window handle</summary>
		//<remarks>It is the caller's responsibility not to modify this handle.  Changing it
		//will result in unexpected behavior by the controller.</remarks>
		//<returns>A window handle</returns>
		static const HWND getHandle()
		{			
			return hwnd;
		}

		//<summary>Determines if a key is being held down on the keyboard</summary>
		//<remarks>The key status is determined by WM_KEYDOWN/WM_KEYUP message pairs.
		//the key code is one of the VK_ codes declared in winuser.h</remarks>
		//<param name='code'>The virtual key code</param>
		//<returns>Returns true if key is being pressed.  Otherwise returns false</returns>
		static bool isDown(unsigned char code)
		{
			return keys[static_cast<int>(code)];
		}

		//<summary>Posts the controller quit message to the message loop</summary>
		//<remarks>This function replaces ::PostQuitMessage, and does not post WM_QUIT, which often
		//does not get processed appropriately on XP systems.</remarks>
		//<param name='code'>The exit code</param>
		static void PostQuitMessage(int code)
		{
			PostMessage(hwnd, QUIT, static_cast<int>(code), 0);
		}

		//<summary>Registers a handler with the controller</summary>
		//<param name='delegate'>A delegate to a function to register with the controller</param>
		static void AddHandler(const Delegate<T> & delegate)
		{			
			switch (delegate.getType())
			{
/*				case Delegate<T>::Glove:	
					handlerList.push_back(new GloveDelegate<T>(*(static_cast<GloveDelegate<T> *>(const_cast<Delegate<T> *>(&delegate)))));
					break;
*/
				case Delegate<T>::Key:
					handlerList.PushBack(new KeyDelegate<T>(*(static_cast<KeyDelegate<T> *>(const_cast<Delegate<T> *>(&delegate)))));
					break;
				case Delegate<T>::Keyboard:
					handlerList.PushBack(new KeyboardDelegate<T>(*(static_cast<KeyboardDelegate<T> *>(const_cast<Delegate<T> *>(&delegate)))));
					break;
				case Delegate<T>::Mouse:
					handlerList.PushBack(new MouseDelegate<T>(*(static_cast<MouseDelegate<T> *>(const_cast<Delegate<T> *>(&delegate)))));
					break;
				case Delegate<T>::Unspecified:
				case Delegate<T>::Specified:
					handlerList.PushBack(new Delegate<T>(delegate));
					break;
				default:
					break;
			}
		}
		
		//<summary>Unregisters a handler with the controller</summary>
		//<remarks>Not implemented</remarks>
		//<param name='delegate'>A delegate to a function to unregister with the controller</param>
		static void RemoveHandler(const Delegate<T> & delegate)
		{				
		}

        //<summary>Checks for messages inside the message loop</summary>
		//<remarks>This function supersedes ::PeekMessage.  It is designed to handle
		//DirectInput messages as well as regular messages</remarks>
		//<param name='msg'>A pointer to a message structure that is to receive the message</param>
		//<returns>Returns true if there are messages on the message queue.  Otherwise returns false</returns>
		static bool PeekMessage(LPMSG msg)
		{
			switch (msg -> message)
			{
				case WM_KEYDOWN:
					keys[static_cast<int>(msg -> wParam)] = true;
					break;
				case WM_KEYUP:
					keys[static_cast<int>(msg -> wParam)] = false;
					break;
			}
			for (List<Delegate<T> *>::Iterator it = handlerList.Begin(); !it.isNull(); it++)
			{
				switch((*it) -> getType())
				{
					case Delegate<T>::Keyboard:
						unsigned char keys[256];
						static_cast<KeyboardDelegate<T> *>(*it) -> Invoke(keys);
						break;
					case Delegate<T>::Mouse:
						static_cast<MouseDelegate<T> *>(*it) -> Invoke();
						break;
/*					case Delegate<T>::Glove:
						static_cast<GloveDelegate<T> *>(*it) -> Invoke();
						break;
*/
					default:
						break;
				}
			}

			return (::PeekMessage(msg, hwnd, 0, 0, PM_REMOVE) ? true : false);
			//::PeekMessage(msg, hwnd, 0, 0, PM_REMOVE | PM_QS_INPUT | PM_QS_POSTMESSAGE | PM_QS_SENDMESSAGE);
			//return true;
		}
		
		//<summary>Public constant that represents the quit message.</summary>
		static const QUIT = 0x0404;

	private:
		static HWND hwnd;
		static WNDCLASSEX wc;

		static bool keys[256];
		

		static List<Delegate<T> *> handlerList;
				
		static LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			LRESULT result(0L);
						
			for (List<Delegate<T> *>::Iterator it = handlerList.Begin(); !it.isNull(); it++)
			{
				switch ((*it) -> getType())
				{
					case Delegate<T>::Key:
						if (message == ((*it) -> getFilter()))
						{
							return static_cast<KeyDelegate<T> *>(*it) -> Invoke(static_cast<unsigned char>(wParam));
						}
						break;
					case Delegate<T>::Specified:
						if (message == ((*it) -> getFilter()))
						{
							return (*it) -> Invoke();
						}
						break;
					case Delegate<T>::Unspecified:
						return (*it) -> Invoke();						
					default:						
						break;
				}
			}
			result = DefWindowProc(hwnd, message, wParam, lParam);
			return result;
		}
};

template<class T>
List<Delegate<T> *> Controller<T>::handlerList;

template<class T>
bool Controller<T>::keys[256];

template<class T>
HWND Controller<T>::hwnd = NULL;

template<class T>
WNDCLASSEX Controller<T>::wc;

#endif
