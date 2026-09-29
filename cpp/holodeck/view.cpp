#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <gl/gl.h>
#include <gl/glu.h>

#include "object.h"
#include "inputhandler.h"
#include "settings.h"
#include "userInterface.h"
#include "wtime.h"
#include "view.h"

ViewPort::ViewPort()
{

	posFront = MVector3(0,0,1000.0f);
	posBack =  MVector3(0,0,-1000.0f);
	posLeft = MVector3(-1000,0,0.0f);
	posRight =  MVector3(1000.0f,0,0.0f);
	posTop = MVector3(0,1000.0f,0.0f);
	posBottom = MVector3(0,-1000.0f,0.0f);
	this->oldPhi = 0;
	this->oldTheta = 0.0f;
	this->phi = 0;
	this->theta = 0.0f;
	this->posLastPos = MVector3(0.0f,0.0f,0.0f);
	this->fov = 45.0f;
	this->perspectiveMode = true;
	this->viewType = ViewPort::Perspective;
	this->startRange = 1.0f;
	this->endRange = 4000.0f;
	this->height = 768;
	this->width = 1024;
	this->zoomLevel = 100;
	this->camera = NULL;
	this->objectToFocus = NULL;
	this->x = 0;
	this->y = 0;

}

//  This function is used to update the location of the camera.
void ViewPort::update()
{	
//	camera->trans.VectorMult(camera->pos, camera->pos);
//	camera->trans.VectorMult(camera->view, camera->view);

	glViewport(0, 0, width, height);

	//set View coord.
	MVector3 rotationVector = camera->View() - camera->Pos();
	camera->View() = camera->Pos() + MVector3::rotate(rotationVector, camera->Theta(), camera->Phi() + HALFPI);

	if ( Settings::WireframeMode )
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if ( viewType == ViewPort::Perspective)
	{
		if ( this->objectToFocus == NULL)
			gluLookAt(camera->Pos().x, camera->Pos().y, camera->Pos().z, camera->View().x, camera->View().y, camera->View().z, camera->Up().x, camera->Up().y, camera->Up().z);
		else
			gluLookAt(camera->Pos().x, camera->Pos().y, camera->Pos().z, objectToFocus->Pos().x, objectToFocus->Pos().y, objectToFocus->Pos().z, camera->Up().x, camera->Up().y, camera->Up().z);		
	}

	if ( viewType == ViewPort::Front )
	{
		camera->View().x = camera->Pos().x;
		camera->View().y = camera->Pos().y;
		camera->View().z = camera->Pos().z - 6;

		posFront = camera->Pos();
		gluLookAt(camera->Pos().x, camera->Pos().y, camera->Pos().z, camera->View().x, camera->View().y, camera->View().z, camera->Up().x, camera->Up().y, camera->Up().z);		
	}
	else if ( viewType == ViewPort::Back )
	{
		camera->View().x = camera->Pos().x;
		camera->View().y = camera->Pos().y;
		camera->View().z = camera->Pos().z + 6;

		posBack = camera->Pos();
		gluLookAt(camera->Pos().x, camera->Pos().y, camera->Pos().z, camera->View().x, camera->View().y, camera->View().z, camera->Up().x, camera->Up().y, camera->Up().z);		
	}
	else if ( viewType == ViewPort::Left )
	{
		camera->View().x = camera->Pos().x + 6;
		camera->View().y = camera->Pos().y;
		camera->View().z = camera->Pos().z;

		posLeft = camera->Pos();
		gluLookAt(camera->Pos().x, camera->Pos().y, camera->Pos().z, camera->View().x, camera->View().y, camera->View().z, camera->Up().x, camera->Up().y, camera->Up().z);		
	}
	else if ( viewType == ViewPort::Right)
	{
		camera->View().x = camera->Pos().x - 6;
		camera->View().y = camera->Pos().y;
		camera->View().z = camera->Pos().z;

		posRight = camera->Pos();
		gluLookAt(camera->Pos().x, camera->Pos().y, camera->Pos().z, camera->View().x, camera->View().y, camera->View().z, camera->Up().x, camera->Up().y, camera->Up().z);		
	}
	else if ( viewType == ViewPort::Top)
	{
		camera->View().x = camera->Pos().x;
		camera->View().y = camera->Pos().y - 6;
		camera->View().z = camera->Pos().z;

		posTop = camera->Pos();
		gluLookAt(camera->Pos().x, camera->Pos().y, camera->Pos().z, camera->View().x, camera->View().y, camera->View().z, camera->Up().x, camera->Up().y, camera->Up().z);		
	}
	else if (viewType == ViewPort::Bottom)
	{
		camera->View().x = camera->Pos().x;
		camera->View().y = camera->Pos().y + 6;
		camera->View().z = camera->Pos().z;

		posBottom = camera->Pos();
		gluLookAt(camera->Pos().x, camera->Pos().y, camera->Pos().z, camera->View().x, camera->View().y, camera->View().z, camera->Up().x, camera->Up().y, camera->Up().z);		
	}
}

