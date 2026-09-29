/*
	Wiktor Kopec
	Last Modified 04/25/04
*/

#ifndef DIRECTDELEGATE_H
#define DIRECTDELEGATE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "delegate.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

//<summary>Base class for Direct Input handlers.</summary>
//<remarks>This class cannot be used in itself to handle DirectX events, but rather serves
//as a base class for Mouse and Keyboard Delegate classes, and it performs initializing tasks
//common to all Direct Input events</remarks>
template<class T>
class DirectDelegate : public Delegate<T>
{
	public:					
				
		//<summary>Default constructor</summary>
		DirectDelegate() : Delegate<T>() {}

		//<summary>Creates a method pointer</summary>
		//<remarks>You should not instantiate DirectDelegate.  Instead, use the inherited
		//classes to provide DirectX functionality</remarks>
		//<param name='hInstance'>Handle to the application instance.  Typically obtained
		//from Controller or WinMain</param>
		//<param name='newType'>The type of the delegate</param>
		//<param name='classInstance'>The instance of the class being pointed to</param>
		//<param name='classMethod'>A pointer to the method to invoke</param>
		//<param name='newFilter'>The filter to use by the controller.  This parameter is not used</param>				
		DirectDelegate(HINSTANCE hInstance, DelegateType newType, T * classInstance, Delegate<T>::VoidMethodV classMethod, unsigned short newFilter = 0x0) : Delegate<T>(newType, classInstance, classMethod, newFilter)
		{
			setType(newType);

			referenceCount++;
			if (directInterface == NULL)
			{				
				DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID *)&directInterface, NULL);
			}
		}

		//<summary>Copy constructor</summary>
		//<param name='source'>The source DirectDelegate</param>
		DirectDelegate(const DirectDelegate<T> & source) : Delegate<T>(source.type, source.instance, source.method, source.filter)
		{
			referenceCount++;
		}

		//<summary>Class destructor</summary>
		virtual ~DirectDelegate()
		{
			referenceCount--;
			if ( (referenceCount == 0) && (directInterface) )
			{
				directInterface -> Release();
				directInterface = NULL;
			}
		}	

	protected:
		static IDirectInput8 * directInterface;
	private:		
		static int referenceCount;
};

template<class T>
IDirectInput8 * DirectDelegate<T>::directInterface = NULL;

template<class T>
int DirectDelegate<T>::referenceCount = 0;

#endif
