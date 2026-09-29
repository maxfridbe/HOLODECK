#include "boundingbox.h"
#include <float.h>

BoundingBox::BoundingBox() {}

BoundingBox::BoundingBox(const MVector3 & min, const MVector3 & max)
{
	this -> min = min;
	this -> max = max;

	bounds[0] = MVector3(min.x, min.y, min.z);
	bounds[1] = MVector3(min.x, min.y, max.z);
	bounds[2] = MVector3(max.x, min.y, max.z);
	bounds[3] = MVector3(max.x, min.y, min.z);	
	bounds[4] = MVector3(min.x, max.y, min.z);
	bounds[5] = MVector3(min.x, max.y, max.z);
	bounds[6] = MVector3(max.x, max.y, max.z);
	bounds[7] = MVector3(max.x, max.y, min.z);
}

void BoundingBox::Transform(const Matrix & rotate, const Matrix & scale, const Matrix & translate)
{
	for (int i = 0; i < 8; i++)
	{
		scale.VectorMult(bounds[i], bounds[i]);
		rotate.VectorMult(bounds[i], bounds[i]);
		translate.VectorMult(bounds[i], bounds[i]);
	}
	
	min.x = FLT_MAX;
	min.y = FLT_MAX;
	min.z = FLT_MAX;

	max.x = FLT_MIN;
	max.y = FLT_MIN;
	max.z = FLT_MIN;

	for (int i = 0; i < 8; i++)
	{
		min.x = ( (bounds[i].x < min.x) ? bounds[i].x : min.x);
		min.y = ( (bounds[i].y < min.y) ? bounds[i].y : min.y);
		min.z = ( (bounds[i].z < min.z) ? bounds[i].z : min.z);
		max.x = ( (bounds[i].x > max.x) ? bounds[i].x : max.x);
		max.y = ( (bounds[i].y > max.y) ? bounds[i].y : max.y);
		max.z = ( (bounds[i].z > max.z) ? bounds[i].z : max.z);
	}
}

BoundingBox::BoundingBox(const BoundingBox & source)
{
	this -> min = source.min;
	this -> max = source.max;
	memcpy(this -> bounds, source.bounds, sizeof(this -> bounds));
}

const BoundingBox & BoundingBox::operator = (const BoundingBox & rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	this -> min = rhs.min;
	this -> max = rhs.max;
	memcpy(this -> bounds, rhs.bounds, sizeof(this -> bounds));
	return *this;
}

const MVector3 * BoundingBox::getBounds() const
{
	return bounds;
}

const MVector3 & BoundingBox::getMax() const
{
	return max;	
}

const MVector3 & BoundingBox::getMin() const
{
	return min;
}

