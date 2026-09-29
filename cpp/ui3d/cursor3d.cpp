/***************************************************************
         Cursor 

	 @desc = This is a drawn mouse that takes over the 
	 default system mouse. It is also openGL rendered.
	 it has controls and states that can be checked to find
	 the current state of the mouse.
 *************************************************************/

#include "userinterface.h"

Cursor3d::Cursor3d()
{
	for (int i = 0 ; i < 3; i++)
	{
		this->onClick[i] = false;
		this->onRelease[i] = false;
		this->onDown[i] = false;
	}

	this->visible = true;
	x = 0;
	y = 0;
	oldx = 0;
	oldy = 0;
	this->relX = 0;
	this->relY = 0;
	type = Cursor3d::normal;
}


//Draw the mouse
void Cursor3d::Display()
{
	if ( !visible )
	{
		return;
	}

	//Start OGL Drawing.
	//Adjust your matrix.
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	{
		glLoadIdentity();
		DrawShadow(0,0,0);
		/*
			//Tracking.
			glBegin(GL_LINES);
				glVertex2i(x, y - 1000);
				glVertex2i(x, y + 1000);

				glVertex2i(x - 1000, y );
				glVertex2i(x + 1000, y );

			glEnd();
		*/
			
		//Mouse

		glColor3ub(255,255,255);

		glDepthFunc(GL_ALWAYS);
		glBegin(GL_QUADS);
			glVertex2i(x,y + 15);
			glVertex2i(x,y);
			glVertex2i(x + 2, y );
			glVertex2i(x + 2, y + 15);
			
			glVertex2i(x + 2 , y );
			glVertex2i(x + 15, y);
			glVertex2i(x + 15, y + 2);
			glVertex2i(x + 2,y + 2 );
		glEnd();

		int variance = 4;

		for (int i = 0 ; i < 3 ; i++ )
		{

			glBegin(GL_QUADS);
				glVertex2i(x + variance,y + variance + 6);
				glVertex2i(x+ variance,y+ variance);
				glVertex2i(x + 2 + variance, y+ variance );
				glVertex2i(x + 2 + variance , y + 6 + variance);
				
				glVertex2i(x + 2 + variance, y + variance);
				glVertex2i(x + 6 + variance, y + variance);
				glVertex2i(x + 6 + variance, y + 2 + variance);
				glVertex2i(x + 2 + variance,y + 2 + variance );
			glEnd();
		
			variance += 4;
		}
		glDepthFunc(GL_LEQUAL);
	}
	glPopMatrix();
}

void Cursor3d::DrawShadow(unsigned char red, unsigned char green, unsigned char blue)
{
	glColor3ub(red, green, blue);
	glBegin(GL_QUADS);
		glVertex2i(x + 1 ,y + 16);
		glVertex2i(x + 1,y + 1);
		glVertex2i(x + 3, y + 1 );
		glVertex2i(x + 3, y + 16);
		
		glVertex2i(x + 3 , y + 1 );
		glVertex2i(x + 16, y + 1);
		glVertex2i(x + 16, y + 3);
		glVertex2i(x + 3,y + 3 );
	glEnd();

	int variance = 5;

	for (int i = 0 ; i < 3 ; i++ )
	{

		glBegin(GL_QUADS);
			glVertex2i(x + variance,y + variance + 6);
			glVertex2i(x+ variance,y+ variance);
			glVertex2i(x + 2 + variance, y+ variance );
			glVertex2i(x + 2 + variance , y + 6 + variance);
			
			glVertex2i(x + 2 + variance, y + variance);
			glVertex2i(x + 6 + variance, y + variance);
			glVertex2i(x + 6 + variance, y + 2 + variance);
			glVertex2i(x + 2 + variance,y + 2 + variance );
		glEnd();
	
		variance += 4;
	}
}

void Cursor3d::PositionMouse(int x, int y)
{
	this->oldx = this->x;
	this->oldy = this->y;
	this->x = x;
	this->y = y;
}

void Cursor3d::setCursor(bool set ) { visible = set; }
int Cursor3d::getX() { 	return x; }
int Cursor3d::getY() { return y; }
int Cursor3d::getOldX() { 	return oldx; }
int Cursor3d::getOldY() { return oldy; } 
int Cursor3d::getWheel() { return wheelValue; }
void Cursor3d::setWheel(int wheelValue) { this->wheelValue = wheelValue; }
int Cursor3d::getRelX(){ return relX; }
int Cursor3d::getRelY(){ return relY; }
void Cursor3d::setRelX(int value) { relX = value; }
void Cursor3d::setRelY(int value) { relY = value; }

