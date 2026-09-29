/*
	Wiktor Kopec
	Last Modified 04/26/04
*/


#include "model.h"
#include <stdio.h>
#include <fstream>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <gl/gl.h>
#include "texturizer.h"
#include "renderableobject.h"
#include "mvector3.h"
#include <float.h>


using namespace std;

RenderableObject * Model::Clone(RenderableObject & source)
{
	RenderableObject * object = new RenderableObject();
	object -> displayList = source.displayList;
	object -> setObjectId(nextId);
	object -> data.comments = source.data.comments;
	object -> data.fileName = source.data.fileName;
	object -> data.type = source.data.type;
	object -> data.lod = source.data.lod;
	object -> data.name = source.data.name;
	    
	object -> data.boundingBox = source.data.boundingBox;

	object->rotate = source.rotate;
	object->trans = source.trans;
	object->scale = source.scale;

	//If the orginal is not a copy, then the object gets the ID of the source
	// else it will get the same parent ID as the source.
	if ( source.copy == false )
	{
		object->copy = true;
		object->parentId = source.getObjectId();
	}
	else
	{
		object->copy = true;
		object->parentId = source.parentId;
	}


	Texturizer::Copy(source.getObjectId());
	
	objects[nextId] = object;
	nextId++;
	return object;
}

void Model::UpdateDisjointSetForObject(int id )
{
	if (objects[id - Object::getIDOffset() - 1] ->isCopy())
	{
		return;
	}
	
	int newParent = promoteChildwithParentID(id - Object::getIDOffset() - 1);
	
	if ( newParent == -1)
	{
		return;
	}

	updateAllChildrenWithID(id - Object::getIDOffset() - 1, newParent);
}

int Model::promoteChildwithParentID(int pID)
{
	for ( int i = 0; i < objects.getSize(); i++ )
	{
		if (objects[i])
		{
			if ( objects[i]->getParentId() == pID )
			{
				objects[i]->setParentId(-1);
				objects[i]->makeCopy(true);
				return objects[i]->getObjectId();
			}
		}
	}

	return -1;
}

void Model::updateAllChildrenWithID(int pID, int newID)
{
	for ( int i = 0; i < objects.getSize(); i++ )
	{
		if ( objects[i] )
		{
			if ( objects[i]->getParentId() == pID )
			{
				objects[i]->setParentId(newID);
				objects[i]->makeCopy(true);
			}
		}
	}
}

void Model::ApplyForces()
{
//	Physics::SetTimeStep(Time::getInterval());
//	for ( int i = 0; i < objects.getSize(); i++ )
//	{
//		if ( objects[i] )
//		{
//			Physics::ApplyForces(*objects[i]);
//		}
//	}
}

Model::Model() : in(NULL), nextId(0), objects(DEFAULTOBJECTCOUNT, NULL) {}

Object & Model::operator [](int id) const
{
	return *objects[id - Object::getIDOffset() - 1];
}

Model::~Model()
{
	if (in)
	{
		fclose(in);
	}
	
	for (int i = 0; i < objects.getSize(); i++)
	{
		if (objects[i])
		{
			delete objects[i];
		}
	}
}

void Model::CloseScene()
{
	nextId = 0;
	objects = Vector<Object *>(DEFAULTOBJECTCOUNT, NULL);
	if (in)
	{
		fclose(in);
		in = NULL;
	}
	Texturizer::Release();
}

void Model::SaveScene(const String & sceneFile)
{
	ofstream out(sceneFile.getCString());
	
	out << "SS\n";
	out << 0.0f << " " << 0.0f << " " << 0.0f << " " << 0.0f << " " << 0.0f << " " << 1.0f << endl;
	out << nextId << endl;
 	for (int i = 0; i < nextId; i++)
	{
		RenderableObject * dobject = dynamic_cast<RenderableObject *>(objects[i]);

		if (dobject)
		{			
			out << dobject -> getObjectId() << endl;			
			//added pid export
			out << dobject -> getParentId() << endl;
			//added copy boolean export
			out << dobject -> copy << endl;
			out << dobject -> getData().fileName.getCString() << endl;
			Matrix rotate = (dobject -> rotate);
			float scale[3];
			dobject -> getScale(scale[0], scale[1], scale[2]);
			float translate[3];
			dobject -> getTranslate(translate[0], translate[1], translate[2]);
			
			out << rotate.Values()[0] << " " << rotate.Values()[1] << " " << rotate.Values()[2] << " " << rotate.Values()[3] << endl
			<< rotate.Values()[4] << " " << rotate.Values()[5] << " " << rotate.Values()[6] << " " << rotate.Values()[7] << endl
			<< rotate.Values()[8] << " " << rotate.Values()[9] << " " << rotate.Values()[10] << " " << rotate.Values()[11] << endl
			<< rotate.Values()[12] << " " << rotate.Values()[13] << " " << rotate.Values()[14] << " " << rotate.Values()[15] << endl;

			out << scale[0] << " " << scale[1] << " " << scale[2] << endl;
			out << translate[0] << " " << translate[1] << " " << translate[2] << endl;
		}
		else
		{
			out << -1 << endl;
		}
	}	
	out.flush();
	out.close();
}

