/*
	Wiktor Kopec
	Last Modified 04/25/04
*/

#ifndef DELEGATE_H
#define DELEGATE_H


//<summary>Provides basic functionality for member function pointers</summary>
//<remarks>The delegate class serves as a base for method function pointers.  It is
//designed to be used with the controller class.</remarks>
template<class T>
class Delegate
{
	public:					
		
		typedef void (T::*VoidMethodV)();		
		
		//<summary>Enumeration for all delegate types</summary>
		enum DelegateType
		{						
			//<summary>Delegate for handling standard keyboard input</summary>
			//<remarks>Corresponds to KeyDelegate.</remarks>
			Key,
			
			//<summary>Delegate for handling DirectX keyboard input</summary>
			//<remarks>Corresponds to KeyboardDelegate.</remarks>
			Keyboard,		
			
			//<summary>Delegate for handling DirectX mouse input</summary>
			//<remarks>Corresponds to MouseDelegate.</remarks>
			Mouse,			
			
			//<summary>An unfiltered delegate.  See remarks</summary>
			//<remarks>Use this type if you want the delegate to always be called
			//when the window procedure is entered.</remarks>
			Unspecified,	
			
			//<summary>Filtered delegate.  See remarks</summary>
			//<remarks>Use this type if you want the delegate to be called only when
			//a specific message has been triggered.</remarks>
			Specified,		
			//<summary>Invalid delegate</summary>
			Invalid,		
			//<summary>Delegate for handling P5 glove input</summary>
			//<remarks>Corresponds to GloveDelegate.</remarks>
			Glove,			
		};

		
		//<summary>Default constructor.  Creates a NULL, Invalid delegate</summary>
		Delegate() : type(Invalid), instance(NULL), method(NULL), filter(0x0) 
		{		
		}
		
		
		
		//<summary>Creates a method pointer</summary>
		//<remarks>The base delegate class can only point to methods that have no arguments
		//and no return value, as specified by the typedef VoidMethodV</remarks>
		//<param name='newType'>The type of the delegate</param>
		//<param name='classInstance'>The instance of the class being pointed to</param>
		//<param name='classMethod'>A pointer to the method to invoke</param>
		//<param name='newFilter'>The filter to use by the controller</param>				
		Delegate(DelegateType newType, T * classInstance, VoidMethodV classMethod, unsigned short newFilter = 0x0) : type(newType), instance(classInstance), method(classMethod), filter(newFilter) 
		{
		}
		
		//<summary>Class destructor</summary>
		virtual ~Delegate() {}
		
		//<summary>Gets the type of this delegate</summary>
		//<returns>Returns the DelegateType</returns>
		DelegateType getType() const
		{
			return type;
		}

		//<summary>Invokes the function currently pointed to by this delegate</summary>		
		//<returns>Used internally.  Always 0L.</returns>
		long Invoke()
		{
			(instance ->* method)();
			return 0L;
		}
		
		//<summary>Sets the delegate type</summary>
		//<param name='newType'>The new delegate type</param>
		void setType(DelegateType newType)
		{
			type = newType;
		}

		//<summary>Gets the filter for the message</summary>
		//<returns>The filter for the message (one of the WM_ costants defined in winuser.h)</returns>
		unsigned short getFilter() const
		{
			return filter;
		}
		
		//<summary>Sets the filter of the message</summary>
		//<remarks>This does not apply to Unspecified delegates</remarks>
		//<param name='newFilter'>The new filter on which the message will be called.</param>
		void setFilter(unsigned short newFilter)
		{
			filter = newFilter;
		}

		

	protected:								
		VoidMethodV method;	
		unsigned short filter;
		T * instance;
		DelegateType type;
};

#endif