void ViewPort::QuickCamView(Camera * quickCam, Model & model, int ScreenX, int ScreenY, int ScreenWidth, int ScreenHeight)
{
	if (!quickCam->BorderWindow)
	{
		quickCam->BorderWindow = UserInterface::AddVideoWindow(30,30,200,200);
		Window3d * BorderWindow = quickCam->BorderWindow;
		BorderWindow->setWidth( ScreenWidth + 60);
		BorderWindow->setHeight( ScreenHeight + 60);
		ScreenX = BorderWindow->getX() + 30;
		ScreenY = Settings::WindowHeight - BorderWindow->getY() - BorderWindow->getHeight() + 30;
	}
	else
	{
		Window3d * BorderWindow = quickCam->BorderWindow;
		BorderWindow->setWidth( ScreenWidth + 60 );
		BorderWindow->setHeight( ScreenHeight + 60);
		ScreenX = BorderWindow->getX() + 30;
		ScreenY = Settings::WindowHeight - BorderWindow->getY() - BorderWindow->getHeight() + 30;
	}
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	{
		//Clear the depth buffer so we get a new buffer for depths.
		glClear(GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		glViewport(ScreenX, ScreenY, ScreenWidth, ScreenHeight);
		gluPerspective(fov, (float)ScreenWidth / (float)ScreenHeight, startRange, endRange);	

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		{			
			glLoadIdentity();

			
			MVector3 rotationVector = quickCam->View() - quickCam->Pos();
			quickCam->View() = quickCam->Pos() + MVector3::rotate(rotationVector, quickCam->Theta(), quickCam->Phi() + HALFPI);

            gluLookAt(quickCam->Pos().x, quickCam->Pos().y, quickCam->Pos().z, quickCam->View().x, quickCam->View().y, quickCam->View().z, quickCam->Up().x, quickCam->Up().y, quickCam->Up().z);

			this->DrawGrid();
			model.Draw();
		}
		glPopMatrix();
	}
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	//Model view matrix is more common.
	glMatrixMode(GL_MODELVIEW);
//	glEnable(GL_DEPTH_TEST);
}

void ViewPort::DrawAxis()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
		glLoadIdentity();
		glViewport(10, 10, 50, 50);
		
		//this->init(width, height, fov);
		//gluPerspective(fov, 1, 0.5, 50);
		glOrtho(-10,10,-10, 10,-100, 500);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
			glLoadIdentity();
			gluLookAt(-camera->View().x + camera->Pos().x, -camera->View().y + camera->Pos().y, -camera->View().z  + camera->Pos().z, 0, 0, 0, camera->Up().x, camera->Up().y, camera->Up().z);		

			int reenable = 0;
			glGetIntegerv(GL_DEPTH_TEST, &reenable);
			if ( reenable )
			{
				glDisable(GL_DEPTH_TEST);
			}

			glColor3ub(0,255,0);
			UserInterface::PrintGL(0,10,0, "Y");
			glColor3ub(0,128,255);
			UserInterface::PrintGL(0,0,10, "Z");
			glColor3ub(255,0,0);
			UserInterface::PrintGL(10,0,0, "X");
			
			glBegin(GL_LINES);
			{
				glLineWidth(3.0f);
				//Y Axis Green
				glColor3ub(0,255,0);
				glVertex3i(0,0,0);
				glVertex3i(0,10,0);
				
				
				//Z axis 
				glColor3ub(0,128,255);
				glVertex3i(0,0,0);
				glVertex3i(0,0,10);
				
				
				//x axis Red
				glColor3ub(255,0,0);
				glVertex3i(0,0,0);
				glVertex3i(10,0,0);
				

			}
			glEnd();

			if ( reenable )
			{
				glEnable(GL_DEPTH_TEST);
			}

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void ViewPort::UpdateProjectionMatrix()
{
	if ( this->perspectiveMode )
	{
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fov, (float)width / (float)height, startRange, endRange);	
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	}
	else
	{
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-zoomLevel * (float)width / 2000, zoomLevel * (float)width / 2000, -zoomLevel * (float)height / 2000, zoomLevel * (float)height / 2000, 0.5, 8000);	
	}
}

