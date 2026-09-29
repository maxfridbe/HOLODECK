/*
	Wiktor Kopec
	Last Modified 04/27/04
*/

#ifndef LIST_H
#define LIST_H

#include "wstring.h"

template<class T>
class ListNode;

template<class T>
class List
{
	public:		
		class Iterator
		{
			friend class List<T>;

			public:
				Iterator() : last(0) {}
				
				/*-----------------------------------------------------------*/

				T & operator * () const
				{
					return this -> last -> value;
				}
				
				/*-----------------------------------------------------------*/

				bool operator == (const Iterator rhs) const
				{
					if (this -> last == rhs.last)
					{
						return true;
					}
					return false;
				}

				/*-----------------------------------------------------------*/
				
				bool operator != (const Iterator rhs) const
				{
					if (this -> last != rhs.last)
					{
						return true;
					}
					return false;
				}
				
				
				/*-----------------------------------------------------------*/
				
				const Iterator operator + (int step) const
				{
					Iterator returnValue(this -> last);
					
					for (int i = 0; i < step; i++)
					{
						if (returnValue.last)
						{
							returnValue.last = returnValue.last -> next;
						}
						else
						{
							throw IteratorException("Null iterator.");
						}
					}

					return returnValue;
				}
				
				/*-----------------------------------------------------------*/

				const Iterator & operator += (int step)
				{
					for (int i = 0; i < step; i++)
					{
						if (this -> last)
						{
							this -> last = this -> last -> next;
						}
						else
						{
							throw IteratorException("Null iterator.");
						}
					}
					return (*this);
				}
				
				/*-----------------------------------------------------------*/

				const Iterator operator - (int step) const
				{
					Iterator returnValue(this -> last);
					
					for (int i = 0; i < step; i++)
					{
						if (returnValue.last)
						{
							returnValue.last = returnValue.last -> prev;
						}
						else
						{
							throw IteratorException("Null iterator.");
						}
					}

					return returnValue;
				}
				
				/*-----------------------------------------------------------*/

				const Iterator & operator -= (int step)
				{
					for (int i = 0; i < step; i++)
					{
						if (this -> last)
						{
							this -> last = this -> last -> prev;
						}
						else
						{
							throw IteratorException("Null iterator.");
						}
					}
					return (*this);
				}

				/*-----------------------------------------------------------*/

				const Iterator & operator ++()
				{
					if (this -> last)
					{
						this -> last = this -> last -> next;
						return (*this);
					}
					else
					{
						throw IteratorException("Null iterator.");
					}
				}

				/*-----------------------------------------------------------*/

				const Iterator & operator ++(int)
				{
					if (this -> last)
					{
						this -> last = this -> last -> next;
						return (*this);
					}
					else
					{
						throw IteratorException("Null iterator.");
					}
				}

				/*-----------------------------------------------------------*/

				const Iterator & operator --()
				{
					if (this -> last)
					{
						this -> last = this -> last -> prev;
						return (*this);
					}
					else
					{
						throw IteratorException("Null iterator.");
					}
				}
				
				/*-----------------------------------------------------------*/
				
				const Iterator & operator --(int)
				{
					if (this -> last)
					{
						this -> last = this -> last -> prev;
						return (*this);
					}
					else
					{
						throw IteratorException("Null iterator.");
					}
				}
				
				/*-----------------------------------------------------------*/

				bool isNull()
				{
					if (this -> last)
					{
						return false;
					}
					return true;					
				}

			private:
				Iterator(ListNode<T> * node) : last(node) {}
				
				/*-----------------------------------------------------------*/
                				
				ListNode<T> * last;												
		};

		/*-----------------------------------------------------------*/

		class ConstIterator
		{
			friend class List<T>;

			public:
				ConstIterator() : last(0) {}
				
				/*-----------------------------------------------------------*/

				const T & operator * () const
				{
					return this -> last -> value;
				}
				
				/*-----------------------------------------------------------*/

				bool operator == (const ConstIterator rhs) const
				{
					if (this -> last == rhs.last)
					{
						return true;
					}
					return false;
				}

				/*-----------------------------------------------------------*/
				
