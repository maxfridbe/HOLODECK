/*
	Wiktor Kopec
	Last Modified 04/26/04
*/

#ifndef MOUSEDELEGATE_H
#define MOUSEDELEGATE_H

#include "directdelegate.h"

//<summary>This class can be registered with the controller to handle mouse input
//using Direct X</summary>
//<remarks>Use this handler if you need to determine the relative motion on the mouse
//axes.  Ideal for looking around.</remarks>
template<class T>
class MouseDelegate : public DirectDelegate<T>
{
	public:
		
		typedef void (T::*VoidMethodIIIIIUCA)(int, int, int, int, int, unsigned char *);

		//<summary>Creates a delegate that can be registered with the controller</summary>
		//<remarks>The first three parameters represent the relative motion on the x y and z axes.
		//The Z axis is the mouse wheel.  The next two represent the absolute x and y position, in window
		//screen coordinates (where (0,0) represents the upper left).  The final parameter is an array of 4
		//elements that represent which mouse buttons are being held down.</remarks>
		//<param name='inst'>Application instance handle.  This can be obtained from the controller
		//or WinMain</param>
		//<param name='handle'>Handle to the window.  This must be obtained from the controller to which
		//the function is going to be registered to</param>
		//<param name='classInstance'>The instance of the class being pointed to</param>
		//<param name='classMethod'>A pointer to the method to invoke.  The method must
		//have no return values, five integers and one unsigned char * parameter</param>		
		MouseDelegate(HINSTANCE inst, HWND handle, T * classInstance, VoidMethodIIIIIUCA classMethod) : DirectDelegate<T>(inst, Mouse, classInstance, reinterpret_cast<VoidMethodV>(classMethod))
		{
			refCount++;
			if (mouse == NULL)
			{
				directInterface -> CreateDevice(GUID_SysMouse, &mouse, NULL);
				mouse -> SetDataFormat(&c_dfDIMouse);
				mouse -> SetCooperativeLevel(handle, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
				mouse -> Acquire();
			}
		}
		
		//<summary>Copy constructor</summary>
		//<param name='source'>Source delegate</param>
		MouseDelegate(const MouseDelegate<T> & source) : DirectDelegate<T>(source)
		{
			refCount++;
		}
		
		//<summary>Class destructor</summary>
		~MouseDelegate()
		{
			refCount--;
			if (refCount == 0)
			{
				mouse -> Release();
				mouse = NULL;
			}
		}

		//<summary>Invokes the method pointed to by this delegate</summary>
		void Invoke()
		{
			POINT point;
			DIMOUSESTATE mouseState;
			
			mouse -> GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);
			GetCursorPos(&point);

			(instance ->* reinterpret_cast<VoidMethodIIIIIUCA>(method))(mouseState.lX, mouseState.lY, mouseState.lZ, point.x, point.y, mouseState.rgbButtons);
		}
		
	private:
		static IDirectInputDevice8 * mouse;
		static int refCount;
};

template<class T>
IDirectInputDevice8 * MouseDelegate<T>::mouse = NULL;

template<class T>
int MouseDelegate<T>::refCount = 0;

#endif