void ViewPort::RenderView()
{
	if ( this->perspectiveMode )
	{
		gluPerspective(fov, (float)width / (float)height, startRange, endRange);	
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	}
	else
	{
		glOrtho(-zoomLevel * (float)width / 2000, zoomLevel * (float)width / 2000, -zoomLevel * (float)height / 2000, zoomLevel * (float)height / 2000, 0.5, 8000);	
	}
}

void ViewPort::ZoomIn(float amount)
{
	if ( zoomLevel - amount >= 1 )
		this->zoomLevel -= amount;
	else
		zoomLevel = 1;
}

void ViewPort::ZoomOut(float amount)
{
	zoomLevel += amount;
}

float ViewPort::ZoomPercent(float percent)
{
	float amt = percent / 100.0f;
	return zoomLevel * amt;
}


//Specific all one time inits.
void ViewPort::init(int w, int h, double newFov, double newNear, double newFar)
{
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	this -> fov = newFov;
//	this->zoomLevel = (1 / newFov) * 300 / 45;
	this -> startRange = newNear;
	this -> endRange = newFar;
	this -> width = w;
	this -> height = h;
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);	

	if ( this->perspectiveMode )
	{
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fov, (float)w / (float)h, startRange, endRange);	
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	}
	else
	{
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-zoomLevel * (float)width / 2000, zoomLevel * (float)width / 2000, -zoomLevel * (float)height / 2000, zoomLevel * (float)height / 2000, 0.5, 8000);	
	}

	//FreeCam
	this->objectToFocus = NULL;
}

void ViewPort::MakePerspective()
{
	//Settings::Grid::SetGridColor(Settings::Grid::Yellow);
	Settings::Grid::SetGridSpacing(3, 100);
	Camera & camera = getCamera();

	if (viewType != ViewPort::Perspective )
	{
		this->phi = this->oldPhi;
		this->theta = this->oldTheta;
		camera.Pos() = this->posLastPos;
	}
	
	camera.Up() = MVector3(0.0f,1.0f,0.0f);
	this->viewType = ViewPort::Perspective;
	this->perspectiveMode = true;
	this->UpdateProjectionMatrix();
}

void ViewPort::MakeOrthoFront()
{
	//Settings::Grid::SetGridColor(Settings::Grid::Blue);
	Settings::Grid::SetGridSpacing(2, 64);
	Camera & camera = getCamera();
	//Store Pervious viewPosition.
	if (viewType == ViewPort::Perspective )
	{
		this->oldPhi = this->phi;
		this->oldTheta = this->theta;
		this->posLastPos = camera.Pos();
	}
	
	camera.Pos() = posFront;
	camera.Up() = MVector3(0.0f,1.0f,0.0f);
	this->viewType = ViewPort::Front;
	this->perspectiveMode = false;
	this->UpdateProjectionMatrix();
}

