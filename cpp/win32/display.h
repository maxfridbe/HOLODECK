/*
	Wiktor Kopec
	Last Modified 04/25/04
*/

#ifndef DISPLAY_H
#define DISPLAY_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//<summary>Configures the OpenGL display for a handle</summary>
class Display
{
	public:
		//<summary>Initialize the rendering context and window</summary>
		//<remarks>The handle should be part of a window class that has its device context.
		//Typically, this handle will be obtained from the Controller class.</remarks>
		//<param name='handle'>Handle to a window</param>
		//<param name='px'>X position of the window. A value of 0 represents the left edge</param>
		//<param name='py'>Y position of the window. A value of 0 represents the top edge</param>
		//<param name='x'>Width</param>
		//<param name='y'>Height</param>
		//<param name='colorBits'>Color bits</param>
		//<param name='depthBits'>Depth buffer bits</param>
		//<param name='accumBits'>Accumulation buffer bits</param>
		//<param name='alphaBits'>Alpha buffer bits</param>
		//<param name='stencilBits'>Stencil buffer bits</param>
		//<param name='auxBits'>Auxiliary buffer bits</param>
		//<returns>If the call succeeds, the function returns true.  Otherwise false</returns>
		static bool Init(HWND handle, int px = 0, int py = 0, int x = 640, int y = 480, int colorBits = 24, int depthBits = 16, int accumBits = 0, int alphaBits = 0, int stencilBits = 0, int auxBits = 0);
		
		//<summary>Release all resources held by the Display class</summary>
		static void Release();

		//<summary>Swap the front and back buffers</summary>
		static void SwapBuffers();		
};

#endif
