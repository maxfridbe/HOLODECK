/**
	Maksim Fridberg, 
	Last modified 4/27/04
**/

/************************************************************************
	MODEL MANIPULATOR
	@desc = This appears when the menuing system invokes it
	it is the class that draws a Rotate tool, Scale tool, and
	Translate tool.  This allowes the user to manipulate objects
	on the screen in 3 space analog form instead of numarical digital
	form.

************************************************************************/



#include "modelManip.h"
#include "settings.h"
#include "renderableobject.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <gl/gl.h>
#include <gl/glu.h>
#include "renderableobject.h"
#include "wtime.h"
#include "matrix.h"
#include "wmath.h"

#define NUM_LINES 40  //used to define complexity of the Rotate Manipulator
double ModelManipulator::distance = 1;


void ModelManipulator::DoTranslate()
{
	RenderableObject * data = (RenderableObject *)ModelManipulator::currentObject[0];

	//OpenGL Stores the matrix transposed!
	data->Translate().Transpose();
	data->Scale().Transpose();

	glMultMatrixf(data->Translate().Values());
	data->Translate().Transpose();
}

void ModelManipulator::DoRotate()
{
	RenderableObject * data = (RenderableObject *)ModelManipulator::currentObject[0];

	Matrix unTrans = data->Translate();
	unTrans[3] = - data->Translate()[3];
	unTrans[7] = - data->Translate()[7];
	unTrans[11] = - data->Translate()[11];

	//OpenGL Stores the matrix transposed!
	unTrans.Transpose();
	data->Translate().Transpose();
	data->Rotate().Transpose();

	glMultMatrixf(data->Translate().Values());
	glMultMatrixf(data->Rotate().Values());

	data->Translate().Transpose();
	data->Rotate().Transpose();
}


/******************************************************************
				STATIC Variable Init
*******************************************************************/

float ModelManipulator::offsetx = 5.0f;
float ModelManipulator::offsety = 5.0f;
float ModelManipulator::offsetz = 5.0f;
float ModelManipulator::base = 2.5f;
float ModelManipulator::rad = 0.25f;
float ModelManipulator::x = 0;
float ModelManipulator::y = 0;
float ModelManipulator::z = 0;

List<Object *> ModelManipulator::currentObject;

MVector3 * ModelManipulator::PlayerPosition = NULL;

//THIS IS THE OPENGL LIST OF OBJECT NUMBERS FOR 
//								   TRANSLATE, SCALE, ROTATE
int ModelManipulator::xObject[3] = { 1 ,		4,		7};
int ModelManipulator::yObject[3] = { 2 ,		5,		8};
int ModelManipulator::zObject[3] = { 3 ,		6,		9};

/*********************************************************************
			STATIC STATES INIT
**********************************************************************/
ModelManipulator::ManipulatorStates ModelManipulator::currentState = ModelManipulator::OFF;
ModelManipulator::ManipulatorTypes ModelManipulator::currentType = ModelManipulator::INVISIBLE;
ModelManipulator::ManipulatorTypes ModelManipulator::oldForm = ModelManipulator::INVISIBLE;


/*constructor*/
ModelManipulator::ModelManipulator() {}


void ModelManipulator::SetPlayer(MVector3 & pos)
{
	ModelManipulator::PlayerPosition = &pos;
}


void ModelManipulator::ClearObjects()
{
	currentObject = List<Object *>();
}



void ModelManipulator::SetState(ManipulatorTypes type ) 
{	
	ModelManipulator::currentType = type; 
}


ModelManipulator::ManipulatorTypes ModelManipulator::GetState()
{ 
	return ModelManipulator::currentType;
}


