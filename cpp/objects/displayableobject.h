/*
	Wiktor Kopec
	Last Modified 04/26/04
*/

#ifndef DISPLAYABLEOBJECT_H
#define DISPLAYABLEOBJECT_H

#include "object.h"
#include "wstring.h"
#include "wvector.h"
#include "matrix.h"
#include "userinterface.h"

//<summary>This class encapsulates objects that can be drawn in an OpenGL rendering context</summary>
//<remarks>Typically these will not be instantiated directly by the user, but loaded in through the
//model</remarks>
class DisplayableObject : public Object
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

				memcpy(boundingBox, source.boundingBox, sizeof(boundingBox));
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

				memcpy(boundingBox, rhs.boundingBox, sizeof(boundingBox));
			}
			//<summary>Object name.  Part of 3dbin specification</summary>
			String name;
			//<summary>Object comments.  Part of 3dbin specification</summary>
			String comments;
			//<summary>The file name from which the model was loaded</summary>
			String fileName;
			//<summary>Object type.  Part of 3dbin specification</summary>
			Object::ObjectType type;
			
			//TODO: Switch to the new matrix approach.

			//<summary>A translation matrix.  Absolute</summary>
		//	float translate[3];
		//	//<summary>A rotation matrix.  Absolute</summary>
		//	float rotate[3];
		//	//<summary>A scaling matrix.  Absolute</summary>
		//	float scale[3];
		//	//<summary>Bounding box for the object. 0-2 are the min values, 3-5 are the max values
		//	//on x y z</summary>
			
			float boundingBox[6];
			//<summary>How many levels of details the object has.  Unsupported</summary>
			int lod;
			//<summary>The meshes of the object.  Each index represents the levels of detail from lowest
			//to highest</summary>
			Vector<MeshData> meshes[3];
		};

#pragma pack(pop, id1)

		//<summary>Default Constructor</summary>
		DisplayableObject();
		
		//<summary>Creates a displayable object</summary>
		//<param name='pos'>The position of the object in three space</param>
		//<param name='view'>The orientation of the object in three space</param>
		DisplayableObject(const MVector3 &, const MVector3 &);

		//<summary>Creates a displayable object</summary>
		//<param name='PosX'>The x position of the object</param>
		//<param name='PosY'>The y position of the object</param>
		//<param name='PosZ'>The z position of the object</param>
		//<param name='viewX'>The x orientation of the object</param>
		//<param name='viewY'>The y orientation of the object</param>
		//<param name='viewZ'>The z orientation of the object</param>
		DisplayableObject(float PosX, float PosY, float PosZ, float viewX, float viewY, float viewZ);
		
		//<summary>Class destructor</summary>
		virtual ~DisplayableObject();
        
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

		unsigned int getList() const;

	private:				
		ObjectData data;
		unsigned int displayList;
};	

#endif
