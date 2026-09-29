#include <iostream>
#include <windows.h>
#include "3dsconvert.h"
#include "mvector3.h"

using namespace std;


Converter3ds::Converter3ds()
{
	this->out = NULL;
	this->globalMeshCount = 0;
	this->globalTextureIndex = 0;
}

void Converter3ds::ReadChunk(Chunk * chunk)
{
	// This reads the chunk ID which is 2 bytes.
	// The chunk ID is like OBJECT or MATERIAL.  It tells what data is
	// able to be read in within the chunks section.  
	chunk->bytesRead = (unsigned int)fread(&chunk->ID, 1, 2, in);

	// Then, we read the length of the chunk which is 4 bytes.
	// This is how we know how much to read in, or read past.
	chunk->bytesRead += (unsigned int)fread(&chunk->length, 1, 4, in);
}

int Converter3ds::GetString(char *pBuffer)
{
	int index = 0;

	// Read 1 byte of data which is the first letter of the string
	fread(pBuffer, 1, 1, in);

	// Loop until we get NULL
	while (*(pBuffer + index++) != 0) {

		// Read in a character at a time until we hit NULL.
		fread(pBuffer + index, 1, 1, in);
	}

	// Return the string length, which is how many bytes we read in (including the NULL)
	return (int)strlen(pBuffer) + 1;
}

Converter3ds::MaterialInfo * Converter3ds::FindString(vector < Converter3ds::MaterialInfo > & v, String find)
{
	try
	{
		for( int i = 0 ; i < (int)v.size(); i++ )
		{
			if ( v[i].fileName == find )
				return &v[i];
		}
		return NULL;
	}
	catch(...)
	{
		throw "BAd texture table";
	}
}

Converter3ds::Data3ds * Converter3ds::ConvertToData3ds( String filename)
{
	//init
	Chunk chunk = {0};

	object.numOfMaterials = 0;
	object.numOfObjects = 0;

	in = fopen(filename.getCString(), "rb");

	//Check to see if file is found;
	if ( !in )
	{
		cerr << "File not Found" << endl;
		return NULL;
	}

	ReadChunk(&chunk);

	// Check to see if this is a 3ds file.
	if ( chunk.ID != PRIMARY)
	{
		cerr << "File incorrect format" << endl;
		return NULL;
	}

	//Last function this will do the rest of the work.
	try
	{
		ProcessNextChunk( &chunk );
	}
	catch (...)
	{
		fclose(in);
		return NULL;
	}

	vector < Converter3ds::MaterialInfo > currentTextures;
	int textC = 0;
	
	for (int i = 0; i < object.numOfMaterials; i++ )
	{
		MaterialInfo * found = FindString( currentTextures, object.materials[i].fileName );
		if ( found )
		{
			object.materials[i].textureId = found->textureId;
		}
		else
		{
			object.materials[i].textureId = textC;
			currentTextures.push_back(object.materials[i]);
			textC++;
		}
	}

	try
	{
		ComputeNormals();
	}
	catch(...) 
	{
		fclose(in);
		return NULL;
	}
	fclose(in);
	return &object;
}