				bool operator != (const ConstIterator rhs) const
				{
					if (this -> last != rhs.last)
					{
						return true;
					}
					return false;
				}
				
				
				/*-----------------------------------------------------------*/
				
				const ConstIterator operator + (int step) const
				{
					ConstIterator returnValue(this -> last);
					
					for (int i = 0; i < step; i++)
					{
						if (returnValue.last)
						{
							returnValue.last = returnValue.last -> next;
						}
						else
						{
							throw ConstIteratorException("Null ConstIterator.");
						}
					}

					return returnValue;
				}
				
				/*-----------------------------------------------------------*/

				const ConstIterator & operator += (int step)
				{
					for (int i = 0; i < step; i++)
					{
						if (this -> last)
						{
							this -> last = this -> last -> next;
						}
						else
						{
							throw ConstIteratorException("Null ConstIterator.");
						}
					}
					return (*this);
				}
				
				/*-----------------------------------------------------------*/

				const ConstIterator operator - (int step) const
				{
					ConstIterator returnValue(this -> last);
					
					for (int i = 0; i < step; i++)
					{
						if (returnValue.last)
						{
							returnValue.last = returnValue.last -> prev;
						}
						else
						{
							throw ConstIteratorException("Null ConstIterator.");
						}
					}

					return returnValue;
				}
				
				/*-----------------------------------------------------------*/

				const ConstIterator & operator -= (int step)
				{
					for (int i = 0; i < step; i++)
					{
						if (this -> last)
						{
							this -> last = this -> last -> prev;
						}
						else
						{
							throw ConstIteratorException("Null ConstIterator.");
						}
					}
					return (*this);
				}

				/*-----------------------------------------------------------*/

				const ConstIterator & operator ++()
				{
					if (this -> last)
					{
						this -> last = this -> last -> next;
						return (*this);
					}
					else
					{
						throw ConstIteratorException("Null ConstIterator.");
					}
				}

				/*-----------------------------------------------------------*/

				const ConstIterator & operator ++(int)
				{
					if (this -> last)
					{
						this -> last = this -> last -> next;
						return (*this);
					}
					else
					{
						throw ConstIteratorException("Null ConstIterator.");
					}
				}

				/*-----------------------------------------------------------*/

				const ConstIterator & operator --()
				{
					if (this -> last)
					{
						this -> last = this -> last -> prev;
						return (*this);
					}
					else
					{
						throw ConstIteratorException("Null ConstIterator.");
					}
				}
				
				/*-----------------------------------------------------------*/
				
				const ConstIterator & operator --(int)
				{
					if (this -> last)
					{
						this -> last = this -> last -> prev;
						return (*this);
					}
					else
					{
						throw ConstIteratorException("Null ConstIterator.");
					}
				}
				
				/*-----------------------------------------------------------*/

				bool isNull()
				{
					if (this -> last)
					{
						return false;
					}
					return true;					
				}

			private:
				ConstIterator(ListNode<T> * node) : last(node) {}
				
				/*-----------------------------------------------------------*/
                				
				ListNode<T> * last;												
		};

		/*-----------------------------------------------------------*/

		List() : head(0), size(0), tail(0) {}
		
		/*-----------------------------------------------------------*/
		
		List(int initSize) : head(0), size(initSize), tail(0)
		{
			if (initSize > 0)
			{
				this -> head = new ListNode<T>();
				
				ListNode<T> * cur = this -> head;
				
				for (int i = 0; i < initSize - 1; i++)
				{
					cur -> next = new ListNode<T>();
					cur -> next -> prev = cur;
					cur = cur -> next;
				}
				
				this -> tail = cur;
			}
			else
			{
				this -> size = 0;
			}
		}

		/*-----------------------------------------------------------*/
		
		List(int initSize, const T & initValue) : head(0), size(initSize), tail(0)
		{
			if (initSize > 0)
			{
				this -> head = new ListNode<T>(initValue);
				
				ListNode<T> * cur = this -> head;
				
				for (int i = 0; i < initSize - 1; i++)
				{
					cur -> next = new ListNode<T>(initValue);
					cur -> next -> prev = cur;
					cur = cur -> next;
				}
				
				this -> tail = cur;
			}
			else
			{
				this -> size = 0;
			}
		}
		