void ViewPort::MakeOrthoBack()
{
	//Settings::Grid::SetGridColor(Settings::Grid::Blue);
	Settings::Grid::SetGridSpacing(2, 64);
	Camera & camera = getCamera();
	//Store Pervious viewPosition.
	if (viewType == ViewPort::Perspective )
	{
		this->oldPhi = this->phi;
		this->oldTheta = this->theta;
		this->posLastPos = camera.Pos();
	}
	camera.Up() = MVector3(0.0f,1.0f,0.0f);
	camera.Pos() = posBack;
	this->viewType = ViewPort::Back;
	this->perspectiveMode = false;
	this->UpdateProjectionMatrix();
}
void ViewPort::MakeOrthoLeft()
{
	//Settings::Grid::SetGridColor(Settings::Grid::Red);
	Settings::Grid::SetGridSpacing(2, 64);
	Camera & camera = getCamera();
	//Store Pervious viewPosition.
	if (viewType == ViewPort::Perspective )
	{
		this->oldPhi = this->phi;
		this->oldTheta = this->theta;
		this->posLastPos = camera.Pos();
	}
	camera.Up() = MVector3(0.0f,1.0f,0.0f);
	camera.Pos() = posLeft;
	this->viewType = ViewPort::Left;
	this->perspectiveMode = false;
	this->UpdateProjectionMatrix();
}
void ViewPort::MakeOrthoRight()
{
	//Settings::Grid::SetGridColor(Settings::Grid::Red);
	Settings::Grid::SetGridSpacing(2, 64);
	Camera & camera = getCamera();
	//Store Pervious viewPosition.
	if (viewType == ViewPort::Perspective )
	{
		this->oldPhi = this->phi;
		this->oldTheta = this->theta;
		this->posLastPos = camera.Pos();
	}
	camera.Up() = MVector3(0.0f,1.0f,0.0f);
	camera.Pos() = posRight;
	this->viewType = ViewPort::Right;
	this->perspectiveMode = false;
	this->UpdateProjectionMatrix();
}
void ViewPort::MakeOrthoTop()
{
	//Settings::Grid::SetGridColor(Settings::Grid::Green);
	Settings::Grid::SetGridSpacing(1, 64);
	Camera & newCam = getCamera();
	//Store Pervious viewPosition.
	if (viewType == ViewPort::Perspective )
	{
		this->oldPhi = this->phi;
		this->oldTheta = this->theta;
		this->posLastPos = newCam.Pos();
	}
	newCam.Up() = MVector3(1.0f,0.0f,0.0f);
	newCam.Pos() = posTop;
	this->viewType = ViewPort::Top;
	this->perspectiveMode = false;
	this->UpdateProjectionMatrix();
}
void ViewPort::MakeOrthoBottom()
{
	//Settings::Grid::SetGridColor(Settings::Grid::Green);
	Settings::Grid::SetGridSpacing(1, 64);
	Camera & newCam = getCamera();
	//Store Pervious viewPosition.
	if (viewType == ViewPort::Perspective )
	{
		this->oldPhi = this->phi;
		this->oldTheta = this->theta;
		this->posLastPos = newCam.Pos();
	}
	newCam.Up() = MVector3(-1.0f,0.0f,0.0f);
	newCam.Pos() = posBottom;
	this->viewType = ViewPort::Bottom;
	this->perspectiveMode = false;
	this->UpdateProjectionMatrix();
}

