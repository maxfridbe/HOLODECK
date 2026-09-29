#include "renderableobject.h"

class TrackingNode
{
	public:
		TrackingNode();
		void LinkToParent( MovableObject * parentToFollow );
		void SetModel(RenderableObject * modelToDraw);
		void SetOffest(MVector3 & offset);
		void Update();

	private:
		MovableObject * parent;
		RenderableObject * model;
		MVector3 offset;

};
