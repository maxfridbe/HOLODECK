#include "camera3d.h"
#include "window3d.h"
#include "movableobject.h"
#include "view.h"


Camera3d::Camera3d(MovableObject * cam, Window3d * window, int x, int y, int ViewPortWidth, int ViewPortHeight, ViewPort * view;)
{
	this->x = x;
	this->y = y;

	this->yBottom = y + ViewPortHeight;
	this->camera = cam;
	this->width = ViewPortWidth;
	this->height = ViewPortHeight;
	this->window = window;
	this->view = view;
}

Camera3d::~Camera3d(void)
{
}

void Camera3d::Display()
{
	glMatrixMode(GL_MODEL_VIEW);
	glPushMatrix();
	{
		glLoadIdentity();

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		{
			glLoadIdentity();
			
			ViewPort view;
			view.camera = 

			glViewport(this->x, this->yBottom, this->width, this->height);
			gluLookAt(this->camera->Pos().x, 
			


		}
		glPopMatrix();		
	}
	glMatrixMode(GL_MODEL_VIEW);
	glPopMatrix();

}

Window3d * Camera3d::getParent()
{
	return this->window;
}