void ModelManipulator::DrawManipulator(bool enable)
{

	GLint wireframe[2];
	glGetIntegerv(GL_POLYGON_MODE, wireframe);
	if ( wireframe[1] == GL_LINE )
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	static float value = 0.0f;

	if ( currentObject.getSize() <= 0 )
	{
		glPolygonMode(GL_FRONT_AND_BACK, wireframe[1]);
		return;
	}

	for (List<Object *>::Iterator it = currentObject.Begin(); !it.isNull(); it++)
	{
		if (*it == NULL)
		{
			break;
		}
		//x = (*it)->pos.x;
		//y = (*it)->pos.y;
		//z = (*it)->pos.z;
		RenderableObject::ObjectData &data = (dynamic_cast<RenderableObject*>(*it))->getData();

		MVector3 high(data.boundingBox.getMax());
		MVector3 low(data.boundingBox.getMin());
		x = (high.x + low.x) / 2;
		y = (high.y + low.y) / 2;
		z = (high.z + low.z) / 2;
		offsetx = base + high.x - x;
		offsety = base + high.y - y;
		offsetz = base + high.z - z;
	}

	if (enable)
		CheckMouse();

	switch (currentType)
	{
	case ModelManipulator::TRANSLATE:
		{		
			oldForm = TRANSLATE;
			if ( currentState == ModelManipulator::OFF )
			{
				currentState = ModelManipulator::IDLE;
			}	
			else 
			{
				RenderableObject * dobject(NULL);
				for (List<Object *>::Iterator it = currentObject.Begin(); !it.isNull(); it++)
				{
					dobject = dynamic_cast<RenderableObject *>((*it));
				}

				float posx, posy, posz;
				dobject->getTranslate(posx, posy, posz);

				DrawPyramidX(MVector3(x + offsetx,	y,				z			));
				DrawPyramidY(MVector3(x ,			y + offsety,	z			));
				DrawPyramidZ(MVector3(x ,			y ,				z + offsetz	));

			}
			break;
		}
	case ModelManipulator::SCALE:
		{
			oldForm = SCALE;
			if ( currentState == ModelManipulator::OFF )
			{
				currentState = ModelManipulator::IDLE;
			}	
			else 
			{
				DrawCubeX(MVector3(x + offsetx,		y,				z			));
				DrawCubeY(MVector3(x,				y + offsety,	z			));
				DrawCubeZ(MVector3(x,				y,				z + offsetz	));

				RenderableObject * dobject;
				for (List<Object *>::Iterator it = currentObject.Begin(); !it.isNull(); it++)
				{
					dobject = dynamic_cast<RenderableObject *>((*it));
				}

			}
			break;
		}
	case ModelManipulator::ROTATE:
		{			
			oldForm = ROTATE;
			if ( currentState == ModelManipulator::OFF )
			{
				currentState = ModelManipulator::IDLE;
			}
			else
			{
				ModelManipulator::DrawCircleX(((offsetx > offsety)?((offsetx > offsetz)?offsetx:offsetz):((offsety > offsetz)?offsety:offsetz)));
				ModelManipulator::DrawCircleY(((offsetx > offsety)?((offsetx > offsetz)?offsetx:offsetz):((offsety > offsetz)?offsety:offsetz)));
				ModelManipulator::DrawCircleZ(((offsetx > offsety)?((offsetx > offsetz)?offsetx:offsetz):((offsety > offsetz)?offsety:offsetz)));
			}
		}
		break;
	case ModelManipulator::INVISIBLE:
		{
			if ( currentState == ModelManipulator::IDLE )
			{
				currentState = ModelManipulator::OFF;
			}
		}
		break;
	default:
		break;
	}
	
	if ( wireframe[1] == GL_LINE )
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	return;
}


bool ModelManipulator::ManipActive()
{
	if(currentState == DRAGGINGX || currentState == DRAGGINGY || currentState == DRAGGINGZ) 
		return true;
	else 
		return false;
}


void ModelManipulator::AddObject(Object & object, bool init)
{
	if (init)
	{
		currentObject = List<Object *>();
	}
	currentObject.PushBack(&object);
}


