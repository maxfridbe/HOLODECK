/*
	Wiktor Kopec
	Last Modified 04/25/04
*/

#ifndef CONSOLE_H
#define CONSOLE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

//<summary>Allows a write-only console to be attached to the process</summary>
//<remarks>Console is useful for debugging using cout/printf statements 
//in processes that do not normally support them.  This class cannot be
//instantiated.</remarks>
class Console	
{
	public:									
		//<summary>Initialize the console and redirects stdout</summary>
		static void	Init();
		
		//<summary>Destroy the console and release all resoures</summary>
		static void Release();	
	private:				
		static HANDLE out;						
		static FILE old;		
		
		//<summary>Private constructor</summary>
		Console();				
};

#endif
