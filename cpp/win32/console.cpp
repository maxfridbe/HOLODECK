/*
	Wiktor Kopec
	Last Modified 04/25/04
*/

#include "console.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "wstring.h"

HANDLE Console::out = NULL;
FILE Console::old = *stdout;

void Console::Init()
{	
	if (!out)
	{
		AllocConsole();
		out = GetStdHandle(STD_OUTPUT_HANDLE);
		*stdout = *(_fdopen(_open_osfhandle((intptr_t)out, _O_TEXT), "w"));
	}
}

void Console::Release()
{
	if (out)
	{
		FreeConsole();
		CloseHandle(out);
		out = NULL;
		*stdout = old;
	}
}
