/*
	Wiktor Kopec
	Last Modified 04/26/04
*/

#ifndef OBJECT_H
#define OBJECT_H

class Object
{
	friend class Model;

	public:
		enum ObjectType
		{
			Static,
			Usable,
			Programmable,
		};
	public:						
		Object();
		virtual ~Object() {}
		
		int getObjectId() const;
		int getParentId() const;
		bool isCopy() const;
		static int getIDOffset();

	private:	
		int objectId;
		int parentId;
		bool copy;
		
		static int globalObjectCount;
		static const int idOffset = 128;

		void setObjectId(int id);	
		void setParentId(int pid);
		void makeCopy(bool copy);
};

inline Object::Object() : objectId(-1), parentId(-1), copy(false) {}

inline void Object::setObjectId(int id) { objectId = id + idOffset; }

inline int Object::getObjectId() const { return objectId - idOffset; }

inline int Object::getParentId() const { return parentId; }

inline void Object::setParentId(int pid) { parentId = pid; }

inline bool Object::isCopy() const { return copy; }

inline void Object::makeCopy(bool copy) { this -> copy = copy; }

inline int Object::getIDOffset()
{
	return idOffset;
}

#endif