void ModelManipulator::CheckMouse()
{
	float offsetX(0.0f);
	float offsetY(0.0f);
	float offsetZ(0.0f);
	
	if (currentState == ModelManipulator::IDLE)
	{
		Settings::ScreenLock = false;
		if ( UserInterface::OnMouseClick())
		{	
			if ( (Settings::mouseOnObject == xObject[0]) || (Settings::mouseOnObject == xObject[1]) || (Settings::mouseOnObject == xObject[2]))
			{
				currentState = DRAGGINGX;
			} 
			else if ((Settings::mouseOnObject == yObject[0]) || (Settings::mouseOnObject == yObject[1]) || (Settings::mouseOnObject == yObject[2]))
			{
				currentState = DRAGGINGY;
			} 
			else if ((Settings::mouseOnObject == zObject[0]) || (Settings::mouseOnObject == zObject[1]) || (Settings::mouseOnObject == zObject[2]))
			{
				currentState = DRAGGINGZ;
			}
		}
	}

	if ( currentState == ModelManipulator::DRAGGINGX )
	{
		Settings::ScreenLock = true;
		if ( UserInterface::OnMouseRelease(0) )
		{		
			currentState = ModelManipulator::IDLE;
			return;
		}

		float movementX = (float)(UserInterface::getMouseRelX());
		float movementY = (float)(UserInterface::getMouseRelY());
		
		if(PlayerPosition->z < z)
		{
			movementX = -movementX;
		}

		MVector3 vectorNew = MVector3(PlayerPosition->x + movementX,PlayerPosition->y + movementY, PlayerPosition->z) - MVector3(x,y,z);

		float angleOld = MVector3::AngleBetweenVectors(*PlayerPosition - MVector3(x,y,z),MVector3(1,0,0));
		
		float angleNew = MVector3::AngleBetweenVectors(MVector3(1,0,0) , vectorNew );
		
		movementX = abs(movementX);
		movementY = abs(movementY);
		
		if ( (angleNew < angleOld))
		{
			offsetX = (movementX + movementY)  / 2.0f;
		}
		else if ( angleNew > angleOld)
		{
			offsetX = -(movementX + movementY)  / 2.0f;		
		}
	}
	else if ( currentState == ModelManipulator::DRAGGINGY )
	{
		Settings::ScreenLock = true;
		if ( UserInterface::OnMouseRelease(0) )
		{		
			currentState = ModelManipulator::IDLE;
			return;
		}

//		float movementX = (float)(UserInterface::getMouseRelX());
		float movementY = (float)(UserInterface::getMouseRelY());
		
		offsetY = -movementY;		

	}

	else if ( currentState == ModelManipulator::DRAGGINGZ )
	{
		Settings::ScreenLock = true;
		if ( UserInterface::OnMouseRelease(0) )
		{		
			currentState = ModelManipulator::IDLE;
			return;
		}

		float movementX = (float)(UserInterface::getMouseRelX());
		float movementY = (float)(UserInterface::getMouseRelY());
		

		if(PlayerPosition->x < x)
		{
			movementX = -movementX;
		}

		MVector3 vectorNew = MVector3(PlayerPosition->x, PlayerPosition->y + movementY, PlayerPosition->z + movementX) - MVector3(x,y,z);
		
		float angleOld = MVector3::AngleBetweenVectors(*PlayerPosition - MVector3(x,y,z), MVector3(0,0,1));
		
		float angleNew = MVector3::AngleBetweenVectors( MVector3(0,0,1), vectorNew );
		
		movementX = abs(movementX);
		movementY = abs(movementY);
		
		if ( (angleNew < angleOld))
		{
			offsetZ = -movementX;
		}
		else if ( angleNew > angleOld)
		{
			offsetZ = movementX;
		}
		else
		{
			return;
		}
	}
	
	UpdateObject(offsetX, offsetY, offsetZ);
}


void ModelManipulator::UpdateObject(float offsetX, float offsetY, float offsetZ)
{
	for (List<Object *>::Iterator it = currentObject.Begin(); !it.isNull(); it++)
	{
		RenderableObject * dobject = dynamic_cast<RenderableObject *>((*it));
		if (ModelManipulator::currentType == ModelManipulator::TRANSLATE)
		{
			dobject->TranslateBy(	offsetX * (float)Time::getInterval() * 10.0f,
									offsetY * (float)Time::getInterval() * 10.0f,
									offsetZ * (float)Time::getInterval() * 10.0f);								
		}
		else if (ModelManipulator::currentType == ModelManipulator::SCALE)
		{
			dobject->ScaleBy(	offsetX * (float)Time::getInterval() * 10.0f,
								offsetY * (float)Time::getInterval() * 10.0f,
								offsetZ * (float)Time::getInterval() * 10.0f);
								
		}
		else if (ModelManipulator::currentType == ModelManipulator::ROTATE)
		{
			dobject->RotateXBy(offsetX * (float)Time::getInterval() * 10.0f);
			dobject->RotateYBy(offsetY * (float)Time::getInterval() * 10.0f);
			dobject->RotateZBy(offsetZ * (float)Time::getInterval() * 10.0f);
		}
	}	
}



/*****************************************************************
					Draw Manipulator Functions
******************************************************************/

