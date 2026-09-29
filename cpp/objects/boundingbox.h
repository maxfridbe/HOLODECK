#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "mvector3.h"
#include "matrix.h"

class BoundingBox
{
	public:
		BoundingBox();
		BoundingBox(const MVector3 & min, const MVector3 & max);
		BoundingBox(const BoundingBox & source);
		const BoundingBox & operator = (const BoundingBox & rhs);
		const MVector3 & getMin() const;
		const MVector3 & getMax() const;
		const MVector3 * getBounds() const;
		void Transform(const Matrix & rotate, const Matrix & scale, const Matrix & translate);

	private:
		MVector3 bounds[8];
		MVector3 min;
		MVector3 max;
		
};

#endif