		/*-----------------------------------------------------------*/

		List(const List<T> & source) : head(0), size(0), tail(0)
		{
			if (source.head)
			{
				this -> head = new ListNode<T>(source.head -> value);
				
				ListNode<T> * curSource = source.head -> next;
				ListNode<T> * curDest = this -> head;
				
				for (int i = 0; i < source.size - 1; i++)
				{
					curDest -> next = new ListNode<T>(curSource -> value);
					curDest -> next -> prev = curDest;
					curDest = curDest -> next;
					curSource = curSource -> next;
				}
				this -> tail = curDest;
				this -> size = source.size;
			}		
		}

		/*-----------------------------------------------------------*/

		~List()
		{
			if (this -> head)
			{
				ListNode<T> * cur(0);
				
				while (this -> head -> next)
				{
					cur = this -> head -> next;
					this -> head -> next = cur -> next;
					delete cur;
				}
				delete this -> head;
			}    		
		}

		/*-----------------------------------------------------------*/		
		
		const List<T> & operator = (const List<T> & rhs)
		{
			if (this == &rhs)
			{
				return (*this);
			}

			if (this -> head)
			{
				ListNode<T> * cur(0);
				
				while (this -> head -> next)
				{
					cur = this -> head -> next;
					this -> head -> next = cur -> next;
					delete cur;
				}
				delete this -> head;
				this -> head = 0;
				this -> tail = 0;
				this -> size = 0;
			}

			if (rhs.head)
			{
				this -> head = new ListNode<T>(rhs.head -> value);
				
				ListNode<T> * curSource = rhs.head -> next;
				ListNode<T> * curDest = this -> head;
				
				for (int i = 0; i < rhs.size - 1; i++)
				{
					curDest -> next = new ListNode<T>(curSource -> value);
					curDest -> next -> prev = curDest;
					curDest = curDest -> next;
					curSource = curSource -> next;
				}
				this -> tail = curDest;
				this -> size = rhs.size;
			}

			return (*this);		
		}
		
		/*-----------------------------------------------------------*/
		
		T & operator [](int index) const
		{
			if (index > this -> size - 1)
			{
				throw ListException("Invalid index.");
			}
			
			ListNode<T> * cur(0);

			if (index < this -> size / 2)
			{
				cur = this -> head;
				for (int i = 0; i < index; i++)
				{
					cur = cur -> next;
				}
			}
			else
			{
				cur = this -> tail;
				for (int i = size - 1; i > index; i--)
				{
					cur = cur -> prev;
				}
			}
			return cur -> value;
		}

        /*-----------------------------------------------------------*/
		
		void PushFront(const T & value)
		{
			(this -> size)++;
			
			if (this -> head)
			{
				ListNode<T> * newNode = new ListNode<T>(value);
				newNode -> next = this -> head;
				this -> head -> prev = newNode;
				this -> head = newNode;
			}
			else
			{
				this -> head = new ListNode<T>(value);
				this -> tail = this -> head;
			}		
		}
		
		/*-----------------------------------------------------------*/
		
		void PushBack(const T & value)
		{
			(this -> size)++;
			
			if (this -> head)
			{
				this -> tail -> next = new ListNode<T>(value);
				this -> tail -> next -> prev = this -> tail;
				this -> tail = this -> tail -> next;
			}
			else
			{
				this -> head = new ListNode<T>(value);
				this -> tail = this -> head;
			}			
		}
		
		/*-----------------------------------------------------------*/

		void PopFront()
		{
			if (this -> head)
			{
				if (--(this -> size) == 0)
				{
					delete this -> head;
					this -> head = 0;
					this -> tail = 0;
				}
				else
				{
					ListNode<T> * cur = this -> head;
					this -> head = this -> head -> next;
					this -> head -> prev = 0;
					delete cur;
				}
			}			
		}
		
		/*-----------------------------------------------------------*/

		void PopFront(T & value)
		{
			if (this -> head)
			{
				if (--(this -> size) == 0)
				{
					value = this -> head -> value;
					delete this -> head;
					this -> head = 0;
					this -> tail = 0;
				}
				else
				{
					ListNode<T> * cur = this -> head;
					this -> head = this -> head -> next;
					this -> head -> prev = 0;
					value = cur -> value;
					delete cur;
				}
			}
		}
		