void ModelManipulator::DrawCircleX(float Rad)
{
	float originalzCoord = z;
	float originalyCoord = y;
	float zCoord = 0.0f;
	float yCoord = 0.0f;
	
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
		glLoadName(xObject[2]);

		DoTranslate();
		double d = ModelManipulator::distance / 30.0;
		glScaled(d, d, d);
		glDepthFunc(GL_ALWAYS);
		glBegin(GL_QUADS);
		{
			static float oldzCoord = 0.0f;
			static float oldyCoord = 0.0f;

			for(int i=0;i<NUM_LINES;i++)
			{
				zCoord = (float)Rad * cos((float)i * 2.0f * (float)PI / (float)NUM_LINES);
				yCoord = (float)Rad * sin((float)i * 2.0f * (float)PI / (float)NUM_LINES);	
				
				glLoadIdentity();

				glColor3ub(255,30,30);
				glVertex3f(x, oldyCoord + originalyCoord, oldzCoord + originalzCoord);
				glVertex3f(x + rad, oldyCoord + originalyCoord, oldzCoord + originalzCoord);
				glVertex3f(x + rad, yCoord + originalyCoord, zCoord + originalzCoord);
				glVertex3f(x, yCoord + originalyCoord, zCoord + originalzCoord);

				oldzCoord = zCoord;
				oldyCoord = yCoord;
					
				
			}
		}
		glEnd();
		glDepthFunc(GL_LEQUAL);

	glPopMatrix();
}


void ModelManipulator::DrawCircleY(float Rad)
{
	float originalxCoord = x;
	float originalzCoord = z;
	float xCoord = 0.0f;
	float zCoord = 0.0f;
	

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();

		DoTranslate();
		double d = ModelManipulator::distance / 30.0;
		glScaled(d, d, d);
		glLoadName(yObject[2]);
		glDepthFunc(GL_ALWAYS);
		glBegin(GL_QUADS);
		{
			static float oldxCoord = 0.0f;
			static float oldzCoord = 0.0f;



			for(int i=0;i<NUM_LINES;i++)
			{
				xCoord = (float)Rad * (float)cos((float)i * 2.0f * (float)PI / (float)NUM_LINES);
				zCoord = (float)Rad * (float)sin((float)i * 2.0f * (float)PI / (float)NUM_LINES);	
				
				glLoadIdentity();

				glColor3ub(30,255,30);
				glVertex3f(oldxCoord + originalxCoord,y, oldzCoord + originalzCoord);
				glVertex3f(oldxCoord + originalxCoord, y + rad, oldzCoord + originalzCoord);
				glVertex3f(xCoord + originalxCoord, y + rad, zCoord + originalzCoord);
				glVertex3f(xCoord + originalxCoord, y,zCoord + originalzCoord );

				oldxCoord = xCoord;
				oldzCoord = zCoord;
					
				
			}
		}
		glEnd();
		glDepthFunc(GL_LEQUAL);
	glPopMatrix();
}


void ModelManipulator::DrawCircleZ(float Rad)
{
	float originalxCoord = x;
	float originalyCoord = y;
	float xCoord = 0.0f;
	float yCoord = 0.0f;
	
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	
		DoTranslate();
		double d = ModelManipulator::distance / 30.0;
		glScaled(d, d, d);
		glLoadName(zObject[2]);
		glDepthFunc(GL_ALWAYS);
		glBegin(GL_QUADS);
		{
			static float oldxCoord = 0.0f;
			static float oldyCoord = 0.0f;



			for(int i=0;i<NUM_LINES;i++)
			{
				xCoord = (float)Rad * (float)cos((float)i * 2.0f * (float)PI / (float)NUM_LINES);
				yCoord = (float)Rad * (float)sin((float)i * 2.0f * (float)PI / (float)NUM_LINES);	
				
				glLoadIdentity();

				glColor3ub(30,30,255);
				glVertex3f(oldxCoord + originalxCoord, oldyCoord + originalyCoord, z);
				glVertex3f(oldxCoord + originalxCoord, oldyCoord + originalyCoord, z + rad);
				glVertex3f(xCoord + originalxCoord, yCoord + originalyCoord, z + rad);
				glVertex3f(xCoord + originalxCoord, yCoord + originalyCoord, z);

				oldxCoord = xCoord;
				oldyCoord = yCoord;
					
				
			}
		}
		glEnd();
		glDepthFunc(GL_LEQUAL);
	glPopMatrix();
}