void Converter3ds::ProcessNextChunk( Chunk * previous )
{
	int textBuf[50000] = {0};
	
	Mesh mesh;
	mesh.numOfFaces = 0;
	mesh.numOfUVs = 0;
	mesh.numOfVerts = 0;


	Chunk current = {0};
	Chunk temp = {0};

	while (previous->bytesRead < previous->length)
	{
		ReadChunk(&current);

		switch ( current.ID )
		{
			case VERSION:
			{
				// If the file was made in 3D Studio Max, this chunk has an int that 
				// holds the file version.  Since there might be new additions to the 3DS file
				// format in 4.0, we give a warning to that problem.
				// However, if the file wasn't made by 3D Studio Max, we don't 100% what the
				// version length will be so we'll simply ignore the value

				// Read the file version and add the bytes read to our bytesRead variable
				current.bytesRead += (unsigned int)fread(textBuf, 1, current.length - current.bytesRead, in);

				// If the file version is over 3, give a warning that there could be a problem
				if ((current.length - current.bytesRead == 4) && (textBuf[0] > 0x03)) 
				{
					cerr << "Possible 3ds file version incompatibility warning." << endl;
				}
			}	//End of version chunk
			break;
	
			case OBJECTINFO:
			{
				// This chunk holds the version of the mesh.  It is also the head of the MATERIAL
				// and OBJECT chunks.  From here on we start reading in the material and object info.

				// Read the next chunk

				ReadChunk(&temp);

				// Get the version of the mesh
				temp.bytesRead += (unsigned int)fread(textBuf, 1, temp.length - temp.bytesRead, in);

				// Increase the bytesRead by the bytes read from the last chunk
				current.bytesRead += temp.bytesRead;

				// Go to the next chunk, which is the object has a texture, it should be MATERIAL, then OBJECT.
				ProcessNextChunk(&current);
				
			}	//End of Object info chunk
			break;

			case MATERIAL:							// This holds the material information
			{
				// This chunk is the header for the material info chunks
				MaterialInfo material;
				object.materials.push_back(material);
				object.numOfMaterials++;

				// Proceed to the material loading function
				ProcessNextMaterialChunk(&current);
			}	//End of material chunk.
			break;
	
			case OBJECT:		
			{

				// This chunk is the header for the mesh info chunks.  It also
				// holds the name of the mesh.

				// Resize the mesh count and add the new mesh, we will set it up in a minute.
				object.meshes.push_back(mesh);
				object.numOfObjects++;

				// Get the name of the object and store it, then add the read bytes to our byte counter.
				
				char * name = new char[256];
				current.bytesRead += GetString( name );
				object.meshes[object.meshes.size() - 1].name = String(name);
				delete [] name;

				// Now proceed to read in the rest of the object information
				ProcessNextObjectChunk(&current);				
			}
			break;
			
			case EDITKEYFRAME:
			{
				//When we get animations, update this to include it...
				//ProcessNextKeyFrameChunk(pModel, currentChunk);

				// Read past this chunk and add the bytes read to the byte counter
				current.bytesRead += (unsigned int)fread(textBuf, 1, current.length - current.bytesRead, in);
			}
			break;

			default: 
			{
				// If we didn't care about a chunk, then we get here.  We still need
				// to read past the unknown or ignored chunk and add the bytes read to the byte counter.
				current.bytesRead += (unsigned int)fread(textBuf, 1, current.length - current.bytesRead, in);
			}
			break;
		}
		// Add the bytes read from the last chunk to the previous chunk passed in.
		previous->bytesRead += current.bytesRead;
		if ( current.bytesRead == 0)
		{
			throw "Error in the whole";
			return;
		}
	}
}

void Converter3ds::ProcessNextMaterialChunk(Chunk * previous )
{
	char textBuf[50000] = {0};
	// The current chunk to work with
	Chunk current = {0};

	// Continue to read these chunks until we read the end of this sub chunk
	while (previous->bytesRead < previous->length)
	{
		ReadChunk(&current);
		switch (current.ID)
		{
			case MATNAME:							// This chunk holds the name of the material
			{
				// Here we read in the material name
				//Texturizer::TextureData texture;
				
				current.bytesRead += (unsigned int)fread(textBuf, 1, current.length - current.bytesRead, in);
				object.materials[object.materials.size() -1].name = String ( textBuf );
			}
			break;

			case MATDIFFUSE:						// This holds the R G B color of our object
			{
				//Go get it.
				ReadColorChunk(&current);	
			}
			break;

			case MATAMBIENT:
			{
				ReadColorChunkAmb(&current);
			}
			break;

			case MATMAP:							// This is the header for the texture info
			{
				// Proceed to read in the material information
				ProcessNextMaterialChunk(&current);
			}
			break;
			
			case MATMAPFILE:						// This stores the file name of the material
			{
				// Here we read in the material's file name
//				DebugBreak();
				current.bytesRead += (unsigned int)fread(textBuf, 1, current.length - current.bytesRead, in);
				object.materials[object.materials.size() - 1].fileName = String ( textBuf );
			}
			break;

			default:  
			{
				// Read past the ignored or unknown chunks
				current.bytesRead += (unsigned int)fread(textBuf, 1, current.length - current.bytesRead, in);
			}
			break;
		}

		// Add the bytes read from the last chunk to the previous chunk passed in.
		previous->bytesRead += current.bytesRead;
	}
}