void Model::LoadScene(const String & sceneFile)
{	
	ifstream in(sceneFile.getCString());

	if (in)
	{
		char magic[3] = {'\0', '\0', '\0'};
		float posview[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
		float rst[22] = {0.0f, 0.0f, 0.0f, 0.0f,
						 0.0f, 0.0f, 0.0f, 0.0f,	//Rot matrix
						 0.0f, 0.0f, 0.0f, 0.0f,
						 0.0f, 0.0f, 0.0f, 0.0f,
						 1.0f, 1.0f, 1.0f,			//Scale values
						 0.0f, 0.0f, 0.0f};			//Trans values
		int totalObjects(0);
		int id(0);
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));		

		in >> magic;

		if ( (magic[0] == 'S') && (magic[1] == 'S') )
		{			
			
			for (int i = 0; i < 6; i++)
			{
				in >> posview[i];
			}

			in >> totalObjects;

			for (int i = 0; i < nextId; i++)
			{
				if (objects[i])
				{
					delete objects[i];
				}
			}

			Texturizer::Release();
			objects = Vector<Object *>(totalObjects * 2, NULL);
			nextId = 0;
			int pid(0);
			bool copy(false);

			for (int i = 0; i < totalObjects; i++)
			{
				in >> id >> pid >> copy;
				
				if (id < 0)
				{
					objects[i] = NULL;
				}
				else if (id != i)
				{
					throw "Inconsistent indexing";
				}
				else
				{					
					in.get();
					in.getline(buffer, sizeof(buffer), '\n');
					
					if (!copy)
					{
						Open(buffer);
					}
					else
					{
						RenderableObject * parent = dynamic_cast<RenderableObject *>(objects[pid]);
						Clone(*parent);
					}
										
					for (int j = 0; j < 22; j++)
					{
						in >> rst[j];
					}
					Matrix rot(	rst[0], rst[1], rst[2], rst[3], 
								rst[4], rst[5], rst[6], rst[7],
								rst[8], rst[9], rst[10], rst[11],
								rst[12], rst[13], rst[14], rst[15]);
					Matrix scale(	rst[16], 0, 0, 0,
									0, rst[17], 0, 0,
									0, 0, rst[18], 0,
									0, 0, 0,		1 );
					Matrix translate(	1,0,0, rst[19],
										0,1,0, rst[20],
										0,0,1, rst[21],
										0,0,0, 1);

					dynamic_cast<RenderableObject *>(objects[id]) -> rotate = rot;
					dynamic_cast<RenderableObject *>(objects[id]) -> scale = scale;
					dynamic_cast<RenderableObject *>(objects[id]) -> trans = translate;
				}			
			}
		}
		else
		{
			in.close();
			//not magic
			/*Do something here*/
		}
	}
	else
	{
		/*invalid file*/
		/*Do something here*/
	}
	in.close();
}

Object * Model::Open(const String & object, const String & ext, const String & path)
{
	in = fopen((((path == "") ? path : path + "\\") + object + "." + ext).getCString(), "rb");
	
	if (in)
	{
		if (ext == "3dbin")
		{
			if (nextId >= objects.getSize())
			{
				//TODO fix initialization
				objects.Resize(objects.getSize() * 2);
			}

			ParseObject(in, object);
			Texturizer::Add(in);
			reinterpret_cast<RenderableObject *>(objects[nextId - 1]) -> Compile();
			return objects[nextId - 1];
		}
	}
	return NULL;
}

