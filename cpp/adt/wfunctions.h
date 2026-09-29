/*
	Wiktor Kopec
	Last Modified 04/27/04
*/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "wstring.h"
#include "wvector.h"

//<summary>Splits a string into multiple components (similar to strtok)</summary>
//<param name='s'>The string to split</param>
//<param name='v'>The vector to receive the string components. The function
//will automatically allocate it</param>
//<param name='c'>The character delimiter</param>
void Split(String s, Vector<String> & v, char c = ' ');

#endif