void ModelManipulator::DrawCubeZ(const MVector3 & loc)
{
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	
		DoRotate();
		double d = ModelManipulator::distance / 30.0;
		glScaled(d, d, d);
		glLoadName(zObject[1]);

		glBegin(GL_LINES);
		{
			glLineWidth(20);
			glColor3ub(255,255,255);
			glVertex3f(x,y,z);
			glVertex3f(loc.x,loc.y, loc.z);
		}
		glEnd();

		glDepthFunc(GL_ALWAYS);

		glBegin(GL_QUADS);
		{
			glColor3ub(0,0,255);
			glVertex3f(loc.x + rad, loc.y + rad, loc.z);
			glVertex3f(loc.x - rad, loc.y + rad, loc.z);
			glVertex3f(loc.x - rad, loc.y - rad, loc.z);
			glVertex3f(loc.x + rad, loc.y - rad, loc.z);
			
			glColor3ub(0,0,255);
			glVertex3f(loc.x + rad, loc.y + rad, loc.z);
			glVertex3f(loc.x - rad, loc.y + rad, loc.z);
			glColor3ub(0,0,125);
			glVertex3f(loc.x - rad, loc.y + rad, loc.z - base);
			glVertex3f(loc.x + rad, loc.y + rad, loc.z - base);

			glColor3ub(0,0,255);
			glVertex3f(loc.x - rad, loc.y + rad, loc.z);
			glVertex3f(loc.x - rad, loc.y - rad, loc.z);
			glColor3ub(0,0,125);
			glVertex3f(loc.x - rad, loc.y - rad, loc.z - base);
			glVertex3f(loc.x - rad, loc.y + rad, loc.z - base);

			glColor3ub(0,0,255);
			glVertex3f(loc.x - rad, loc.y - rad, loc.z);
			glVertex3f(loc.x + rad, loc.y - rad, loc.z);
			glColor3ub(0,0,125);
			glVertex3f(loc.x + rad, loc.y - rad, loc.z - base);
			glVertex3f(loc.x - rad, loc.y - rad, loc.z - base);

			glColor3ub(0,0,255);
			glVertex3f(loc.x + rad, loc.y - rad, loc.z);
			glVertex3f(loc.x + rad, loc.y + rad, loc.z);
			glColor3ub(0,0,125);
			glVertex3f(loc.x + rad, loc.y + rad, loc.z - base);
			glVertex3f(loc.x + rad, loc.y - rad, loc.z - base);

			glColor3ub(0,0,125);
			glVertex3f(loc.x + rad, loc.y + rad, loc.z - base);
			glVertex3f(loc.x - rad, loc.y + rad, loc.z - base);
			glVertex3f(loc.x - rad, loc.y - rad, loc.z - base);
			glVertex3f(loc.x + rad, loc.y - rad, loc.z - base);
		}
		glEnd();

		glColor4f(1.0,1.0,1.0,1.0);	
		float posx, posy, posz;
		RenderableObject * dobject(NULL);
		for (List<Object *>::Iterator it = currentObject.Begin(); !it.isNull(); it++)
		{
			dobject = dynamic_cast<RenderableObject *>((*it));
		}
		dobject->getTranslate(posx, posy, posz);
		UserInterface::PrintGL(loc.x,loc.y, loc.z, String::ToString(posz));

		glDepthFunc(GL_LEQUAL);	
	glPopMatrix();
}


void ModelManipulator::DrawCubeY(const MVector3 & loc)
{
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	
		DoRotate();
		double d = ModelManipulator::distance / 30.0;
		glScaled(d, d, d);
		glLoadName(yObject[1]);
		
		glBegin(GL_LINES);
		{
			glLineWidth(20);
			glColor3ub(255,255,255);
			glVertex3f(x,y,z);
			glVertex3f(loc.x,loc.y, loc.z);
		}
		glEnd();

		glDepthFunc(GL_ALWAYS);

		glBegin(GL_QUADS);
		{
			glColor3ub(0,255,0);
			glVertex3f(loc.x + rad, loc.y, loc.z + rad);
			glVertex3f(loc.x + rad, loc.y, loc.z - rad);
			glVertex3f(loc.x - rad, loc.y, loc.z - rad);
			glVertex3f(loc.x - rad, loc.y, loc.z + rad);

			glColor3ub(0,255,0);
			glVertex3f(loc.x + rad, loc.y, loc.z + rad);
			glVertex3f(loc.x + rad, loc.y, loc.z - rad);
			glColor3ub(0,125,0);
			glVertex3f(loc.x + rad, loc.y - base, loc.z - rad);
			glVertex3f(loc.x + rad, loc.y - base, loc.z + rad);


			glColor3ub(0,255,0);
			glVertex3f(loc.x + rad, loc.y, loc.z - rad);
			glVertex3f(loc.x - rad, loc.y, loc.z - rad);
			glColor3ub(0,125,0);
			glVertex3f(loc.x - rad, loc.y - base, loc.z - rad);
			glVertex3f(loc.x + rad, loc.y - base, loc.z - rad);

			glColor3ub(0,255,0);
			glVertex3f(loc.x - rad, loc.y, loc.z - rad);
			glVertex3f(loc.x - rad, loc.y, loc.z + rad);
			glColor3ub(0,125,0);
			glVertex3f(loc.x - rad, loc.y - base, loc.z + rad);
			glVertex3f(loc.x - rad, loc.y - base, loc.z - rad);

			glColor3ub(0,255,0);
			glVertex3f(loc.x - rad, loc.y, loc.z + rad);
			glVertex3f(loc.x + rad, loc.y, loc.z + rad);
			glColor3ub(0,125,0);
			glVertex3f(loc.x + rad, loc.y - base, loc.z + rad);
			glVertex3f(loc.x - rad, loc.y - base, loc.z + rad);

			glColor3ub(0,125,0);
			glVertex3f(loc.x + rad, loc.y - base, loc.z + rad);
			glVertex3f(loc.x + rad, loc.y - base, loc.z - rad);
			glVertex3f(loc.x - rad, loc.y - base, loc.z - rad);
			glVertex3f(loc.x - rad, loc.y - base, loc.z + rad);

		}
		glEnd();

		glColor4f(1.0,1.0,1.0,1.0);	
		float posx, posy, posz;
		RenderableObject * dobject(NULL);
		for (List<Object *>::Iterator it = currentObject.Begin(); !it.isNull(); it++)
		{
			dobject = dynamic_cast<RenderableObject *>((*it));
		}
		dobject->getScale(posx, posy, posz);
		UserInterface::PrintGL(loc.x,loc.y, loc.z, String::ToString(posy));

		glDepthFunc(GL_LEQUAL);
	glPopMatrix();
}


