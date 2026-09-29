#include "camera.h"

Camera::Camera(void)
{
	this->phi = 0;
	this->theta = 0;
	this->model = NULL;
	this->up = NULL;
	this->pos = NULL;
	this->view = NULL;
	this->visible = true;
	this->BorderWindow = NULL;
	this->viewportOn = false;
	this->name = String("Camera x");
}

Camera::~Camera(void)
{
}

void Camera::SyncModel()
{
	this->model->TranslateTo(pos->x, pos->y, pos->z);
	this->model->Rotate().LoadIdentity();
	double phiDeg = phi * (180 / PI);
	double thetaDeg = theta * (180 / PI);
	this->model->RotateXBy((float)phiDeg);
	this->model->RotateYBy((float)thetaDeg);
}

Camera::Camera(RenderableObject * obj)
{
	this->model = obj;
}

void Camera::SetCameraModel(RenderableObject * obj)
{
	this->model = obj;
	this->pos = &obj->Pos();
	this->view = &obj->View();
	this->up = &obj->Up();
}

RenderableObject * Camera::GetModel()
{
	return this->model;
}

MVector3 & Camera::Pos()
{
	return *pos;
}
MVector3 & Camera::View()
{
	return *view;
}
MVector3 & Camera::Up()
{
	return *up;
}

double & Camera::Phi()
{
	return this->phi;
}
double & Camera::Theta()
{
	return this->theta;
}

double & Camera::Fov()
{
	return this->fov;
}


void Camera::setPos(MVector3 & pos)
{
	this->pos = &pos;
}
void Camera::setView(MVector3 & view)
{
	this->view = &view;
}
void Camera::setUp(MVector3 & up)
{
	this->up = &up;
}
void Camera::setControl(bool state)
{
	if ( state )
	{
		this->model->setVisible(false);
		this->control = state;
	}
	else
	{
		this->model->setVisible( true );
		this->control = state;
	}
}


