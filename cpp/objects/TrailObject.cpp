#include "trailobject.h"
#include "view.h"

TrailObject::TrailObject(MovableObject * obj, bool orient)
{
}

TrailObject::~TrailObject()
{
}

void TrailObject::Draw()
{
	this->Pos();
	this->DrawByMatrix();
}