void ModelManipulator::DrawCubeX(const MVector3 & loc)
{
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
		DoRotate();
		double d = ModelManipulator::distance / 30.0;
		glScaled(d, d, d);
		glLoadName(xObject[1]);
		
		glBegin(GL_LINES);
		{
			glLineWidth(20);
			glColor3ub(255,255,255);
			glVertex3f(x,y,z);
			glVertex3f(loc.x,loc.y, loc.z);
		}
		glEnd();

		glDepthFunc(GL_ALWAYS);

		glBegin(GL_QUADS);
		{
			glColor3ub(255,0,0);
			glVertex3f(loc.x, loc.y + rad, loc.z + rad);
			glVertex3f(loc.x, loc.y + rad, loc.z - rad);
			glVertex3f(loc.x, loc.y - rad, loc.z - rad);
			glVertex3f(loc.x, loc.y - rad, loc.z + rad);

			glColor3ub(255,0,0);
			glVertex3f(loc.x, loc.y + rad, loc.z + rad);
			glVertex3f(loc.x, loc.y + rad, loc.z - rad);
			glColor3ub(125,0,0);
			glVertex3f(loc.x - base, loc.y + rad, loc.z - rad);
			glVertex3f(loc.x - base, loc.y + rad, loc.z + rad);

			glColor3ub(255,0,0);
			glVertex3f(loc.x, loc.y + rad, loc.z - rad);
			glVertex3f(loc.x, loc.y - rad, loc.z - rad);
			glColor3ub(125,0,0);
			glVertex3f(loc.x - base, loc.y - rad, loc.z - rad);
			glVertex3f(loc.x - base, loc.y + rad, loc.z - rad);

			glColor3ub(255,0,0);
			glVertex3f(loc.x, loc.y - rad, loc.z - rad);
			glVertex3f(loc.x, loc.y - rad, loc.z + rad);
			glColor3ub(125,0,0);
			glVertex3f(loc.x - base, loc.y - rad, loc.z + rad);
			glVertex3f(loc.x - base, loc.y - rad, loc.z - rad);

			glColor3ub(255,0,0);
			glVertex3f(loc.x, loc.y - rad, loc.z + rad);
			glVertex3f(loc.x, loc.y + rad, loc.z + rad);
			glColor3ub(125,0,0);
			glVertex3f(loc.x - base, loc.y + rad, loc.z + rad);
			glVertex3f(loc.x - base, loc.y - rad, loc.z + rad);

			glColor3ub(125,0,0);
			glVertex3f(loc.x - base, loc.y + rad, loc.z + rad);
			glVertex3f(loc.x - base, loc.y + rad, loc.z - rad);
			glVertex3f(loc.x - base, loc.y - rad, loc.z - rad);
			glVertex3f(loc.x - base, loc.y - rad, loc.z + rad);


		}
		glEnd();

		glColor4f(1.0,1.0,1.0,1.0);	
		float posx, posy, posz;
		RenderableObject * dobject(NULL);
		for (List<Object *>::Iterator it = currentObject.Begin(); !it.isNull(); it++)
		{
			dobject = dynamic_cast<RenderableObject *>((*it));
		}
		dobject->getScale(posx, posy, posz);
		UserInterface::PrintGL(loc.x,loc.y, loc.z, String::ToString(posx));

		glDepthFunc(GL_LEQUAL);
	glPopMatrix();
}