		/*-----------------------------------------------------------*/
		
		void PopBack()
		{
			if (this -> head)
			{
				if (--(this -> size) == 0)
				{
					delete this -> head;
					this -> head = 0;
					this -> tail = 0;
				}
				else
				{
					this -> tail = this -> tail -> prev;
					delete this -> tail -> next;
					this -> tail -> next = 0;
				}
			}
		}
		
		/*-----------------------------------------------------------*/
		
		void PopBack(T & value)
		{
			if (this -> head)
			{
				if (--(this -> size) == 0)
				{
					value = this -> head -> value;
					delete this -> head;
					this -> head = 0;
					this -> tail = 0;
				}
				else
				{
					this -> tail = this -> tail -> prev;
					value = this -> tail -> next -> value;
					delete this -> tail -> next;
					this -> tail -> next = 0;
				}
			}
		}
		
		/*-----------------------------------------------------------*/
		
		void InsertBefore(int index, const T & value)
		{			

#ifdef _DEBUG
			if ( (index >= this -> size) || (index < 0) )
			{
				throw ListException("Bad index.");
			}
#endif			

			ListNode<T> * cur(0);

			if (index < this -> size / 2)
			{
				cur = this -> head;
				for (int i = 0; i < index; i++)
				{
					cur = cur -> next;
				}
			}
			else
			{
				cur = this -> tail;
				for (int i = size - 1; i > index; i--)
				{
					cur = cur -> prev;
				}
			}

			if (cur == head)
			{
				head = new ListNode<T>(value);
				head -> next = cur;
				cur -> prev = head;
			}
			else
			{
				cur -> prev -> next = new ListNode<T>(value);
				cur -> prev -> next -> next = cur;
				cur -> prev -> next -> prev = cur -> prev;
				cur -> prev = cur -> prev -> next;
			}

			size++;
		}
		
		/*-----------------------------------------------------------*/	

		void InsertAfter(int index, const T & value)
		{

#ifdef _DEBUG
			if ( (index >= this -> size) || (index < 0) )
			{
				throw ListException("Bad index.");
			}
#endif
			
			ListNode<T> * cur(0);

			if (index < this -> size / 2)
			{
				cur = this -> head;
				for (int i = 0; i < index; i++)
				{
					cur = cur -> next;
				}
			}
			else
			{
				cur = this -> tail;
				for (int i = size - 1; i > index; i--)
				{
					cur = cur -> prev;
				}
			}

			if (cur == tail)
			{
				cur -> next = new ListNode<T>(value);
				cur -> next -> prev = cur;
				tail = cur -> next;
			}
			else
			{
				//Make this block insert after index
				
				ListNode<T> * newNode = new ListNode<T>(value);
				newNode -> next = cur -> next;
				cur -> next -> prev = newNode;
				cur -> next = newNode;
				newNode -> prev = cur;				
			}

			size++;
		}

		/*-----------------------------------------------------------*/
		
		void InsertBefore(Iterator & it, const T & value)
		{
#ifdef _DEBUG
			if (it.isNull())
			{
				throw ListException("Bad iterator.");
			}
#endif

			if (it.last == head)
			{
				head = new ListNode<T>(value);
				head -> next = it.last;
				it.last -> prev = head;
			}
			else
			{
				it.last -> prev -> next = new ListNode<T>(value);
				it.last -> prev -> next -> next = it.last;
				it.last -> prev -> next -> prev = it.last -> prev;
				it.last -> prev = it.last -> prev -> next;			
			}

			size++;
		}


		/*-----------------------------------------------------------*/

		void InsertAfter(Iterator & it, const T & value)
		{
#ifdef _DEBUG
			if (it.isNull())
			{
				throw ListException("Bad iterator.");
			}
#endif			

			if (it.last == tail)
			{
				it.last -> next = new ListNode<T>(value);
				it.last -> next -> prev = it.last;
				tail = it.last -> next;
			}
			else
			{
				//Make this block insert after index
				
				ListNode<T> * newNode = new ListNode<T>(value);
				newNode -> next = it.last -> next;
				it.last -> next -> prev = newNode;
				it.last -> next = newNode;
				newNode -> prev = it.last;				
			}

			size++;

		}