void ViewPort::DrawGridBody()
{

#pragma warning( disable : 4244 )
	//floor
	glColor3ub(Settings::Grid::bred, Settings::Grid::bgreen, Settings::Grid::bblue);
	glBegin(GL_QUADS);
		glVertex3f( -1, -1, -1);
		glVertex3f( Settings::Grid::finalW + 1, -1, -1);
		glVertex3f( Settings::Grid::finalW + 1, -1, Settings::Grid::finalD + 1);
		glVertex3f( -1, -1, Settings::Grid::finalD + 1);
	glEnd();
	
	//front
	glBegin(GL_QUADS);
		glVertex3f( -1, -1, -1);
		glVertex3f( Settings::Grid::finalW + 1, -1, -1);
		glVertex3f( Settings::Grid::finalW + 1, Settings::Grid::finalH + 1, -1 );
		glVertex3f( -1, Settings::Grid::finalH + 1, -1);
	glEnd();

	//Back
	glBegin(GL_QUADS);
		glVertex3f( -1, -1, Settings::Grid::finalD + 1 );
		glVertex3f( Settings::Grid::finalW + 1, -1, Settings::Grid::finalD + 1 );
		glVertex3f( Settings::Grid::finalW + 1, Settings::Grid::finalH + 1, Settings::Grid::finalD + 1  );
		glVertex3f( -1, Settings::Grid::finalH + 1, Settings::Grid::finalD + 1 );
	glEnd();

	//side
	glBegin(GL_QUADS);
		glVertex3f( -1, -1, -1);
		glVertex3f( -1, Settings::Grid::finalH + 1, -1);
		glVertex3f( -1, Settings::Grid::finalH + 1, Settings::Grid::finalD + 1);
		glVertex3f( -1, -1, Settings::Grid::finalD + 1);
	glEnd();

	//side
	glBegin(GL_QUADS);
		glVertex3f( Settings::Grid::finalW + 1, -1, -1);
		glVertex3f( Settings::Grid::finalW + 1, Settings::Grid::finalH + 1, -1);
		glVertex3f( Settings::Grid::finalW + 1, Settings::Grid::finalH + 1, Settings::Grid::finalD + 1);
		glVertex3f( Settings::Grid::finalW + 1, -1, Settings::Grid::finalD + 1);
	glEnd();

	//top
	glBegin(GL_QUADS);
		glVertex3f( Settings::Grid::finalW + 1, Settings::Grid::finalH + 1, -1);
		glVertex3f( Settings::Grid::finalW + 1, Settings::Grid::finalH + 1, Settings::Grid::finalD + 1);
		glVertex3f( -1, Settings::Grid::finalH + 1, Settings::Grid::finalD + 1);
		glVertex3f( -1, Settings::Grid::finalH + 1, -1);
	glEnd();


	glColor3ub(Settings::Grid::red,Settings::Grid::green,Settings::Grid::blue);

	//floor
	for ( int i = 0; i <= Settings::Grid::finalW; i+=Settings::Grid::spacing)
	{
		glBegin(GL_QUADS);
			glVertex3f( i, 0, 0);
			glVertex3f( i, 0, Settings::Grid::finalD);
			glVertex3f( i + Settings::Grid::thickness, 0, Settings::Grid::finalD);
			glVertex3f( i + Settings::Grid::thickness, 0, 0);
		glEnd();
	}
	for ( int i = 0; i <= Settings::Grid::finalD; i+=Settings::Grid::spacing)
	{
		glBegin(GL_QUADS);
			glVertex3f( 0, 0, i);
			glVertex3f( Settings::Grid::finalW, 0, i );
			glVertex3f( Settings::Grid::finalW, 0,i + Settings::Grid::thickness );
			glVertex3f( 0 , 0, i + Settings::Grid::thickness );
		glEnd();
	}

	// top
	for ( int i = 0; i <= Settings::Grid::finalW; i+=Settings::Grid::spacing)
	{
		glBegin(GL_QUADS);
			glVertex3f( i, Settings::Grid::finalH, 0);
			glVertex3f( i, Settings::Grid::finalH, Settings::Grid::finalD);
			glVertex3f( i + Settings::Grid::thickness, Settings::Grid::finalH, Settings::Grid::finalD);
			glVertex3f( i + Settings::Grid::thickness, Settings::Grid::finalH, 0);
		glEnd();
	}
	for ( int i = 0; i <= Settings::Grid::finalD; i+=Settings::Grid::spacing)
	{
		glBegin(GL_QUADS);
			glVertex3f( 0, Settings::Grid::finalH, i);
			glVertex3f( Settings::Grid::finalW, Settings::Grid::finalH, i );
			glVertex3f( Settings::Grid::finalW, Settings::Grid::finalH,i + Settings::Grid::thickness );
			glVertex3f( 0 , Settings::Grid::finalH, i + Settings::Grid::thickness );
		glEnd();
	}

	//Front
	for ( int i = 0; i <= Settings::Grid::finalW; i+=Settings::Grid::spacing)
	{
		glBegin(GL_QUADS);
			glVertex3f( i, 0, 0);
			glVertex3f( i, Settings::Grid::finalH, 0);
			glVertex3f( i + Settings::Grid::thickness, Settings::Grid::finalH, 0);
			glVertex3f( i + Settings::Grid::thickness, 0, 0);
		glEnd();
	}
	for ( int i = 0; i <= Settings::Grid::finalH; i+=Settings::Grid::spacing)
	{
		glBegin(GL_QUADS);
			glVertex3f( 0, i, 0);
			glVertex3f( Settings::Grid::finalW, i, 0);
			glVertex3f( Settings::Grid::finalW,i + Settings::Grid::thickness, 0);
			glVertex3f( 0, i + Settings::Grid::thickness, 0);
		glEnd();
	}

	//back
	for ( int i = 0; i <= Settings::Grid::finalW; i+=Settings::Grid::spacing)
	{
		glBegin(GL_QUADS);
			glVertex3f( i, 0, Settings::Grid::finalD);
			glVertex3f( i, Settings::Grid::finalH, Settings::Grid::finalD);
			glVertex3f( i + Settings::Grid::thickness, Settings::Grid::finalH, Settings::Grid::finalD);
			glVertex3f( i + Settings::Grid::thickness, 0, Settings::Grid::finalD);
		glEnd();
	}
	for ( int i = 0; i <= Settings::Grid::finalH; i+=Settings::Grid::spacing)
	{
		glBegin(GL_QUADS);
			glVertex3f( 0, i, Settings::Grid::finalD);
			glVertex3f( Settings::Grid::finalW, i, Settings::Grid::finalD);
			glVertex3f( Settings::Grid::finalW,i + Settings::Grid::thickness, Settings::Grid::finalD);
			glVertex3f( 0, i + Settings::Grid::thickness, Settings::Grid::finalD);
		glEnd();
	}

	//Side
	for ( int i = 0; i <= Settings::Grid::finalD; i+=Settings::Grid::spacing)
	{
		glBegin(GL_QUADS);
			glVertex3f( Settings::Grid::finalW, 0, i );
			glVertex3f( Settings::Grid::finalW, Settings::Grid::finalH,i );
			glVertex3f( Settings::Grid::finalW, Settings::Grid::finalH,i + Settings::Grid::thickness );
			glVertex3f( Settings::Grid::finalW, 0, i + Settings::Grid::thickness);
		glEnd();
	}
	for ( int i = 0; i <= Settings::Grid::finalH; i+=Settings::Grid::spacing)
	{
		glBegin(GL_QUADS);
			glVertex3f( Settings::Grid::finalW, i, 0 );
			glVertex3f( Settings::Grid::finalW, i,Settings::Grid::finalD );
			glVertex3f( Settings::Grid::finalW,i + Settings::Grid::thickness, Settings::Grid::finalD);
			glVertex3f( Settings::Grid::finalW, i + Settings::Grid::thickness, 0);
		glEnd();
	}
	
	//side 2
	for ( int i = 0; i <= Settings::Grid::finalD; i+=Settings::Grid::spacing)
	{
		glBegin(GL_QUADS);
			glVertex3f( 0, 0, i );
			glVertex3f( 0, Settings::Grid::finalH,i );
			glVertex3f( 0, Settings::Grid::finalH,i + Settings::Grid::thickness );
			glVertex3f( 0, 0, i + Settings::Grid::thickness);
		glEnd();
	}
	for ( int i = 0; i <= Settings::Grid::finalH; i+=Settings::Grid::spacing)
	{
		glBegin(GL_QUADS);
			glVertex3f( 0, i, 0 );
			glVertex3f( 0, i,Settings::Grid::finalD );
			glVertex3f( 0,i + Settings::Grid::thickness, Settings::Grid::finalD);
			glVertex3f( 0, i + Settings::Grid::thickness, 0);
		glEnd();
	}

#pragma warning( default : 4244 )

}

