/*
	Wiktor Kopec
	Last Modified 04/26/04
*/

#ifndef KEYBOARDDELEGATE_H
#define KEYBOARDDELEGATE_H

#include "directdelegate.h"

//<summary>This class can be registered with the controller to handle keyboard input</summary>
//<remarks>Use this class if you need to obtain realtime keyboard input from the user.  Do not use it
//for typing purposes</remarks>
template<class T>
class KeyboardDelegate : public DirectDelegate<T>
{
	public:
		typedef void (T::*VMethodUCA)(unsigned char *);
		
		//<summary>Creates a delegate that can be registered with the controller</summary>
		//<remarks>To test if a key is down, determine if the high order bits are true on any 
		//particular key.  E.G. if (keys(DIK_UP) bitwise AND 0x80) is true, that means the up arrow key
		//is currently in the pressed state.  The DIK_ macros can be found in dinput.h</remarks>
		//<param name='inst'>Application instance handle.  This can be obtained from the controller
		//or WinMain</param>
		//<param name='handle'>Handle to the window.  This must be obtained from the controller to which
		//the function is going to be registered to</param>
		//<param name='classInstance'>The instance of the class being pointed to</param>
		//<param name='classMethod'>A pointer to the method to invoke.  The method must
		//have no return values, and have one unsigned char * parameter</param>
		KeyboardDelegate(HINSTANCE inst, HWND handle, T * classInstance, VMethodUCA classMethod) : DirectDelegate<T>(inst, Keyboard, classInstance, reinterpret_cast<VoidMethodV>(classMethod))
		{
			refCount++;
			if (keyboard == NULL)
			{
				directInterface -> CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
				keyboard -> SetDataFormat(&c_dfDIKeyboard);
				keyboard -> SetCooperativeLevel(handle, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
				keyboard -> Acquire();
			}
		}
		
		//<summary>Copy constructor</summary>
		//<param name='source'>Source delegate</param>
		KeyboardDelegate(const KeyboardDelegate<T> & source) : DirectDelegate<T>(source)
		{
			refCount++;
		}
		
		//<summary>Class destructor</summary>
		~KeyboardDelegate()
		{
			refCount--;
			if (refCount == 0)
			{
				keyboard -> Release();
				keyboard = NULL;
			}
		}
	
		//<summary>Invokes the method pointed to by this delegate</summary>
		//<remarks>To test if a key is down, determine if the high order bits are true on any 
		//particular key.  E.G. if (keys(DIK_UP) bitwise AND 0x80) is true, that means the up arrow key
		//is currently in the pressed state.  The DIK_ macros can be found in dinput.h</remarks>
		//<param name='keys'>An array of keys.  This array must have 256 elements, each representing
		//the state of a key</param>
		void Invoke(unsigned char * keys)
		{
			keyboard -> GetDeviceState(256, keys);
			(instance ->* reinterpret_cast<VMethodUCA>(method))(keys);
		}
		
	private:
		static IDirectInputDevice8 * keyboard;
		static int refCount;
};

template<class T>
IDirectInputDevice8 * KeyboardDelegate<T>::keyboard = NULL;

template<class T>
int KeyboardDelegate<T>::refCount = 0;

#endif
