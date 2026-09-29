/*
	Wiktor Kopec
	Last Modified 04/26/04
*/

#include "renderableobject.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include "wtime.h"
#include "texturizer.h"
#include "settings.h"
#include "wmath.h"
#include "mvector3.h"

#pragma comment(lib, "opengl32.lib")

void RenderableObject::Compile()
{
	this -> displayList = glGenLists(1);
	
	glNewList(this -> displayList, GL_COMPILE);

	glMatrixMode(GL_MODELVIEW);

	int count(0);
	
		glEnable(GL_TEXTURE_2D);	
		
			for (int i = 0; i < data.meshes[0].getSize(); i++)
			{						
				for (int j = 0; j < data.meshes[0][i].vertices.getSize(); j++)
				{															
#ifdef _DEBUG

					float u = data.meshes[0][i].uvs[j].u;
					float v = data.meshes[0][i].uvs[j].v;
					float x = data.meshes[0][i].vertices[j].x;
					float y = data.meshes[0][i].vertices[j].y;
					float z = data.meshes[0][i].vertices[j].z;

					float r = data.meshes[0][i].colors[j].r;
					float g = data.meshes[0][i].colors[j].g;
					float b = data.meshes[0][i].colors[j].b;
					float a = data.meshes[0][i].colors[j].a;
#endif

					int bind = data.meshes[0][i].uvs[j].binding;

					if (bind > 0)
					{
						if (Texturizer::getBind(getObjectId(), bind) != Texturizer::CurrentBinding)
						{
							Texturizer::CurrentBinding = Texturizer::Bind(getObjectId(), bind);							
						}
						
						if (count == 0)
						{
							glBegin(GL_TRIANGLES);
						}
						count++;						

						glTexCoord2f(data.meshes[0][i].uvs[j].u, data.meshes[0][i].uvs[j].v);
						glNormal3f(data.meshes[0][i].normals[j].x, data.meshes[0][i].normals[j].y, data.meshes[0][i].normals[j].z);
						glVertex3f(data.meshes[0][i].vertices[j].x, data.meshes[0][i].vertices[j].y, data.meshes[0][i].vertices[j].z);
						if (count == 3)
						{
							glEnd();
							count = 0;
						}
					}
					else
					{												
						glBindTexture(GL_TEXTURE_2D, 0);
						Texturizer::CurrentBinding = 0;

						if (count == 0)
						{
							glBegin(GL_TRIANGLES);
						}
						count++;
						glColor4f(data.meshes[0][i].colors[j].r, data.meshes[0][i].colors[j].g, data.meshes[0][i].colors[j].b, data.meshes[0][i].colors[j].a);
						glNormal3f(data.meshes[0][i].normals[j].x, data.meshes[0][i].normals[j].y, data.meshes[0][i].normals[j].z);
						glVertex3f(data.meshes[0][i].vertices[j].x, data.meshes[0][i].vertices[j].y, data.meshes[0][i].vertices[j].z);
						if (count == 3)
						{
							glEnd();
							count = 0;
						}
					}
				}		
			}		
		//glEnd();		
		glDisable(GL_TEXTURE_2D);
	

	glEndList();
}

void RenderableObject::DrawByMatrix()
{
	glPushMatrix();
	glLoadName(getObjectId() + Object::getIDOffset() + 1);	
		//OpenGL Stores the matrix transposed!
		trans.Transpose();
		rotate.Transpose();
		scale.Transpose();

		glMultMatrixf(trans.Values());
		glMultMatrixf(rotate.Values());
		glMultMatrixf(scale.Values());

		trans.Transpose();
		rotate.Transpose();
		scale.Transpose();
		

		if ( Settings::mouseOnObject - 1 == getObjectId() + Object::getIDOffset() )
		{		
			if ( dirUp )
			{
				if ( flux > upperFlux )
				{
					dirUp = false;
				}
				flux += 100 * (float)Time::getInterval();
			}
			else
			{
				if ( flux < lowerFlux )
				{
					dirUp = true;
				}
				flux -= 100 * (float)Time::getInterval();
			}
			glColor3ub(255 - flux, flux, 0);
		}
		else
		{
			glColor3ub(255, 255, 255);
		}
		glCallList(this -> displayList);

	glPopMatrix();
}


