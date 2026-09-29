/*
	Wiktor Kopec
	Last Modified 04/27/04
*/

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "wstring.h"
#include "wlist.h"
#include "wvector.h"

struct IntKey
{
	int operator ()(const int key)
	{
		return key;
	}
};

struct StringKey
{
	int operator ()(const String & key)
	{
		int index(0);
		for (int i = 0; i < key.getLength(); i++)
		{
			index += key[i];
		}
		return index;
	}
};

template<class T, class K, class F = IntKey>
class Hashtable
{
	public:
		struct Pair
		{
			T value;
			K key;
		};
	public:				
		Hashtable() : elemCount(0) {}
		
		Hashtable(int initSize) : elemCount(0)
		{
			data.Resize(initSize);
		}

		T & operator [] (const K & key) const
		{
			F keygen;

			int index = keygen(key);
			if (index < 0)
			{
				index *= -1;
			}
			index %= data.getSize();
			
			List<Pair>::Iterator it = data[index].Begin();
            
			for(; !it.isNull(); it++)
			{
				if ((*it).key == key)
				{					
					return (*it).value;
				}
			}
			
			const_cast<Hashtable<T, K, F> *>(this) -> elemCount++;
			
			Pair p;
			p.key = key;
			memset(&p.value, 0, sizeof(p.value));
			
			data[index].PushFront(p);
			it = data[index].Begin();
			return (*it).value;
		}

		float getLoadFactor() const
		{
			return static_cast<float>(elemCount) / static_cast<float>(data.getSize());
		}
				
		typename List<typename Pair>::Iterator Begin(int bucket)
		{
			return data[bucket].Begin();
		}

		int getCount() const
		{
			return elemCount;
		}

		int getBucketCount() const
		{
			return data.getSize();
		}

		int getChainLength(int bucket)
		{
			return data[bucket].getSize();
		}		
		
		void Rehash(int newSize)
		{
			if (newSize <= data.getSize())
			{
				throw HashtableException("Cannot rehash to smaller size.");
			}

			Vector<List<Pair> > tempData(newSize);
            
			for (int i = 0; i < data.getSize(); i++)
			{
				for (List<Pair>::Iterator it = data[i].Begin(); !it.isNull(); it++)
				{
					F keygen;

					int index = keygen((*it).key);
					
					if (index < 0)
					{
						index *= -1;
					}

					index %= tempData.getSize();
												
					tempData[index].PushFront(*it);
				}
			}

			data = tempData;
		}
        
	private:
		Vector<List<Pair> > data;
		int elemCount;
};

/*-----------------------------------------------------------*/

class HashtableException
{
	public:
		HashtableException(const String &);
		const String & getMessage();
		void setMessage(const String &);
	private:
		String message;
};

#endif 
