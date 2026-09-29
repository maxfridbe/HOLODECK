/*
	Wiktor Kopec
	Last Modified 04/26/04
*/

#include "displayableobject.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include "wtime.h"
#include "texturizer.h"
#include "settings.h"
#include "wmath.h"
#include "mvector3.h"

#pragma comment(lib, "opengl32.lib")

DisplayableObject::DisplayableObject() : Object() 
{
	this->copy = false;
	this->parentId = -1;
	memset(&data, 0, sizeof(data));
}

DisplayableObject::DisplayableObject(const MVector3 & pos, const MVector3 & view) : Object(pos, view) 
{
	this->copy = false;
	this->parentId = -1;
	memset(&data, 0, sizeof(data));
}

DisplayableObject::DisplayableObject(float PosX, float PosY, float PosZ, float viewX, float viewY, float viewZ) : Object(PosX, PosY, PosZ, viewX, viewY, viewZ) 
{
	this->copy = false;
	this->parentId = -1;
	memset(&data, 0, sizeof(data));
}

DisplayableObject::~DisplayableObject() {}

void DisplayableObject::Compile()
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
					a;b;r;g;u;v;x;y;z;
#endif

					int bind = data.meshes[0][i].uvs[j].binding;

					if (bind > 0)
					{
						if (Texturizer::getBind(objectId - Object::IDOffset, bind) != Texturizer::CurrentBinding)
						{
							Texturizer::CurrentBinding = Texturizer::Bind(objectId - Object::IDOffset, bind);							
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

void DisplayableObject::DrawByMatrix()
{
	glPushMatrix();
		glLoadName(objectId + 1);	
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

		if ( Settings::mouseOnObject - 1 == objectId )
		{		
			glColor3ub(255, 255, 0);
		}
		else
		{
			glColor3ub(255, 255, 255);
		}
		glCallList(this -> displayList);

	glPopMatrix();
}


void DisplayableObject::Draw()
{
//	this->RotatePure();
//	this->RotateGlobalYRelXZ();


	this->DrawByMatrix();
}

DisplayableObject::ObjectData & DisplayableObject::getData()
{
	return data;
}

void DisplayableObject::setData(const DisplayableObject::ObjectData & newData)
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

	memcpy(data.boundingBox, newData.boundingBox, sizeof(data.boundingBox));
}

//void DisplayableObject::setScale(float * scaling)
//{	
//	if ( abs(scaling[0]) < 0.01f )
//		scaling[0] = 0.01f;
//	
//	if ( abs(scaling[1]) < 0.01f )
//		scaling[1] = 0.01f;
//
//	if ( abs(scaling[2]) < 0.01f )
//		scaling[2] = 0.01f;
//
//	data.scale[0] = scaling[0] / data.scale[0];
//	data.scale[1] = scaling[1] / data.scale[1];
//	data.scale[2] = scaling[2] / data.scale[2];
//
//	data.boundingBox[0] -= data.translate[0];
//	data.boundingBox[1] -= data.translate[1];
//	data.boundingBox[2] -= data.translate[2];
//	data.boundingBox[3] -= data.translate[0];
//	data.boundingBox[4] -= data.translate[1];
//	data.boundingBox[5] -= data.translate[2];
//
//	data.boundingBox[0] *= data.scale[0];
//	data.boundingBox[1] *= data.scale[1];
//	data.boundingBox[2] *= data.scale[2];
//	data.boundingBox[3] *= data.scale[0];
//	data.boundingBox[4] *= data.scale[1];
//	data.boundingBox[5] *= data.scale[2];
//
//	for (int i = 0; i < data.meshes[0].getSize(); i++)
//	{
//		for (int j = 0; j < data.meshes[0][i].vertices.getSize(); j++)		
//		{
//			data.meshes[0][i].vertices[j].x -= data.translate[0];
//			data.meshes[0][i].vertices[j].y -= data.translate[1];
//			data.meshes[0][i].vertices[j].z -= data.translate[2];
//
//			data.meshes[0][i].vertices[j].x *= data.scale[0];
//			data.meshes[0][i].vertices[j].y *= data.scale[1];
//			data.meshes[0][i].vertices[j].z *= data.scale[2];			
//
//			data.meshes[0][i].vertices[j].x += data.translate[0];
//			data.meshes[0][i].vertices[j].y += data.translate[1];
//			data.meshes[0][i].vertices[j].z += data.translate[2];
//		}
//	}
//
//	data.boundingBox[0] += data.translate[0];
//	data.boundingBox[1] += data.translate[1];
//	data.boundingBox[2] += data.translate[2];
//	data.boundingBox[3] += data.translate[0];
//	data.boundingBox[4] += data.translate[1];
//	data.boundingBox[5] += data.translate[2];
//
//	memcpy(data.scale, scaling, sizeof(data.scale));
//}
//
//void DisplayableObject::setRotate(float * rotate)
//{	
//	
	
	/*convert radians to degrees*/

	/*	for (int i = 0; i < 3; i++)
	{
		while ( abs(rotate[i]) >= 360.0f)
		{
			rotate[i] = rotate[i] + ((rotate[i] < 0.0f) ? 360.0f : -360.0f);
		}
		while (abs(data.rotate[i] >= 360.0f))
		{
			data.rotate[i] = data.rotate[i] + ((data.rotate[i] < 0.0f) ? 360.0f : -360.0f);
		}
		
		rotate[i] = WMath::DegreeToRadian(rotate[i]);
		data.rotate[i] = WMath::DegreeToRadian(data.rotate[i]);
	}
*/
	
	/*setup the cos/sin arrays*/
/*
	float oldC[3] = {0.0f, 0.0f, 0.0f};
	float oldS[3] = {0.0f, 0.0f, 0.0f};
	float c[3] = {0.0f, 0.0f, 0.0f};
	float s[3] = {0.0f, 0.0f, 0.0f};

	for (int i = 0; i < 3; i++)
	{
		oldC[i] = cos(data.rotate[i]);
		oldS[i] = -sin(data.rotate[i]);
		c[i] = cos(rotate[i]);
		s[i] = sin(rotate[i]);
	}
*/

	/*untranslate the boundingbox*/
	//for (int i = 0, j = 0; i < 6; i++, j = (j + 1) % 3)
	//{
	//	data.boundingBox[i] -= data.translate[j];
	//}
	//
	///*unrotate*/
	//WMath::RotatePrecomputedZYX(data.boundingBox, oldC, oldS, data.boundingBox);
	//WMath::RotatePrecomputedZYX(&data.boundingBox[3], oldC, oldS, &data.boundingBox[3]);
	//
	///*rotate*/
	//WMath::RotatePrecomputedXYZ(data.boundingBox, c, s, data.boundingBox);	
	//WMath::RotatePrecomputedXYZ(&data.boundingBox[3], c, s, &data.boundingBox[3]);

	///*Retranslate*/
	//for (int i = 0, j = 0; i < 6; i++, j = (j + 1) % 3)
	//{
	//	data.boundingBox[i] += data.translate[j];
	//}
	//
	///*foreach vertex*/

	//for (int i = 0; i < data.meshes[0].getSize(); i++)
	//{
	//	for (int j = 0; j < data.meshes[0][i].vertices.getSize(); j++)
	//	{
	//		/*untranslate vertex*/
	//		data.meshes[0][i].vertices[j].x -= data.translate[0];
	//		data.meshes[0][i].vertices[j].y -= data.translate[1];
	//		data.meshes[0][i].vertices[j].z -= data.translate[2];
	//		
	//		/*unrotate*/
	//		WMath::RotatePrecomputedZYX(data.meshes[0][i].vertices[j].x, data.meshes[0][i].vertices[j].y, data.meshes[0][i].vertices[j].z, oldC, oldS, data.meshes[0][i].vertices[j].x, data.meshes[0][i].vertices[j].y, data.meshes[0][i].vertices[j].z);
	//		
	//		/*rotate*/
	//		WMath::RotatePrecomputedXYZ(data.meshes[0][i].vertices[j].x, data.meshes[0][i].vertices[j].y, data.meshes[0][i].vertices[j].z, c, s, data.meshes[0][i].vertices[j].x, data.meshes[0][i].vertices[j].y, data.meshes[0][i].vertices[j].z);

	//		/*retranslate vertex*/
	//		data.meshes[0][i].vertices[j].x += data.translate[0];
	//		data.meshes[0][i].vertices[j].y += data.translate[1];
	//		data.meshes[0][i].vertices[j].z += data.translate[2];
	//	}
	//}
	//
	//for (int i = 0; i < 3; i++)
	//{
	//	rotate[i] = WMath::RadianToDegree(rotate[i]);
//	//	data.rotate[i] = WMath::RadianToDegree(data.rotate[i]);
//	//}
//
//	memcpy(data.rotate, rotate, sizeof(data.rotate));
//}
//
//void DisplayableObject::setTranslate(float * translate)
//{
	/*data.translate[0] = translate[0] - data.translate[0];
	data.translate[1] = translate[1] - data.translate[1];
	data.translate[2] = translate[2] - data.translate[2];


	for (int i = 0; i < data.meshes[0].getSize(); i++)
	{
		for (int j = 0; j < data.meshes[0][i].vertices.getSize(); j++)		
		{			
			data.meshes[0][i].vertices[j].x += data.translate[0];
			data.meshes[0][i].vertices[j].y += data.translate[1];
			data.meshes[0][i].vertices[j].z += data.translate[2];
		}
	}

	data.boundingBox[0] += data.translate[0];
	data.boundingBox[1] += data.translate[1];
	data.boundingBox[2] += data.translate[2];
	data.boundingBox[3] += data.translate[0];
	data.boundingBox[4] += data.translate[1];
	data.boundingBox[5] += data.translate[2];*/

//	pos.x = translate[0];
//	pos.y = translate[1];
//	pos.z = translate[2];
//	memcpy(data.translate, translate, sizeof(data.translate));
//
//}

unsigned int DisplayableObject::getList() const
{
	return displayList;
}
//
//void DisplayableObject::RotatePure()
//{
//	glPushMatrix();
//	{
//		glLoadName(objectId + 1);		
//		glTranslatef(trans.Values()[3], trans.Values()[7], trans.Values()[11]);
//
//		glPushMatrix();			
//		{		
//			MPoint3d PositiveX(1.0,0.0,0.0);
//			MPoint3d PositiveY(0.0,1.0,0.0);
//			MPoint3d PositiveZ(0.0,0.0,1.0);
//
//			Matrix A, B, C, ModelView;
//
//			glPushMatrix();
//			{					
//				glLoadIdentity();
//		
//				glRotatef(data.rotate[1], 0.0, 1.0, 0.0);	
//				glGetFloatv(GL_MODELVIEW_MATRIX, A.Values());
//
//				MPoint3d zAxis = PositiveZ;
//				A.Transpose(NULL);
//				A.VectorMult(PositiveZ, zAxis);
//
//
//				MPoint3d newXAxis = PositiveX;
//				A.VectorMult(PositiveX, newXAxis);
//				
//
//				//This works, trying to use the multiplication of point.
//				//MVector3 newXAxis = MVector3::XProduct(MVector3(0,1,0), point);
//
//				glLoadIdentity();
//				glRotatef(data.rotate[0], 1,0,0);	
//				glGetFloatv(GL_MODELVIEW_MATRIX, B.Values());
//
//				B.Transpose();
//				MPoint3d newZAxis = zAxis;
//				B.VectorMult(zAxis, newZAxis);
//
//				glLoadIdentity();
//				glRotatef(data.rotate[2], 0,0,1);
//				glGetFloatv(GL_MODELVIEW_MATRIX, C.Values());
//
//				/*
//				glLoadIdentity();
//				MPoint3d newYAxis = MVector3::XProduct(newXAxis, newZAxis);
//				glRotatef(data.rotate[1], newYAxis.x, newYAxis.y, newYAxis.z);
//				glGetFloatv(GL_MODELVIEW_MATRIX, A.Values());
//				*/
//
//				B.Transpose();
//				A.Transpose();
//
//				glLoadIdentity();
//				glMultMatrixf(C.Values());
//				glMultMatrixf(B.Values());
//				glMultMatrixf(A.Values());				
//				
//
//				glGetFloatv(GL_MODELVIEW_MATRIX, ModelView.Values());
//			}
//			glPopMatrix();
//
//			glMultMatrixf(ModelView.Values());
//
//			glScalef(data.scale[0], data.scale[1], data.scale[2]);
//
//			if ( Settings::mouseOnObject - 1 == objectId )
//			{
//				glColor3ub(255, 255, 0);
//			}
//			else
//			{
//				glColor3ub(255, 255, 255);
//			}
//			glCallList(this -> displayList);
//		}
//		glPopMatrix();
//	}
//	glPopMatrix();
//}

//void DisplayableObject::RotateGlobalYRelXZ()
//{
//	glPushMatrix();
//	{
//		glLoadName(objectId + 1);		
//		glTranslatef(data.translate[0], data.translate[1], data.translate[2]);
//
//		glPushMatrix();			
//		{		
//			MPoint3d PositiveX(1.0,0.0,0.0);
//			MPoint3d PositiveY(0.0,1.0,0.0);
//			MPoint3d PositiveZ(0.0,0.0,1.0);
//
//			Matrix A, B, C, ModelView;
//
//			glPushMatrix();
//			{					
//				glLoadIdentity();
//
//				static MPoint3d yAxis(0,1,0);
//
//				glRotatef(data.rotate[1], yAxis.x, yAxis.y, yAxis.z);	
//				glGetFloatv(GL_MODELVIEW_MATRIX, A.Values());
//
//				MPoint3d zAxis = PositiveZ;
//				A.Transpose(NULL);
//				A.VectorMult(PositiveZ, zAxis);
//
//
//				MPoint3d newXAxis = PositiveX;
//				A.VectorMult(PositiveX, newXAxis);
//				
//
//				//This works, trying to use the multiplication of point.
//				//MVector3 newXAxis = MVector3::XProduct(MVector3(0,1,0), point);
//
//				glLoadIdentity();
//				glRotatef(data.rotate[0], newXAxis.x, newXAxis.y, newXAxis.z);	
//				glGetFloatv(GL_MODELVIEW_MATRIX, B.Values());
//
//				B.Transpose();
//				MPoint3d newZAxis = zAxis;
//				B.VectorMult(zAxis, newZAxis);
//
//				glLoadIdentity();
//				glRotatef(data.rotate[2], newZAxis.x, newZAxis.y, newZAxis.z);
//				glGetFloatv(GL_MODELVIEW_MATRIX, C.Values());
//
//
//				/*
//				glLoadIdentity();
//				MPoint3d newYAxis = MVector3::XProduct(newXAxis, newZAxis);
//				glRotatef(data.rotate[1], newYAxis.x, newYAxis.y, newYAxis.z);
//				glGetFloatv(GL_MODELVIEW_MATRIX, A.Values());
//				*/
//
//				B.Transpose();
//				A.Transpose();
//
//				glLoadIdentity();
//				glMultMatrixf(C.Values());
//				glMultMatrixf(B.Values());
//				glMultMatrixf(A.Values());				
//				
//
//				glGetFloatv(GL_MODELVIEW_MATRIX, ModelView.Values());
//			}
//			glPopMatrix();
//
//			glMultMatrixf(ModelView.Values());
//
//			glScalef(data.scale[0], data.scale[1], data.scale[2]);
//
//			if ( Settings::mouseOnObject - 1 == objectId )
//			{
//				glColor3ub(255, 255, 0);
//			}
//			else
//			{
//				glColor3ub(255, 255, 255);
//			}
//			glCallList(this -> displayList);
//		}
//		glPopMatrix();
//	}
//	glPopMatrix();
//}

