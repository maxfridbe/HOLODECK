#ifndef DSCONVERT_H
#define DSCONVERT_H

#include <fstream>
#include <vector>
#include "wstring.h"
#include "mvector3.h"
//#include "whashtable.h"

using namespace std;

// Primary Chunk, Tells you this is the beginning of a 3ds.
#define PRIMARY       0x4D4D

//Main chunk types
#define OBJECTINFO    0x3D3D			// This gives the version of the mesh and is found right before the material and object information
#define VERSION       0x0002			// This gives the version of the .3ds file
#define EDITKEYFRAME  0xB000			// This is the header for all of the key frame info

// OBJECTINFO values
#define MATERIAL	  0xAFFF			// This stored the texture info
#define OBJECT		  0x4000			// This stores the faces, vertices, etc...

// MATERIAL values
#define MATNAME       0xA000			// This holds the material name
#define MATAMBIENT    0xA010			// amb light
#define MATDIFFUSE    0xA020			// This holds the color of the object/material
#define TRANS	  0xA053
#define MATMAP        0xA200			// This is a header for a new material
#define MATMAPFILE    0xA300			// This holds the file name of the texture
#define OBJECT_MESH   0x4100			// This lets us know that we are reading a new object

// OBJECT_MESH values.
#define OBJECT_VERTICES     0x4110		// The objects vertices
#define OBJECT_FACES		0x4120		// The objects faces
#define OBJECT_MATERIAL		0x4130		// This is found if the object has a material, either texture map or color
#define OBJECT_UV			0x4140		// The UV texture coordinates

class Converter3ds
{
	public:
		//Index structure
		struct Indices
		{
			unsigned short x;
			unsigned short y;
			unsigned short z;
			unsigned short visible;
		};

		//3ds is divided into chunks of data. 
		struct Chunk
		{
			unsigned short int ID;					// The chunk's ID		
			unsigned int length;					// The length of the chunk
			unsigned int bytesRead;					// The amount of bytes read within that chunk
		};
		
		struct Vertex
		{
			float x;
			float y;
			float z;
		};

		struct UV
		{
			float u;
			float v;
		};

		struct Color
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
		};

		//Use this to store wiktor's texture info...
		struct MaterialInfo
		{
			String name;			// The texture name
			String fileName;			// The texture file name (If this is set it's a texture map)
			Color color;
			//			RenderableObject::Color color;	//Color of the object.
			int   textureId;				// the texture ID
			float uTile;				// u tiling of texture  (Currently not used)
			float vTile;				// v tiling of texture	(Currently not used)
			float uOffset;			    // u offset of texture	(Currently not used)
			float vOffset;				// v offset of texture	(Currently not used)
		} ;

		struct Face
		{
			int vertIndex[3];			// indicies for the verts that make up this triangle
			int coordIndex[3];			// indicies for the tex coords to texture this face
		};

		struct Mesh 
		{
			int  numOfVerts;			// The number of verts in the model
			int  numOfFaces;			// The number of faces in the model
			int  numOfUVs;			// The number of texture coordinates
			int  materialID;			// The texture ID to use, which is the index into our texture array
			bool bHasTexture;			// This is TRUE if there is a texture map for this object
			String name;				// The name of the object
			vector < Vertex > vertices;		// The object's vertices
			vector < MVector3 > normals;		// The object's normals
			vector < UV > uvs;		// The texture's UV coordinates
			vector < Face > faces;				// The faces information of the object
		};

		// This holds our model information.  This should also turn into a robust class.
		// We use STL's (Standard Template Library) vector class to ease our link list burdens. :)
		struct Data3ds 
		{
			int numOfObjects;					// The number of objects in the model
			int numOfMaterials;					// The number of materials for the model
			vector < MaterialInfo > materials;	// The list of material information (Textures and colors)
			vector < Mesh >  meshes;			// The object list for our model
		};

		Converter3ds();
		
		Data3ds * ConvertToData3ds( String filename);

	private:

		MaterialInfo * FindString( vector < MaterialInfo > & v, String find);
		void ReadChunk(Chunk * current);
		void ProcessNextChunk( Chunk * previous);
		void ProcessNextObjectChunk( Chunk * previous);
		void ProcessNextMaterialChunk( Chunk * previous);

		void ReadColorChunk( Chunk * previous );
		void ReadColorChunkAmb( Chunk * previous );
		void ReadVertices( Chunk * previous );
		void ReadVertexIndices( Chunk * previous );
		void ReadUVs( Chunk * previous );
		void ReadObjectMaterial(Chunk * previous );
		void ComputeNormals();

		int GetString( char * place );

		int globalTextureIndex;
		int globalMeshCount;

		ofstream textureOut;
		ofstream asciiout;
		Data3ds object;
		FILE * out;
		FILE * in;

		Converter3ds(const Converter3ds & ) {}
		const Converter3ds & operator = (const Converter3ds &) {return *this;}
};


#endif
