#include "renderableobject.h"

class TrailObject :	public RenderableObject
{
	public:
		TrailObject(MovableObject * obj, bool orient);
		~TrailObject();

		//Override:
		void Draw();

	private:
		bool orientationMatch;
		MovableObject * obj;
};
