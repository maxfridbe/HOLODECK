/*
	Wiktor Kopec
	Last Modified 04/25/04
*/

#include "display.h"

#pragma comment(lib, "opengl32.lib")

void Display::SwapBuffers()
{
	::SwapBuffers(wglGetCurrentDC());
}

void Display::Release()
{	
	wglDeleteContext(wglGetCurrentContext());	
	wglMakeCurrent(NULL, NULL);
}

bool Display::Init(HWND handle, int px, int py, int x, int y, int colorBits, int depthBits, int accumBits, int alphaBits, int stencilBits, int auxBits)
{
	SetWindowPos(handle, HWND_TOP, px, py, x, y, SWP_SHOWWINDOW);

	HDC hdc = GetDC(handle);
	
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	
	int iFormat(0);

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;

	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |	PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = static_cast<BYTE>(colorBits);
	pfd.cDepthBits = static_cast<BYTE>(depthBits);
	pfd.cAccumBits = static_cast<BYTE>(accumBits);
	pfd.cAlphaBits = static_cast<BYTE>(alphaBits);
	pfd.cAuxBuffers = static_cast<BYTE>(auxBits);
	pfd.cStencilBits = static_cast<BYTE>(stencilBits);
	pfd.dwLayerMask = PFD_MAIN_PLANE;
	
	iFormat = ChoosePixelFormat(hdc, &pfd);
	
	bool succeeded(true);

	if (iFormat)
	{
		if (SetPixelFormat(hdc, iFormat, &pfd))
		{
			HGLRC glrc = wglCreateContext(hdc);
			if (glrc)
			{
				if (!wglMakeCurrent(hdc, glrc))
				{					
					succeeded = false;
				}	
			}
			else
			{
				succeeded = false;
			}
		}
		else
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
