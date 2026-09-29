/*
	Wiktor Kopec
	Last Modified 04/27/04
*/

#ifndef STRING_H
#define STRING_H

#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include <cmath>

//<summary>Provides a managed string type</summary>
//<remarks>This class is similar to the stl string, but more specialized.  It is also
//not a template class.</remarks>
class String
{
	public:
		//<summary>Default constructor</summary>
		String();
		
		//<summary>Constructs a new string</summary>
		//<param name='newString'>The new string value</param>
		String(const char * newString);
		
		//<summary>Constructs a new string</summary>
		//<remarks>Passing in a zero (\0) for the initial character value
		//can lead to memory leaks</remarks>
		//<param name='length'>The length of the string</param>
		//<param name='value'>The initial character for every index in the string</param>
		String(int length, char value);

		//<summary>Copy constructor</summary>
		//<param name='source'>The source string</param>
		String(const String & source);
		
		//<summary>Class destructor</summary>
		~String();

		//<summary>Assignment operator</summary>
		//<param name='rhs'>The string to copy from</param>
		//<returns>The newly assigned string reference</returns>
		const String & operator = (const String & rhs);

		//<summary>Indexer</summary>
		//<param name='index'>The index in the string</param>
		//<returns>The character at the index</returns>
		char & operator [] (int index) const;

		//<summary>Concatonation operator</summary>
		//<param name='rhs'>The string to concatonate</param>
		//<returns>A new string, which consists of lhs concatonated with the rhs</returns>
		String operator + (const String & rhs) const;
		
		//<summary>Checks if two strings are equal</summary>
		//<remarks>The empty string "" is equivalent to a string created by a 
		//default constructor.</remarks>
		//<param name='rhs'>The string to compare to</param>
		//<returns>True if the strings are equal, otherwise false</returns>
		bool operator == (const String & rhs) const;

		//<summary>Checks if two strings are equal</summary>
		//<param name='rhs'>The string to compare to</param>
		//<returns>True if the strings are not equal, otherwise false</returns>
		bool operator != (const String & rhs) const;
		
		//<summary>Concatonation with assignment</summary>
		//<param name='rhs'>The string to concatonate</param>
		//<returns>A new string, which consists of lhs concatonated with the rhs</returns>		
		const String & operator += (const String & rhs);

		//<summary>Concatonates a character to a string</summary>
		//<param name='rhs'>The character to concatonate</param>
		//<returns>The left hand side concatonated with the character</returns>
		const String & operator += (char rhs);
		
		//<summary>Trims the end of the string</summary>
		//<remarks>Note this trims by amount, not by whitespace or other characters</remarks>
		//<param name='amount'>Amount to trim by</param>
		void TrimEnd(int amount);

		//<summary>Trims the front of the string</summary>
		//<remarks>Note this trims by amount, not by whitespace or other characters</remarks>
		//<param name='amount'>Amount to trim by</param>
		void TrimFront(int amount);

		//<summary>Gets a substring from the current string</summary>
		//<remarks>Note this takes the substring using indexes not lengths</remarks>
		//<param name='firstIndex'>The index from which to start the substring</param>
		//<param name='lastIndex'>The index at which to end the substring.  This is inclusive</param>
		//<returns>The new substring formed</returns>
		String Substring(int firstIndex, int lastIndex) const;

		//<summary>Finds a character in a string</summary>
		//<param name='c'>The character to find</summary>
		//<returns>Returns the index at which the character resides, or -1 if the character is not found</returns>
		int FindChar(char c) const;		

		//<summary>Returns the length of the string</summary>
		//<returns>The length of the string</returns>
		int getLength() const;

		//<summary>Returns a pointer to the underlying C-type string</summary>
		//<remarks>The user should not const_cast and modify this string</remarks>
		//<returns>A const pointer to the underlying string</returns>
		const char * getCString() const;
				
		//<summary>Converts the string to an integer</summary>
		//<returns>Returns the parsed integer from the string</returns>
		int ToInt() const;

		//<summary>Converts the string to a float</summary>
		//<returns>Returns the parsed float from the string</returns>
		float ToFloat() const;

		//<summary>Converts the string to a double</summary>
		//<returns>Returns the parsed double from the string</returns>
		double ToDouble() const;
		
		//<summary>Converts an integer to a string representation</summary>
		//<param name='value'>The value to convert</param>
		//<returns>The string representation of the value converted</returns>
		static String ToString(int value);

		//<summary>Converts a float to a string representation</summary>
		//<param name='value'>The value to convert</param>
		//<returns>The string representation of the value converted</returns>
		static String ToString(float value);

		//<summary>Converts a double to a string representation</summary>
		//<param name='value'>The value to convert</param>
		//<returns>The string representation of the value converted</returns>
		static String ToString(double value);
        		
	private:
		char * str;
		int size;
};

/*-----------------------------------------------------------*/

class StringException
{
	public:
		StringException(const String &);
		
		const String & getMessage();
		void setMessage(const String &);
	private:
		String message;
};	

#endif
