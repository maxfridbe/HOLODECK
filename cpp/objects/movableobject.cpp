#include "movableobject.h"

void MovableObject::TranslateTo(float x, float y, float z)
{
	trans[3] = x;
	trans[7] = y;
	trans[11] = z;
	this->pos.x = x;
	this->pos.y = y;
	this->pos.z = z;
	this->view.x = x;
	this->view.y = y;
	this->view.z = z + 1;
}

void MovableObject::TranslateBy(float x, float y, float z)
{
	trans[3] += x;
	trans[7] += y;
	trans[11] += z;
	this->pos.x += x;
	this->pos.y += y;
	this->pos.z += z;
	this->view.x += x;
	this->view.y += y;
	this->view.z += z;
}

void MovableObject::ScaleBy(float x, float y, float z)
{
	//Scaleby uses addition since its interval based, I need to be able to add 0 to it.
	scale[0] += x;
	scale[5] += y;
	scale[10] += z;

	if (scale[0] == 0)
	{
		scale[0] = 0.001f;
	}
	if (scale[5] == 0)
	{
		scale[5] = 0.001f;
	}
	if (scale[10] == 0)
	{
		scale[10] = 0.001f;
	}

}

void MovableObject::ScaleTo(float x, float y, float z)
{
	if ( x == 0 )
	{
		x = 0.001f;
	}
	if ( y == 0 )
	{
		y = 0.001f;
	}
	if ( z == 0 )
	{
		z = 0.001f;
	}

	scale[0] = x;
	scale[5] = y;
	scale[10] = z;
}

void MovableObject::RotateXBy(float amount)
{

	amount = WMath::DegreeToRadian(amount);
	float c = cos(amount);
	float s = sin(amount);

	Matrix R(	1,	0,	0,	0,
				0,	c,	-s,	0,
				0,	s,	c,	0,
				0,	0,	0,	1);
        
	this->rotate = R * rotate;
	//view = MVector3(0.0, 0.0, 1.0f);
	//MVector3 viewCopy = view;
	//rotate.VectorMult(viewCopy, view);
	//view += pos;
}
void MovableObject::RotateYBy(float amount)
{

	amount = WMath::DegreeToRadian(amount);
	float c = cos(amount);
	float s = sin(amount);

	Matrix R(	c,	0,	s,	0,
				0,	1,	0,	0,
				-s,	0,	c,	0,
				0,	0,	0,	1);
    
	this->rotate = R * rotate;
	//view = MVector3(0.0, 0.0, 1.0f);
	//MVector3 viewCopy = view;
	//rotate.VectorMult(viewCopy, view);
	//view += pos;
}

void MovableObject::RotateZBy(float amount)
{
	amount = WMath::DegreeToRadian(amount);
	float c = cos(amount);
	float s = sin(amount);

	Matrix R(	c,	-s,	0,	0,
				s,	c,	0,	0,
				0,	0,	1,	0,
				0,	0,	0,	1);
        
	this->rotate = R * rotate;
	//view = MVector3(0.0, 0.0, 1.0f);
	//MVector3 viewCopy = view;
	//rotate.VectorMult(viewCopy, view);
	//view += pos;
}

void MovableObject::AddPhysicsNode(PNode & node)
{
	this -> pnode = &node;
}



/*
void Object::ResetForceAccum() 
{
	forceAccum = MVector3 (0.0, 0.0, 0.0);
}
*/

//Accessors::::::::