void RenderableObject::Draw()
{
	if (this -> pnode)
	{
		this -> pnode -> UpdateForces(Time::getInterval());
		this -> pnode -> UpdatePosition(Time::getInterval());
		this -> pnode -> UpdateVelocity(Time::getInterval());
		
		this -> TranslateTo((float)this ->pnode ->GetX(), (float)this ->pnode ->GetY(),(float) this ->pnode ->GetZ());
	}

	if ( this->visible )
	{
		this->DrawByMatrix();
	}
}

RenderableObject::ObjectData & RenderableObject::getData()
{
	return data;
}

void RenderableObject::setVisible(bool state)
{
	this->visible = state;
}
bool RenderableObject::IsVisible()
{
	return this->visible;
}



void RenderableObject::GenerateNewBoundingBox()
{
	//TODO: must recalculate the bounding box, but it has to be on the rotated vertices, 
	//		this means we must multiply said points by the R, S, T matrices.  and return a new box.
	//data.boundingBox[0] = ((data.meshes[j][i].vertices[k].x < data.boundingBox[0]) ? data.meshes[j][i].vertices[k].x : data.boundingBox[0]);
	//data.boundingBox[1] = ((data.meshes[j][i].vertices[k].y < data.boundingBox[1]) ? data.meshes[j][i].vertices[k].y : data.boundingBox[1]);
	//data.boundingBox[2] = ((data.meshes[j][i].vertices[k].z < data.boundingBox[2]) ? data.meshes[j][i].vertices[k].z : data.boundingBox[2]);

	//data.boundingBox[3] = ((data.meshes[j][i].vertices[k].x > data.boundingBox[3]) ? data.meshes[j][i].vertices[k].x : data.boundingBox[3]);
	//data.boundingBox[4] = ((data.meshes[j][i].vertices[k].y > data.boundingBox[4]) ? data.meshes[j][i].vertices[k].y : data.boundingBox[4]);
	//data.boundingBox[5] = ((data.meshes[j][i].vertices[k].z > data.boundingBox[5]) ? data.meshes[j][i].vertices[k].z : data.boundingBox[5]);
}


