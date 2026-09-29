/*
	Wiktor Kopec
	Last Modified 04/26/04
*/

#ifndef TEXTURIZER_H
#define TEXTURIZER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "whashtable.h"
#include <gl/gl.h>
#include <stdio.h>

//<summary>This class is used to load and bind all texture and color information</summary>
//<remarks>Calls to texturizer are typically done right after calling a Open call in the 
//model class.  Typically, binding is done by the model during the draw phase, and not by
//the user.  This class cannot be instantiated</remarks>
class Texturizer
{
	public:
		
		//<summary>Adds a texture to the texture collection</summary>
		//<remarks>Add specifically takes a FILE * pointer instead of a name, because
		//it typically gets called automatically by the model when loading on object.
		//The 3dbin format specification includes texture information, which makes passing in
		//the file pointer more convenient.</remarks>
		//<param name='fin'>An opened FILE pointer which contains the texture data</param>
		//<returns>For future use.  Always returns 0</returns>
		static int Add(FILE * fin);
				
		//<summary>Binds a specified texture object</summary>
		//<remarks>Texture objects are are created on a per object basis.  In order to figure out
		//which texture to bind to, the texturizer needs the object number as well as localBinding,
		//which is given in the 3dbin.  Binding is typically done by the model instead of the user.</remarks>
		//<param name='objectNumber'>The object number which provided the texture.  This does not represent the
		//object id from glLoadName but rather an index.  It can be obtained from the objectId by subtracting
		//(Object::IDOffset + 1)</param>
		//<param name='localTextureBinding'>This is the per object texture binding.  It is specified in the 3dbin
		//file</param>
		//<returns>Returns the name of the newly bound texture object</returns>
		static int Bind(int objectNumber, int localTextureBinding);
		
		//<summary>Gets the texture object number from the object Number and local binding</summary>
		//<param name='objectNumber'>The object number which provided the texture.  This does not represent the
		//object id from glLoadName but rather an index.  It can be obtained from the objectId by subtracting
		//(Object::IDOffset + 1)</param>
		//<param name='localTextureBinding'>This is the per object texture binding.  It is specified in the 3dbin
		//file</param>
		//<returns>Returns the name of the texture object</returns>
		static int getBind(int objectNumber, int localTextureBinding);
			
		//<summary>Stores the currently bound texture object name</summary>
		static int CurrentBinding;
		
		//<summary>Releases all the resources held by the class.  All textures are cleared</summary>
		static void Release();

		static void Copy(int source);

	private:
		static float defaultColor[4];
		static int globalTextureIndex;
		static Vector<Vector<int> > textureList;		
		Texturizer();
};

#endif