void ViewPort::DrawGridLines()
{
	glLineWidth(1.0f);

	glColor3ub(Settings::Grid::bred, Settings::Grid::bgreen, Settings::Grid::bblue);
	float width = (float) Settings::Grid::finalW;
	float height = (float)Settings::Grid::finalH;
	float depth = (float)Settings::Grid::finalD;
	float spacing = (float)Settings::Grid::spacing;
	
#pragma warning( disable : 4244 )

	glColor3ub(0,255,0);
	//Front Right Lines XY plane on Z axis (Verticle)
	for ( int i = width / 2 + spacing; i < width / 2 + 1000; i += spacing)
	{
		glBegin(GL_LINES);
			glVertex3f(i, height / 2 + 1000, depth / 2);
			glVertex3f(i, height / 2 - 1000, depth / 2);
		glEnd();
	}
	//Front left
	for ( int i = width / 2 - spacing; i > width / 2 - 1000; i -= spacing)
	{
		glBegin(GL_LINES);
			glVertex3f(i, height / 2 + 1000, depth / 2);
			glVertex3f(i, height / 2 - 1000, depth / 2);
		glEnd();
	}

	glColor3ub(255,0,0);
	//Front Right Lines ( Horizontal )
	for ( int i = width / 2 + spacing; i < width / 2 + 1000; i += spacing)
	{
		glBegin(GL_LINES);
			glVertex3f(width / 2 + 1000, i, depth / 2);
			glVertex3f(width / 2 - 1000, i, depth / 2);
		glEnd();
	}
	//Front left
	for ( int i = width / 2 - spacing; i > width / 2 - 1000; i -= spacing)
	{
		glBegin(GL_LINES);
			glVertex3f(width / 2 + 1000, i, depth / 2);
			glVertex3f(width / 2 - 1000, i, depth / 2);
		glEnd();
	}

	//Side Right Lines YZ plane on X Axis
	glColor3ub(0,255,0);
	for ( int i = depth / 2 + spacing; i < depth / 2 + 1000; i += spacing)
	{
		glBegin(GL_LINES);
			glVertex3f(width / 2, height / 2 + 1000, i);
			glVertex3f(width / 2, height / 2 - 1000, i);
		glEnd();
	}
	//Side left
	for ( int i = depth / 2 - spacing; i > depth / 2 - 1000; i -= spacing)
	{
		glBegin(GL_LINES);
			glVertex3f(width / 2, height / 2 + 1000, i);
			glVertex3f(width / 2, height / 2 - 1000, i);
		glEnd();
	}
	//Side Right Lines ( horizontal )
	glColor3ub(0,0,255);
	for ( int i = depth / 2 + spacing; i < depth / 2 + 1000; i += spacing)
	{
		glBegin(GL_LINES);
			glVertex3f(width / 2, i, depth / 2 + 1000);
			glVertex3f(width / 2, i, depth / 2 - 1000);
		glEnd();
	}
	//Side left
	for ( int i = depth / 2 - spacing; i > depth / 2 - 1000; i -= spacing)
	{
		glBegin(GL_LINES);
			glVertex3f(width / 2, i, depth / 2 + 1000);
			glVertex3f(width / 2, i, depth / 2 - 1000);
		glEnd();
	}

	//top Right Lines XZ plane on Y Axis
	glColor3ub(0,0,255);
	for ( int i = depth / 2 + spacing; i < depth / 2 + 1000; i += spacing)
	{
		glBegin(GL_LINES);
			glVertex3f(i, height / 2 , depth / 2 + 1000);
			glVertex3f(i, height / 2 , depth / 2 - 1000);
		glEnd();
	}
	//top left
	for ( int i = depth / 2 - spacing; i > depth / 2 - 1000; i -= spacing)
	{
		glBegin(GL_LINES);
			glVertex3f(i, height / 2 , depth / 2 + 1000);
			glVertex3f(i, height / 2 , depth / 2 - 1000);
		glEnd();
	}
	//top Right Lines ( horizontal )
	glColor3ub(255,0,0);
	for ( int i = width / 2 + spacing; i < width / 2 + 1000; i += spacing)
	{
		glBegin(GL_LINES);
			glVertex3f(width / 2 + 1000, height / 2 , i);
			glVertex3f(width / 2 - 1000, height / 2 , i);
		glEnd();
	}
	//top left
	for ( int i = depth / 2 - spacing; i > depth / 2 - 1000; i -= spacing)
	{
		glBegin(GL_LINES);
			glVertex3f(width / 2 + 1000, height / 2 , i);
			glVertex3f(width / 2 - 1000, height / 2 , i);
		glEnd();
	}


#pragma warning ( default : 4244)

	// AXIS
	glLineWidth(3.0f);
	glColor3ub(255,255,255);
	//y
	glBegin(GL_LINES);
		glVertex3f(width / 2, height / 2 + 1000, depth / 2);
		glVertex3f(width / 2, height / 2 - 1000, depth / 2);
	glEnd();

	//X
	glBegin(GL_LINES);
		glVertex3f(width / 2 + 1000, height / 2, depth / 2);
		glVertex3f(width / 2 - 1000, height / 2, depth / 2);
	glEnd();

	//z
	glBegin(GL_LINES);
		glVertex3f(width / 2, height / 2, depth / 2 + 1000);
		glVertex3f(width / 2, height / 2, depth / 2 - 1000);
	glEnd();


}