bool RenderableObject::ObjectIntersectBox(RenderableObject * object2)
{
	//TODO: add the check for time based bounding boxs, requires knowledge of object's next position.

	//TODO: Find out why Rotations go the wrong way :)

	bool colResult = true;

	
	BoundingBox ourBox(getData().boundingBox);
	BoundingBox objBox(object2->getData().boundingBox);

	ourBox.Transform(Rotate(), Scale(), Translate());
	objBox.Transform(object2 -> Rotate(), object2 -> Scale(), object2 -> Translate());

	MVector3 ourMin = MVector3(ourBox.getMin());
	MVector3 ourMax = MVector3(ourBox.getMax());
	MVector3 objMin = MVector3(objBox.getMin());
	MVector3 objMax = MVector3(objBox.getMax());

	/*
	float tempX;
	float tempY;
	float tempZ;

	if ( objMin.x > objMax.x )
	{
		tempX = objMax.x;
		objMax.x = objMin.x;
		objMin.x = tempX;
	}
	if ( objMin.y > objMax.y )
	{
		tempY = objMax.y;
		objMax.y = objMin.y;
		objMin.y = tempY;
	}
	if ( objMin.z > objMax.z )
	{
		tempZ = objMax.z;
		objMax.z = objMin.z;
		objMin.z = tempZ;
	}


	if ( ourMin.x > ourMax.x )
	{
		tempX = ourMax.x;
		ourMax.x = ourMin.x;
		ourMin.x = tempX;
	}
	if ( ourMin.y > ourMax.y )
	{
		tempY = ourMax.y;
		ourMax.y = ourMin.y;
		ourMin.y = tempY;
	}
	if ( ourMin.z > ourMax.z )
	{
		tempZ = ourMax.z;
		ourMax.z = ourMin.z;
		ourMin.z = tempZ;
	}
	*/

	// max x1  <  min of x2 and


	if ( 
			(ourMax.x < objMin.x) || (ourMin.x > objMax.x) ||
			(ourMax.y < objMin.y) || (ourMin.y > objMax.y) ||
			(ourMax.z < objMin.z) || (ourMin.z > objMax.z)   )
		//No collision possible.
	{
		colResult = false;
	}


	if ( colResult )
	{
		glColor3ub(255,0,0);
	}
	else
	{
		glColor3ub(255,255,255);
	}
	

	//Top and Bottom
	glPushMatrix();
	glViewport(0,0,Settings::WindowWidth, Settings::WindowHeight);
	glBegin(GL_LINE_STRIP);
		glVertex3f(ourMin.x, ourMin.y, ourMin.z);
		glVertex3f(ourMin.x, ourMin.y, ourMax.z);
		glVertex3f(ourMax.x, ourMin.y, ourMax.z);
		glVertex3f(ourMax.x, ourMin.y, ourMin.z);
		glVertex3f(ourMin.x, ourMin.y, ourMin.z);
		
		glVertex3f(ourMin.x, ourMax.y, ourMin.z);
		glVertex3f(ourMin.x, ourMax.y, ourMax.z);
		glVertex3f(ourMax.x, ourMax.y, ourMax.z);
		glVertex3f(ourMax.x, ourMax.y, ourMin.z);
		glVertex3f(ourMin.x, ourMax.y, ourMin.z);
	glEnd();

	//verticle lines.
	glBegin(GL_LINES);
		glVertex3f(ourMin.x, ourMin.y, ourMax.z);
		glVertex3f(ourMin.x, ourMax.y, ourMax.z);

		glVertex3f(ourMax.x, ourMin.y, ourMax.z);
		glVertex3f(ourMax.x, ourMax.y, ourMax.z);

		glVertex3f(ourMax.x, ourMin.y, ourMin.z);
		glVertex3f(ourMax.x, ourMax.y, ourMin.z);
	glEnd();

	//2nd Object for measure:
	glBegin(GL_LINE_STRIP);
		glVertex3f(objMin.x, objMin.y, objMin.z);
		glVertex3f(objMin.x, objMin.y, objMax.z);
		glVertex3f(objMax.x, objMin.y, objMax.z);
		glVertex3f(objMax.x, objMin.y, objMin.z);
		glVertex3f(objMin.x, objMin.y, objMin.z);
		
		glVertex3f(objMin.x, objMax.y, objMin.z);
		glVertex3f(objMin.x, objMax.y, objMax.z);
		glVertex3f(objMax.x, objMax.y, objMax.z);
		glVertex3f(objMax.x, objMax.y, objMin.z);
		glVertex3f(objMin.x, objMax.y, objMin.z);
	glEnd();

	//verticle lines.
	glBegin(GL_LINES);
		glVertex3f(objMin.x, objMin.y, objMax.z);
		glVertex3f(objMin.x, objMax.y, objMax.z);

		glVertex3f(objMax.x, objMin.y, objMax.z);
		glVertex3f(objMax.x, objMax.y, objMax.z);

		glVertex3f(objMax.x, objMin.y, objMin.z);
		glVertex3f(objMax.x, objMax.y, objMin.z);
	glEnd();
	glPopMatrix();
	
	return true;
}

void RenderableObject::setData(const RenderableObject::ObjectData & newData)
{
	data.comments = newData.comments;
	for (int i = 0; i < 3; i++)
	{
		data.meshes[i] = newData.meshes[i];
	}
	
	data.lod = newData.lod;
	data.name = newData.name;
	data.type = newData.type;
	data.fileName = newData.fileName;

	data.boundingBox = newData.boundingBox;
}

unsigned int RenderableObject::getList() const
{
	return displayList;
}