void ModelManipulator::DrawPyramidZ(const MVector3 & loc)
{	
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
		
		DoTranslate();
		double d = ModelManipulator::distance / 30.0;
		glScaled(d, d, d);
		glLoadName(zObject[0]);
		
		


		glBegin(GL_LINES);
		{
			glLineWidth(20);
			glColor3ub(255,255,255);
			glVertex3f(x,y,z);
			glVertex3f(loc.x,loc.y, loc.z);
		}
		glEnd();
		
		glDepthFunc(GL_ALWAYS);

		glBegin(GL_QUADS);
		{
			glColor3ub(0,0,125);
			
			glVertex3f(loc.x + rad, loc.y + rad, loc.z - base);
			glVertex3f(loc.x + rad, loc.y - rad, loc.z - base);
			glVertex3f(loc.x - rad, loc.y - rad, loc.z - base);
			glVertex3f(loc.x - rad, loc.y + rad, loc.z - base);

		}
		glEnd();

		glBegin(GL_TRIANGLES);
		{
			glColor3ub(0,0,255);
			glVertex3f(loc.x, loc.y, loc.z);
			glColor3ub(0,0,125);
			glVertex3f(loc.x + rad, loc.y + rad, loc.z - base);
			glVertex3f(loc.x - rad, loc.y + rad, loc.z - base);

			glColor3ub(0,0,255);
			glVertex3f(loc.x, loc.y, loc.z);
			glColor3ub(0,0,125);
			glVertex3f(loc.x - rad, loc.y + rad, loc.z - base);
			glVertex3f(loc.x - rad, loc.y - rad, loc.z - base);

			glColor3ub(0,0,255);
			glVertex3f(loc.x, loc.y, loc.z);
			glColor3ub(0,0,125);
			glVertex3f(loc.x - rad, loc.y - rad, loc.z - base);
			glVertex3f(loc.x + rad, loc.y - rad, loc.z - base);

			glColor3ub(0,0,255);
			glVertex3f(loc.x, loc.y, loc.z);
			glColor3ub(0,0,125);
			glVertex3f(loc.x + rad, loc.y - rad, loc.z - base);
			glVertex3f(loc.x + rad, loc.y + rad, loc.z - base);

		}
		glEnd();

		glColor4f(1.0,1.0,1.0,1.0);	
		float posx, posy, posz;
		RenderableObject * dobject(NULL);
		for (List<Object *>::Iterator it = currentObject.Begin(); !it.isNull(); it++)
		{
			dobject = dynamic_cast<RenderableObject *>((*it));
		}
		dobject->getTranslate(posx, posy, posz);
		UserInterface::PrintGL(loc.x,loc.y, loc.z, String::ToString(posz));

		glDepthFunc(GL_LEQUAL);
	glPopMatrix();

}


void ModelManipulator::DrawPyramidY(const MVector3 & loc)
{		
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
		
		DoTranslate();
		double d = ModelManipulator::distance / 30.0;
		glScaled(d, d, d);
		glLoadName(yObject[0]);
		


		glBegin(GL_LINES);
		{
			glLineWidth(20);
			glColor3ub(255,255,255);
			glVertex3f(x,y,z);
			glVertex3f(loc.x,loc.y, loc.z);
		}
		glEnd();
		
		glDepthFunc(GL_ALWAYS);
		
		glBegin(GL_QUADS);
		{
			glColor3ub(0,125,0);

			glVertex3f(loc.x + rad, loc.y - base, loc.z + rad);
			glVertex3f(loc.x + rad, loc.y - base, loc.z - rad);
			glVertex3f(loc.x - rad, loc.y - base, loc.z - rad);
			glVertex3f(loc.x - rad, loc.y - base, loc.z + rad);

		}
		glEnd();

		glBegin(GL_TRIANGLES);
		{
			glColor3ub(0,255,0);
			glVertex3f(loc.x, loc.y, loc.z);
			glColor3ub(0,125,0);
			glVertex3f(loc.x + rad, loc.y - base, loc.z + rad);
			glVertex3f(loc.x + rad, loc.y - base, loc.z - rad);

			glColor3ub(0,255,0);
			glVertex3f(loc.x, loc.y, loc.z);
			glColor3ub(0,125,0);
			glVertex3f(loc.x + rad, loc.y - base, loc.z - rad);
			glVertex3f(loc.x - rad, loc.y - base, loc.z - rad);

			glColor3ub(0,255,0);
			glVertex3f(loc.x, loc.y, loc.z);
			glColor3ub(0,125,0);
			glVertex3f(loc.x - rad, loc.y - base, loc.z - rad);
			glVertex3f(loc.x - rad, loc.y - base, loc.z + rad);

			glColor3ub(0,255,0);
			glVertex3f(loc.x, loc.y, loc.z);
			glColor3ub(0,125,0);
			glVertex3f(loc.x - rad, loc.y - base, loc.z + rad);
			glVertex3f(loc.x + rad, loc.y - base, loc.z + rad);

		}
		glEnd();
		
		glColor4f(1.0,1.0,1.0,1.0);	
		float posx, posy, posz;
		RenderableObject * dobject(NULL);
		for (List<Object *>::Iterator it = currentObject.Begin(); !it.isNull(); it++)
		{
			dobject = dynamic_cast<RenderableObject *>((*it));
		}
		dobject->getTranslate(posx, posy, posz);
		UserInterface::PrintGL(loc.x,loc.y, loc.z, String::ToString(posy));

		glDepthFunc(GL_LEQUAL);
	glPopMatrix();

}