void Converter3ds::ProcessNextObjectChunk( Chunk * previous )
{
	int textBuf[50000] = {0};
	// The current chunk to work with
	Chunk current = {0};

	// Continue to read these chunks until we read the end of this sub chunk
	while (previous->bytesRead < previous->length)
	{
		// Read the next chunk
		ReadChunk(&current);

		// Check which chunk we just read
		switch (current.ID)
		{
			case OBJECT_MESH:					// This lets us know that we are reading a new object
			{
				// We found a new object, so let's read in it's info using recursion
				ProcessNextObjectChunk(&current);
			}
			break;

			case OBJECT_VERTICES:				// This is the objects vertices
			{
				ReadVertices(&current);
			}
			break;

			case OBJECT_FACES:					// This is the objects face information
			{
				ReadVertexIndices(&current);
			}
			break;

			case OBJECT_MATERIAL:				// This holds the material name that the object has
			{
				// This chunk holds the name of the material that the object has assigned to it.
				// This could either be just a color or a texture map.  This chunk also holds
				// the faces that the texture is assigned to (In the case that there is multiple
				// textures assigned to one object, or it just has a texture on a part of the object.
				// Since most of my game objects just have the texture around the whole object, and 
				// they aren't multitextured, I just want the material name.

				// We now will read the name of the material assigned to this object
				ReadObjectMaterial(&current);			
				
			}
			break;
			
			case OBJECT_UV:						// This holds the UV texture coordinates for the object
			{
				// This chunk holds all of the UV coordinates for our object.  Let's read them in.
				ReadUVs(&current);
			}
			break;

			default:  
			{
				// Read past the ignored or unknown chunks
				current.bytesRead += (unsigned int)fread(textBuf, 1, current.length - current.bytesRead, in);
			}
			break;
		}

		// Add the bytes read from the last chunk to the previous chunk passed in.
		previous->bytesRead += current.bytesRead;
	}

}


void Converter3ds::ReadColorChunk(Chunk * current)
{
	Chunk temp = {0};

	// Read the color chunk info
	ReadChunk(&temp);

	// Read in the R G B color (3 bytes - 0 through 255)
	unsigned char color[3];
	temp.bytesRead += (unsigned int)fread(color, 1, temp.length - temp.bytesRead, in);
	object.materials[object.materials.size() - 1].color.r = color[0];
	object.materials[object.materials.size() - 1].color.g = color[1];
	object.materials[object.materials.size() - 1].color.b = color[2];

	// Add the bytes read to our chunk
	current->bytesRead += temp.bytesRead;
}

void Converter3ds::ReadColorChunkAmb(Chunk * current)
{
	Chunk temp = {0};

	// Read the color chunk info
	ReadChunk(&temp);

	// Read in the R G B color (3 bytes - 0 through 255)
	unsigned char color[3];
	temp.bytesRead += (unsigned int)fread(color, 1, temp.length - temp.bytesRead, in);

	//TODO: Add ambient color setting.

	// Add the bytes read to our chunk
	current->bytesRead += temp.bytesRead;
}

void Converter3ds::ReadVertices(Chunk * previous)
{
	// Like most chunks, before we read in the actual vertices, we need
	// to find out how many there are to read in.  Once we have that number
	// we then fread() them into our vertice array.

	Mesh * mesh = &object.meshes[object.meshes.size() - 1];

	// Read in the number of vertices (int)
	previous->bytesRead += (unsigned int)fread(&mesh->numOfVerts, 1, 2, in);

	// Allocate the memory for the verts and initialize the structure
//	object.meshes[object.meshes.size() - 1].vertices.resize(object.meshes[object.meshes.size() - 1].numOfVerts);
//	object.getData().meshes[object.getData().meshes->getSize() - 1][0].vertices.Resize(vertexCount);

//	Vector < RenderableObject::Vertex > * vert = &(object.getData().meshes[object.getData().meshes->getSize() - 1][0].vertices);

	// Read in the array of vertices (an array of 3 floats)

	Vertex * verts = new Vertex[mesh->numOfVerts];

	previous->bytesRead += (unsigned int)fread( verts, 1, previous->length - previous->bytesRead, in);

	for ( int i = 0; i < mesh->numOfVerts; i++ )
	{
		mesh->vertices.push_back(verts[i]);
	}

	// Now we should have all of the vertices read in.  Because 3D Studio Max
	// Models with the Z-Axis pointing up (strange and ugly I know!), we need
	// to flip the y values with the z values in our vertices.  That way it
	// will be normal, with Y pointing up.  If you prefer to work with Z pointing
	// up, then just delete this next loop.  Also, because we swap the Y and Z
	// we need to negate the Z to make it come out correctly.

	// Go through all of the vertices that we just read and swap the Y and Z values
	for( int i = 0; i < mesh->numOfVerts; i++)
	{
		// Store off the Y value
		float fTempY = mesh->vertices[i].y;

		// Set the Y value to the Z value
		mesh->vertices[i].y = mesh->vertices[i].z;

		// Set the Z value to the Y value, 
		// but negative Z because 3D Studio max does the opposite.
		mesh->vertices[i].z = -fTempY;
	}
}

