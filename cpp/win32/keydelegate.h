/*
	Wiktor Kopec
	Last Modified 04/26/04
*/

#ifndef KEYDELEGATE_H
#define KEYDELEGATE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "delegate.h"

//<summary>Provides a method pointer that can be registered to handle keyboard input
//messages from the controller.</summary>
//<remarks>By default, the keydelegate only invokes the method it points to if the 
//key pressed generates a character.  To handle all key codes, pass WM_KEYDOWN as
//the filter.  For real-time user input, use the DirectInput version KeyboardDelegate</remarks>
template<class T>
class KeyDelegate : public Delegate<T>
{
	public:
		
		typedef void (T::*VoidMethodUC)(unsigned char);

		//<summary>Default constructor</summary>
		KeyDelegate() : instance(NULL), method(NULL), Delegate<T>() {}
		
		//<summary>Creates a pointer to the method</summary>
		//<param name='classInstance'>The instance of the class being pointed to</param>
		//<param name='classMethod'>A pointer to the method to invoke</param>
		//<param name='filter'>The filter to use by the controller.</param>				
		KeyDelegate(T * classInstance, VoidMethodUC classMethod, unsigned short filter = WM_CHAR) : Delegate<T>(Key, classInstance, reinterpret_cast<VoidMethodV>(classMethod), filter) {}
		
		//<summary>Invokes the function currently pointed to by this delegate</summary>
		//<param name='key'>The key code or character passed into the function</param>
		//<returns>Used internally.  Always 0L.</returns>
		long Invoke(unsigned char key)
		{			
			(instance ->* reinterpret_cast<VoidMethodUC>(method))(key);
			return 0L;
		}
};

#endif