		void Remove(int index)
		{
#ifdef _DEBUG
			if ( (index >= size) || (index < 0) )
			{
				throw ListException("Bad index.");
			}
#endif
			ListNode<T> * cur(0);

			if (index < this -> size / 2)
			{
				cur = this -> head;
				for (int i = 0; i < index; i++)
				{
					cur = cur -> next;
				}
			}
			else
			{
				cur = this -> tail;
				for (int i = size - 1; i > index; i--)
				{
					cur = cur -> prev;
				}
			}		
			
			if (cur == head)
			{
				PopFront();
			}
			else if (cur == tail)
			{
				PopBack();				
			}
			else
			{
				cur -> prev -> next = cur -> next;
				cur -> next -> prev = cur -> prev;
				delete cur;
				size--;
			}
		}

		void Remove(Iterator & it)
		{
#ifdef _DEBUG
			if (it.isNull())
			{
				throw ListException("Bad iterator.");
			}
#endif
			if (it.last == head)
			{
				it.last = it.last -> prev;
				PopFront();
				
			}
			else if (it.last == tail)
			{
				it.last = it.last -> prev;
				PopBack();
			}
			else
			{				
				ListNode<T> * temp = it.last -> prev;
				it.last -> prev -> next = it.last -> next;
				it.last -> next -> prev = it.last -> prev;
				delete it.last;
				it.last = temp;
				size--;
			}
		}

		/*-----------------------------------------------------------*/
		
		Iterator Begin()
		{
			return Iterator(this -> head);
		}
		
		/*-----------------------------------------------------------*/

		Iterator Last()
		{
			return Iterator(this -> tail);	
		}
		
		/*-----------------------------------------------------------*/

		Iterator End()
		{
			return ConstIterator(0x0);
		}		
		
		/*-----------------------------------------------------------*/

		ConstIterator Begin() const
		{
			return ConstIterator(this -> head);
		}
		
		/*-----------------------------------------------------------*/

		ConstIterator Last() const
		{
			return ConstIterator(this -> tail);	
		}
		
		/*-----------------------------------------------------------*/

		ConstIterator End() const
		{
			return ConstIterator(0x0);
		}		
		
		/*-----------------------------------------------------------*/

		int getSize() const
		{
			return this -> size;
		}
		
		/*-----------------------------------------------------------*/
		
		const T & getFront() const
		{
			if (this -> head)
			{
				return this -> head -> value;
			}
			else
			{
				throw ListException("Empty list.");
			}		
		}
		
		/*-----------------------------------------------------------*/
				
		const T & getBack() const
		{
			if (this -> tail)
			{
				return this -> tail -> value;
			}
			else
			{
				throw ListException("Empty list.");
			}		
		}

	private:
		ListNode<T> * head;
		ListNode<T> * tail;
		int size;
};

template<class T>
class ListNode
{
	friend class List<T>;
	friend class List<T>::Iterator;

	public:
		ListNode() : next(0), prev(0) {}

		/*-----------------------------------------------------------*/

		ListNode(const ListNode & source) : next(0), prev(0), value(source.value) {} 

		/*-----------------------------------------------------------*/

		ListNode(const T & initValue) : next(0), prev(0), value(initValue) {}

		/*-----------------------------------------------------------*/

        const ListNode<T> & operator = (const ListNode<T> & rhs)
		{
			if (this == &rhs)
			{
				return (*this);
			}

			this -> next = 0;
			this -> prev = 0;
			this -> value = rhs.value;

			return (*this);
		}
	
	private:
		T value;
		ListNode<T> * next;
		ListNode<T> * prev;
};


/*-----------------------------------------------------------*/

class IteratorException
{
	public:
		IteratorException(const String &);
		const String & getMessage();
		void setMessage(const String &);
	private:
		String message;
};

/*-----------------------------------------------------------*/

class ListException
{
	public:
		ListException(const String &);
		const String & getMessage();
		void setMessage(const String &);
	private:
		String message;
};

#endif