void Converter3ds::ReadVertexIndices(Chunk * previous)
{
	Mesh * mesh = &object.meshes[object.meshes.size() - 1];
	unsigned short index = 0;					// This is used to read in the current face index

	// In order to read in the vertex indices for the object, we need to first
	// read in the number of them, then read them in.  Remember,
	// we only want 3 of the 4 values read in for each face.  The fourth is
	// a visibility flag for 3D Studio Max that doesn't mean anything to us.

	// Read in the number of faces that are in this object (int)
	previous->bytesRead += (unsigned int)fread(&mesh->numOfFaces, 1, 2, in);
	mesh->faces.resize( mesh->numOfFaces);

	// Go through all of the faces in this object
	for( int i = 0; i < mesh->numOfFaces; i++)
	{
		// Next, we read in the A then B then C index for the face, but ignore the 4th value.
		// The fourth value is a visibility flag for 3D Studio Max, we don't care about this.
		for(int j = 0; j < 4; j++)
		{
			// Read the first vertice index for the current face 
			previous->bytesRead += (unsigned int)fread(&index, 1, sizeof(index), in);

			if(j < 3)
			{
				// Store the index in our face structure.
				mesh->faces[i].vertIndex[j] = index;
			}
		}
	}
}

void Converter3ds::ReadObjectMaterial(Chunk * previous)
{
	int textBuf[50000] = {0};
	char name[255] = {0};			// This is used to hold the objects material name
	Mesh * mesh = &object.meshes[object.meshes.size() - 1];

	// *What is a material?*  - A material is either the color or the texture map of the object.
	// It can also hold other information like the brightness, shine, etc... Stuff we don't
	// really care about.  We just want the color, or the texture map file name really.

	// Here we read the material name that is assigned to the current object.
	// fileName should now have a string of the material name, like "Material #2" etc..
	previous->bytesRead += GetString(name);

	// Now that we have a material name, we need to go through all of the materials
	// and check the name against each material.  When we find a material in our material
	// list that matches this name we just read in, then we assign the materialID
	// of the object to that material index.  You will notice that we passed in the
	// model to this function.  This is because we need the number of textures.
	// Yes though, we could have just passed in the model and not the object too.

	// Go through all of the textures
	for( int i = 0; i < object.numOfMaterials; i++)
	{
		// If the material we just read in matches the current texture name
		if(strcmp(name, object.materials[i].name.getCString() ) == 0)
		{
			// Set the material ID to the current index 'i' and stop checking
			mesh->materialID = i;

			// Now that we found the material, check if it's a texture map.
			// If the strFile has a string length of 1 and over it's a texture
			if( object.materials[i].fileName.getCString() > 0) 
			{
				// Set the object's flag to say it has a texture map to bind.
				mesh->bHasTexture = true;
			}	
			break;
		}
		else
		{
			// Set the ID to -1 to show there is no material for this object
			mesh->materialID = -1;
		}
	}
	
	// Read past the rest of the chunk since we don't care about shared vertices
	// You will notice we subtract the bytes already read in this chunk from the total length.
	previous->bytesRead += (unsigned int)fread(textBuf, 1, previous->length - previous->bytesRead, in);
}

void Converter3ds::ReadUVs( Chunk * previous)
{
	Mesh * mesh = &object.meshes[object.meshes.size() - 1];
	// In order to read in the UV indices for the object, we need to first
	// read in the amount there are, then read them in.

	// Read in the number of UV coordinates there are (int)
	previous->bytesRead += (unsigned int)fread(&mesh->numOfUVs, 1, 2, in);
	
	// Allocate memory to hold the UV coordinates
	UV * uvs = new UV[mesh->numOfUVs];

	// Read in the texture coodinates (an array 2 float)
	previous->bytesRead += (unsigned int)fread(uvs, 1, previous->length - previous->bytesRead, in);

	//Put the UVs' into place;

	mesh->uvs.resize(mesh->numOfUVs);

	//UV's seem to bind to the mesh...
	for ( int i = 0; i < mesh->numOfUVs; i++ )
	{
		mesh->uvs[i].u = uvs[i].u;
		mesh->uvs[i].v = uvs[i].v;
	}
}

