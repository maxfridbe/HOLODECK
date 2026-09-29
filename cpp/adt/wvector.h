/*
	Wiktor Kopec
	Last Modified 04/27/04
*/

#ifndef VECTOR_H
#define VECTOR_H

#include "wstring.h"

template<class T>
class Vector
{
	public:
		Vector() : data(0), size(0) {}
		
		/*-----------------------------------------------------------*/		

		Vector(int initSize) : data(0), size(initSize)
		{
			if (initSize > 0)
			{
				this -> data = new T[initSize];
			}
			else
			{
				this -> size = 0;
			}
		}
		
		/*-----------------------------------------------------------*/

		Vector(int initSize, const T & initValue) : data(0), size(initSize)
		{
			if (initSize > 0)
			{
				this -> data = new T[initSize];
				for (int i = 0; i < initSize; i++)
				{
					this -> data[i] = initValue;
				}
			}
			else
			{
				this -> size = 0;
			}		
		}

		/*-----------------------------------------------------------*/
		
		Vector(const Vector<T> & source) : data(0), size(0)
		{
			if (source.data)
			{
				this -> size = source.size;
				this -> data = new T[this -> size];

				for (int i = 0; i < this -> size; i++)
				{
					this -> data[i] = source.data[i];
				}
			}
		}
		
		/*-----------------------------------------------------------*/

		~Vector()
		{
			if (this -> data)
			{
				delete [] this -> data;
			}		
		}

		/*-----------------------------------------------------------*/
		
		const Vector<T> & operator = (const Vector<T> & rhs)
		{
			if (this == &rhs)
			{
				return (*this);
			}

			if (this -> data)
			{
				delete [] this -> data;
				this -> data = 0;
				this -> size = 0;
			}

			if (rhs.data)
			{
				this -> size = rhs.size;
				this -> data = new T[this -> size];

				for (int i = 0; i < this -> size; i++)
				{
					this -> data[i] = rhs.data[i];
				}
			}

			return (*this);
		}

		/*-----------------------------------------------------------*/

		T & operator [] (int index) const
		{		

#ifdef _DEBUG
			if (index > this -> size - 1)
			{
				throw VectorException("Invalid index.");
			}
#endif			
			return this -> data[index];
		}

		/*-----------------------------------------------------------*/

		const T * getData() const
		{
			return this -> data;
		}
		
		/*-----------------------------------------------------------*/
		
		void Resize(int newSize)
		{
			if (newSize < 0)
			{
				return;
			}

			if (newSize > 0)
			{
				T * temp = new T[newSize];
				
				int copySize = ((this -> size < newSize) ? this -> size : newSize);
				
				for (int i = 0; i < copySize; i++)
				{
					temp[i] = this -> data[i];
				}
				
				delete [] this -> data;
				this -> data = temp;
			}
			else
			{
				if (this -> data)
				{
					delete [] this -> data;
				}
				this -> data = 0;
			}

			this -> size = newSize;			
		}

		/*-----------------------------------------------------------*/
		
		int getSize() const
		{
			return this -> size;
		}

	private:
		T * data;
		int size;
};

/*-----------------------------------------------------------*/

class VectorException
{
	public:
		VectorException(const String &);
		const String & getMessage();
		void setMessage(const String &);
	private:
		String message;
};

#endif