void ViewPort::DrawGrid()
{

	if ( Settings::InnerGrid || viewType != ViewPort::Perspective )
	{
		glPushMatrix();

			glTranslatef((float) -1 * Settings::Grid::finalW /2 ,(float) -1 * Settings::Grid::finalH /2, (float) -1 * Settings::Grid::finalD /2);
			glScalef(Settings::Grid::scale, Settings::Grid::scale, Settings::Grid::scale);		
			DrawGridLines();
		glPopMatrix();
	}
		
	if ( Settings::Grid::gridState == States::Grid::OFF )
	{
		glColor3ub(255,255,255);
		UserInterface::PrintGL(1000,0,0, "X");
		UserInterface::PrintGL(0,1000,0, "Y");
		UserInterface::PrintGL(0,0,1000, "Z");

		UserInterface::PrintGL(-1000,0,0, "X");
		UserInterface::PrintGL(0,-1000,0, "Y");
		UserInterface::PrintGL(0,0,-1000, "Z");
		return;
	}

	else if ( Settings::Grid::gridState == States::Grid::ON )
	{
		glPushMatrix();
		{
			glTranslatef((float) -1 * Settings::Grid::finalW /2 ,(float) -1 * Settings::Grid::finalH /2, (float) -1 * Settings::Grid::finalD /2);
			glScalef(Settings::Grid::scale, Settings::Grid::scale, Settings::Grid::scale);
				
			if ( this->viewType == ViewPort::Perspective )
			{	
				DrawGridBody();	
			}
		}
		glPopMatrix();
	}

	else if ( Settings::Grid::gridState == States::Grid::Enable )
	{
		glPushMatrix();
		{	

			glScalef(Settings::Grid::scale, Settings::Grid::dynamicScale, Settings::Grid::scale);			
			glTranslatef((float) -1 * Settings::Grid::finalW /2 ,(float) -1 * Settings::Grid::finalH /2, (float) -1 * Settings::Grid::finalD /2);
			Settings::Grid::dynamicScale += 1.0f * static_cast<float>(Time::getInterval());

			if ( Settings::Grid::dynamicScale >= Settings::Grid::scale )
			{
				Settings::Grid::gridState = States::Grid::ON;
				Settings::Grid::dynamicScale = (float)Settings::Grid::scale;	
			}
			
			DrawGridBody();	
		}
		glPopMatrix();
	}
    else if ( Settings::Grid::gridState == States::Grid::Disable )
	{
		glPushMatrix();
		{
			glScalef(Settings::Grid::scale, Settings::Grid::dynamicScale, Settings::Grid::scale);			
			glTranslatef((float) -1 * Settings::Grid::finalW /2 ,(float) -1 * Settings::Grid::finalH /2, (float) -1 * Settings::Grid::finalD /2);			
			Settings::Grid::dynamicScale -= 1.0f * static_cast<float>(Time::getInterval());

			if ( Settings::Grid::dynamicScale <= 0 )
			{
				Settings::Grid::gridState = States::Grid::OFF;
				Settings::Grid::dynamicScale = 0.0f;	
			}			
			
			DrawGridBody();	
		}
		glPopMatrix();

	}
		
	int reEnable = 0;
	glGetIntegerv(GL_DEPTH_TEST, &reEnable);
	
	if ( reEnable)
	{
		glDisable(GL_DEPTH_TEST);
	}

	glColor3ub(255,255,255);
	UserInterface::PrintGL(1000,0,0, "X");
	UserInterface::PrintGL(0,1000,0, "Y");
	UserInterface::PrintGL(0,0,1000, "Z");

	UserInterface::PrintGL(-1000,0,0, "X");
	UserInterface::PrintGL(0,-1000,0, "Y");
	UserInterface::PrintGL(0,0,-1000, "Z");

	if ( reEnable )
	{
		glEnable(GL_DEPTH_TEST);
	}
	return;
}

