/*
	Wiktor Kopec
	Last Modified 04/26/04
*/

#ifndef RENDERABLEOBJECT_H
#define RENDERABLEOBJECT_H

#include "wstring.h"
#include "wvector.h"
#include "matrix.h"
#include "userinterface.h"
#include "movableobject.h"
#include "boundingbox.h"

//<summary>This class encapsulates objects that can be drawn in an OpenGL rendering context</summary>
//<remarks>Typically these will not be instantiated directly by the user, but loaded in through the
//model</remarks>
class RenderableObject : public MovableObject
{
	friend class Model;

	public:

#pragma pack(push, id1, 1)

		//<summary>Structure that represents a vertex in 3 space</summary>
		struct Vertex
		{
			float x;
			float y;
			float z;
		};
		
		//<summary>Structure that represents texture data</summary>
		struct TextureData
		{
			int binding;
			float u;
			float v;			
		};
		
		//<summary>Structure representing color data</summary>
		struct Color
		{
			float r;
			float g;
			float b;
			float a;
		};
		
		//<summary>Structure representing mesh data</summary>
		struct MeshData
		{
			//<summary>Mesh name.  Unused</summary>
			String name;
			//<summary>Mesh id.  Unused</summary>
			int id;
			Vector<Vertex> vertices;
			Vector<Color> colors;
			Vector<TextureData> uvs;	
			Vector<Vertex> normals;			
		};

		//<summary>Structure which holds all the model data</summary>		
		struct ObjectData
		{
			ObjectData() {}
			ObjectData(const ObjectData & source)
			{
				name = source.name;
				comments = source.comments;
				fileName = source.fileName;
				type = source.type;
				lod = source.lod;
				for (int i = 0; i < 3; i++)
				{
					meshes[i] = source.meshes[i];
				}
				boundingBox = source.boundingBox;
			}

			const ObjectData & operator = (const ObjectData & rhs)
			{
				if (this == &rhs)
				{
					return *this;
				}

				name = rhs.name;
				comments = rhs.comments;
				fileName = rhs.fileName;
				type = rhs.type;
				lod = rhs.lod;

				for (int i = 0; i < 3; i++)
				{
					meshes[i] = rhs.meshes[i];
				}

				boundingBox = rhs.boundingBox;

			}
			//<summary>Object name.  Part of 3dbin specification</summary>
			String name;
			//<summary>Object comments.  Part of 3dbin specification</summary>
			String comments;
			//<summary>The file name from which the model was loaded</summary>
			String fileName;
			//<summary>Object type.  Part of 3dbin specification</summary>
			Object::ObjectType type;
			
			//<summary>Bounding box for the object. 0-2 are the min values, 3-5 are the max values
			//on x y z</summary>
			
			BoundingBox boundingBox;
			
			//<summary>How many levels of details the object has.  Unsupported</summary>
			int lod;
			//<summary>The meshes of the object.  Each index represents the levels of detail from lowest
			//to highest</summary>
			Vector<MeshData> meshes[3];
		};

#pragma pack(pop, id1)

		//<summary>Default Constructor</summary>
		RenderableObject();
		
		//<summary>Class destructor</summary>
		virtual ~RenderableObject() {}
        
		void Compile();
		
		//<summary>Draws the object</summary>
		void Draw();

		//<summary>Returns a reference to the ObjectData structure</summary>
		//<returns>ObjectData structure</returns>
		ObjectData & getData();

		//<summary>Sets the object data</summary>
		//<remarks>This is typically done by the model</remarks>
		//<param name='data'>The object data structure</remarks>
		void setData(const ObjectData & data);
	
	//	void RotateGlobalYRelXZ();
	//	void RotatePure();
		void DrawByMatrix();

		void GenerateNewBoundingBox();
		unsigned int getList() const;
		void setVisible(bool state);
		bool IsVisible();

		bool ObjectIntersectBox(RenderableObject * object2);

	private:				
		ObjectData data;
		unsigned int displayList;
		//Hidden from Wiktor:
		float flux;
		float upperFlux;
		float lowerFlux;
		bool dirUp;
		bool visible;
};	

inline RenderableObject::RenderableObject() : MovableObject()
{
	this->visible = true;
	this->upperFlux = 254;
	this->lowerFlux = 1;
	this->flux = lowerFlux;
	this->dirUp = true;
	memset(&data, 0, sizeof(data));
}

#endif