void Converter3ds::ComputeNormals()
{
	MVector3 vVector1, vVector2, vNormal, vPoly[3];

	// If there are no objects, we can skip this part
	if(object.numOfObjects <= 0 )
		return;

	// What are vertex normals?  And how are they different from other normals?
	// Well, if you find the normal to a triangle, you are finding a "Face Normal".
	// If you give OpenGL a face normal for lighting, it will make your object look
	// really flat and not very round.  If we find the normal for each vertex, it makes
	// the smooth lighting look.  This also covers up blocky looking objects and they appear
	// to have more polygons than they do.    Basically, what you do is first
	// calculate the face normals, then you take the average of all the normals around each
	// vertex.  It's just averaging.  That way you get a better approximation for that vertex.

	// Go through each of the objects to calculate their normals
	for(int index = 0; index < object.numOfObjects; index++)
	{
		// Get the current object
		Mesh * mesh = &object.meshes[index];

		// Here we allocate all the memory we need to calculate the normals
		MVector3 *pNormals		= new MVector3 [mesh->numOfFaces];
		MVector3 *pTempNormals	= new MVector3 [mesh->numOfFaces];
		mesh->normals.resize(mesh->numOfVerts);

		// Go though all of the faces of this object
		for(int i=0; i < mesh->numOfFaces; i++)
		{												
			// To cut down LARGE code, we extract the 3 points of this face
			vPoly[0] = MVector3(mesh->vertices[mesh->faces[i].vertIndex[0]].x, mesh->vertices[mesh->faces[i].vertIndex[0]].y, mesh->vertices[mesh->faces[i].vertIndex[0]].z );
			vPoly[1] = MVector3(mesh->vertices[mesh->faces[i].vertIndex[1]].x, mesh->vertices[mesh->faces[i].vertIndex[1]].y, mesh->vertices[mesh->faces[i].vertIndex[1]].z );
			vPoly[2] = MVector3(mesh->vertices[mesh->faces[i].vertIndex[2]].x, mesh->vertices[mesh->faces[i].vertIndex[2]].y, mesh->vertices[mesh->faces[i].vertIndex[2]].z );
			
			
			// Now let's calculate the face normals (Get 2 vectors and find the cross product of those 2)

            vVector1 = vPoly[0] - vPoly[2];		// Get the vector of the polygon (we just need 2 sides for the normal)
			vVector2 = vPoly[2] - vPoly[1];		// Get a second vector of the polygon

			vNormal  = MVector3::XProduct(vVector1, vVector2);		// Return the cross product of the 2 vectors (normalize vector, but not a unit vector)
			pTempNormals[i] = vNormal;					// Save the un-normalized normal for the vertex normals
			vNormal /= vNormal.GenerateMag();				// Normalize the cross product to give us the polygons normal

			pNormals[i] = vNormal;						// Assign the normal to the list of normals
		}

		//////////////// Now Get The Vertex Normals /////////////////

		MVector3 vSum = MVector3(0.0, 0.0, 0.0);
		MVector3 vZero = vSum;
		int shared=0;

		for (int i = 0; i < mesh->numOfVerts; i++)			// Go through all of the vertices
		{
			for (int j = 0; j < mesh->numOfFaces; j++)	// Go through all of the triangles
			{												// Check if the vertex is shared by another face
				if (mesh->faces[j].vertIndex[0] == i || 
					mesh->faces[j].vertIndex[1] == i || 
					mesh->faces[j].vertIndex[2] == i)
				{
					vSum += pTempNormals[j];				// Add the un-normalized normal of the shared face
					shared++;								// Increase the number of shared triangles
				}
			}      
			
			// Get the normal by dividing the sum by the shared.  We negate the shared so it has the normals pointing out.
			mesh->normals[i] = vSum / float(-shared);

			// Normalize the normal for the final vertex normal
			//mesh->normals[i] = mesh->normals[i];	

			vSum = vZero;									// Reset the sum
			shared = 0;										// Reset the shared
		}
	
		// Free our memory and start over on the next object
		delete [] pTempNormals;
		delete [] pNormals;
	}
}