void Model::ParseObject(FILE * in, const String & fname)
{
	RenderableObject::ObjectData data;
	memset(&data, 0, sizeof(data));
	
	data.fileName = fname;

	int size(0);
	int magic[2] = {0, 0};
	char name[32];
	char comments[128];
	int lodCount;
	int objectType; 

	if (in)
	{		
		fread(magic, sizeof(magic), 1, in);

		if ( (magic[0] == 4) && (magic[1] == 2) )
		{
			fread(name, sizeof(name), 1, in);
			fread(comments, sizeof(comments), 1, in);
			fread(&lodCount, sizeof(int), 1, in);
			fread(&objectType, sizeof(int), 1, in);

			data.name = name;
			data.comments = comments;
			data.lod = lodCount;
			data.type = static_cast<Object::ObjectType>(objectType);
		}
		else
		{
			throw "Invalid file type";
		}		
	}
	else
	{
		throw "Could not parse file";
	}
		
	for (int j = 0; j < lodCount; j++)
	{
		fread(&size, sizeof(int), 1, in);	
	
		data.meshes[j] = Vector<RenderableObject::MeshData>(size);
		
		MVector3 minPoint(FLT_MAX, FLT_MAX, FLT_MAX);
		MVector3 maxPoint(FLT_MIN, FLT_MIN, FLT_MIN);

		for (int i = 0; i < data.meshes[j].getSize(); i++)
		{
			fread(&size, sizeof(int), 1, in);
					
			data.meshes[j][i].vertices = Vector<RenderableObject::Vertex>(size * 3);
			data.meshes[j][i].normals = Vector<RenderableObject::Vertex>(size * 3);
			data.meshes[j][i].uvs = Vector<RenderableObject::TextureData>(size * 3);
			data.meshes[j][i].colors = Vector<RenderableObject::Color>(size * 3);

			for (int k = 0; k < size * 3; k++)
			{
#ifdef _DEBUG
				RenderableObject::Vertex v;
				RenderableObject::Color c;
				RenderableObject::TextureData t;
				RenderableObject::Vertex n;

				fread(&v, sizeof(v), 1, in);
				fread(&c, sizeof(c), 1, in);
				fread(&t, sizeof(t), 1, in);
				fread(&n, sizeof(n), 1, in);

				data.meshes[j][i].vertices[k] = v;
				data.meshes[j][i].colors[k] = c;
				data.meshes[j][i].uvs[k] = t;
				data.meshes[j][i].normals[k] = n;

#else
				fread(&data.meshes[j][i].vertices[k], sizeof(RenderableObject::Vertex), 1, in);
				fread(&data.meshes[j][i].colors[k], sizeof(RenderableObject::Color), 1, in);
				fread(&data.meshes[j][i].uvs[k], sizeof(RenderableObject::TextureData), 1, in);
				fread(&data.meshes[j][i].normals[k], sizeof(RenderableObject::Vertex), 1, in);
#endif

				minPoint.x = ((data.meshes[j][i].vertices[k].x < minPoint.x) ? data.meshes[j][i].vertices[k].x : minPoint.x);
				minPoint.y = ((data.meshes[j][i].vertices[k].y < minPoint.y) ? data.meshes[j][i].vertices[k].y : minPoint.y);
				minPoint.z = ((data.meshes[j][i].vertices[k].z < minPoint.z) ? data.meshes[j][i].vertices[k].z : minPoint.z);

				maxPoint.x = ((data.meshes[j][i].vertices[k].x > maxPoint.x) ? data.meshes[j][i].vertices[k].x : maxPoint.x);
				maxPoint.y = ((data.meshes[j][i].vertices[k].y > maxPoint.y) ? data.meshes[j][i].vertices[k].y : maxPoint.y);
				maxPoint.z = ((data.meshes[j][i].vertices[k].z > maxPoint.z) ? data.meshes[j][i].vertices[k].z : maxPoint.z);
			}		
		}
		data.boundingBox = BoundingBox(minPoint, maxPoint);
	}
	
	RenderableObject * object = new RenderableObject();		

	object -> setObjectId(nextId);
	object -> setData(data);
	
	objects[nextId++] = object;
}

void Model::Close(int id)
{	
	UpdateDisjointSetForObject(id);	
	delete objects[id - Object::getIDOffset() - 1];
	objects[id - Object::getIDOffset() - 1] = NULL;
}

void Model::Draw()
{

	for (int i = 0; i < nextId; i++)
	{
		if (objects[i])
		{
			RenderableObject * dobject = dynamic_cast<RenderableObject *>(objects[i]);
			if (dobject)
			{
				dobject -> Draw();
			}
		}
	}

	glFlush();
}

bool Model::CollisionTest()
{
	//Bounding box test for collisions
	bool result = false;
	for (int i = 0; i < objects.getSize() - 1; i++)
	{
		for ( int j = i + 1; j < objects.getSize(); j++)
		{
			if ( objects[i] && objects[j] )
			{
				if ( ((RenderableObject*)objects[i])->ObjectIntersectBox( (RenderableObject*)objects[j] ) )
				{
					result = true;
				}
			}
		}
	}
	return result;

}

int Model::getCount() const
{
	return objects.getSize();
}
