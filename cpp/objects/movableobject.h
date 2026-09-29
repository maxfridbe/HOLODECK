#ifndef MOVABLEOBJECT_H
#define MOVABLEOBJECT_H

#include "object.h"
#include "matrix.h"
#include "mvector3.h"
#include "wmath.h"
#include "PNode.h"

class MovableObject : public Object
{
	public:
		MovableObject() : Object(), pnode(NULL) {
			pos = MVector3(0,0,0);
			view = MVector3(0,0,1);
			up = MVector3(0,1,0);
		};
		MovableObject(const MVector3 & pos, const MVector3 & view);

		void TranslateTo(float x, float y, float z);
		void TranslateBy(float x, float y, float z);
		void ScaleTo(float x, float y, float z);
		void ScaleBy(float x, float y, float z);

		void RotateXBy(float amountDegrees);
		void RotateYBy(float amountDegrees);
		void RotateZBy(float amountDegrees);
	
		void getScale(float & x, float & y, float & z);
		void getTranslate(float & x, float & y, float & z);
		
		

		Matrix & Rotate();
		Matrix & Translate();
		Matrix & Scale();
		MVector3 & Up();
		MVector3 & Pos();
		MVector3 & View();
		
		void AddPhysicsNode(PNode & node);
		PNode * getPhysicsNode();

	protected:
		PNode * pnode;
		Matrix rotate;
		Matrix trans;
		Matrix scale;
		MVector3 up;
		MVector3 view;
		MVector3 pos;

};

inline MovableObject::MovableObject(const MVector3 & newPos, const MVector3 & newView) : pos(newPos), view(newView), up(MVector3(0,1,0)), pnode(NULL) {}

inline void MovableObject::getScale(float & x, float & y, float & z) { x = scale[0]; y = scale[5];	z = scale[10]; }

inline void MovableObject::getTranslate(float & x, float & y, float & z) {	x = trans[3]; y = trans[7]; z = trans[11]; }

inline Matrix & MovableObject::Rotate() { return rotate; }

inline Matrix & MovableObject::Translate() { return trans; }

inline Matrix & MovableObject::Scale() { return scale; }

inline MVector3 & MovableObject::Up() { return up; }
inline MVector3 & MovableObject::Pos() { return pos; }
inline MVector3 & MovableObject::View() { return view; }

inline PNode * MovableObject::getPhysicsNode()
{
	return pnode;
}

#endif