void ViewPort::DrawColorCube(float x, float y, float z)
{
	//The GL MODELVIEW matrix Onto the Stack 
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glScalef(2.0f, 2.0f, 2.0f);
		glTranslatef((float)x,(float)y,(float)z);

		glBegin(GL_QUADS);
		//face back ( green blue ):
			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex3i(-1,-1,-1);

			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex3i(-1,1,-1);

			glColor3f(0.0f, 1.0f, 1.0f);
			glVertex3i(-1,1,1);

			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3i(-1,-1,1);
		//green red side
		glEnd();
		glBegin(GL_QUADS);
			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex3i(-1,-1,-1);

			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex3i(-1,1,-1);

			glColor3f(1.0f, 1.0f, 0.0f);
			glVertex3i(1,1,-1);

			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3i(1,-1,-1);

		// top
		glEnd();
		glBegin(GL_QUADS);
			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex3i(-1,1,-1);

			glColor3f(0.0f, 1.0f, 1.0f);
			glVertex3i(-1,1,1);

			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3i(1,1,1);

			glColor3f(1.0f, 1.0f, 0.0f);
			glVertex3i(1,1,-1);

		glEnd();
		glBegin(GL_QUADS);
		// bottom
			glColor3f(0.0f, 0.0f, 0.0f);
			glVertex3i(-1,-1,-1);

			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3i(-1,-1,1);

			glColor3f(1.0f, 0.0f, 1.0f);
			glVertex3i(1,-1,1);

			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3i(1,-1,-1);

		glEnd();
		glBegin(GL_QUADS);
		// blueish? right side:
			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3i(-1,-1,1);

			glColor3f(1.0f, 0.0f, 1.0f);
			glVertex3i(1,-1,1);

			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3i(1,1,1);

			glColor3f(0.0f, 1.0f, 1.0f);
			glVertex3i(-1,1,1);

		glEnd();
		glBegin(GL_QUADS);
		//Front brights:
			glColor3f(1.0f, 0.0f, 1.0f);
			glVertex3i(1,-1,1);

			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3i(1,-1,-1);

			glColor3f(1.0f, 1.0f, 0.0f);
			glVertex3i(1,1,-1);

			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3i(1,1,1);			
		glEnd();		

	//End the plain Stack 
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


void ViewPort::setCamera(Camera & camera)
{
	this -> camera = &camera;
}

//Have the current Object focus on another object.
void ViewPort::setFocus(MovableObject & camera)
{
	this -> objectToFocus = &camera;
}

//Return to Free Cam
void ViewPort::unFocus()
{
	this -> objectToFocus = NULL;
}


Camera & ViewPort::getCamera()
{
	if ( camera )
		return *camera;
	else
	{
		Camera X;
		return X;
	}
}

double ViewPort::getFov() const
{
	return fov;
}