void ModelManipulator::DrawPyramidX(const MVector3 & loc)
{		
	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	
		
		DoTranslate();
		double d = ModelManipulator::distance / 30.0;
		glScaled(d, d, d);
		glLoadName(xObject[0]);
		

		
		glBegin(GL_LINES);
		{
			glLineWidth(20);
			glColor3ub(255,255,255);
			glVertex3f(x,y,z);
			glVertex3f(loc.x,loc.y, loc.z);
		}
		glEnd();


		glDepthFunc(GL_ALWAYS);

		glBegin(GL_QUADS);
		{
			glColor3ub(125,0,0);
			glVertex3f(loc.x - base, loc.y + rad, loc.z + rad);
			glVertex3f(loc.x - base, loc.y + rad, loc.z - rad);
			glVertex3f(loc.x - base, loc.y - rad, loc.z - rad);
			glVertex3f(loc.x - base, loc.y - rad, loc.z + rad);
		}
		glEnd();

		glBegin(GL_TRIANGLES);
		{
			glColor3ub(255,0,0);
			glVertex3f(loc.x, loc.y, loc.z);
			glColor3ub(125,0,0);
			glVertex3f(loc.x - base, loc.y + rad, loc.z + rad);
			glVertex3f(loc.x - base, loc.y + rad, loc.z - rad);

			glColor3ub(255,0,0);
			glVertex3f(loc.x, loc.y, loc.z);
			glColor3ub(125,0,0);
			glVertex3f(loc.x - base, loc.y + rad, loc.z - rad);
			glVertex3f(loc.x - base, loc.y - rad, loc.z - rad);

			glColor3ub(255,0,0);
			glVertex3f(loc.x, loc.y, loc.z);
			glColor3ub(125,0,0);
			glVertex3f(loc.x - base, loc.y - rad, loc.z - rad);
			glVertex3f(loc.x - base, loc.y - rad, loc.z + rad);

			glColor3ub(255,0,0);
			glVertex3f(loc.x, loc.y, loc.z);
			glColor3ub(125,0,0);
			glVertex3f(loc.x - base, loc.y - rad, loc.z + rad);
			glVertex3f(loc.x - base, loc.y + rad, loc.z + rad);

		}
		glEnd();
		
		glColor4f(1.0,1.0,1.0,1.0);	
		float posx, posy, posz;
		RenderableObject * dobject(NULL);
		for (List<Object *>::Iterator it = currentObject.Begin(); !it.isNull(); it++)
		{
			dobject = dynamic_cast<RenderableObject *>((*it));
		}
		dobject->getTranslate(posx, posy, posz);
		UserInterface::PrintGL(loc.x,loc.y, loc.z, String::ToString(posx));

		glDepthFunc(GL_LEQUAL);
	glPopMatrix();
}

void ModelManipulator::setDistance(Camera * camera)
{
	//Manhatten distance to camera from manipulator
	double d = 0;
	d += abs(camera->Pos().x - ModelManipulator::x);
	d += abs(camera->Pos().y - ModelManipulator::y);
	d += abs(camera->Pos().z - ModelManipulator::z);

	distance = d;
}

double ModelManipulator::getDistance(){ return distance; }