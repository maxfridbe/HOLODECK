#include "texturizer.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

#pragma comment(lib, "opengl32.lib")

float Texturizer::defaultColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
int Texturizer::globalTextureIndex = 0;
Vector<Vector<int> > Texturizer::textureList = Vector<Vector<int> >(64);
int Texturizer::CurrentBinding = 0;

//Hashtable<int, String, StringKey> Texturizer::textureNameTable;
//Hashtable<int, int> Texturizer::textureIndexTable;

int Texturizer::Add(FILE * fin)
{	
	/*
	static bool once = true;
	if (once)
	{
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		once = false;
	}

	float magic(0.0f);

	fread(&magic, sizeof(float), 1, fin);

	if (magic != 3.14f)
	{
		return -1;
	}

	for (int j = 0; j < 1; j++)
	{
		int totalLocalTextures(0);
		int width(0);
		int height(0);
		unsigned char * texels = NULL;
		fread(&totalLocalTextures, sizeof(int), 1, fin);	

		textureList[globalTextureIndex].Resize(totalLocalTextures);

		GLuint textureId(0);

		for (int i = 0; i < totalLocalTextures; i++)
		{									
			int id = 0;
			fread(&id, sizeof(int), 1, fin);
			fread(&width, sizeof(int), 1, fin);
			fread(&height, sizeof(int), 1, fin);

			texels = new unsigned char[width * height * 4];
			fread(texels, width * height * 4, 1, fin);
			
			glGenTextures(1, &textureId);
			textureList[globalTextureIndex][i] = textureId;
			glBindTexture(GL_TEXTURE_2D, textureId);
			if (glGetError())
			{
				printf("%d", glGetError());
			}

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		
	//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texels);

			if (glGetError())
			{
				printf("%d", glGetError());
			}

			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, texels);
			delete [] texels;
		}
	}
	globalTextureIndex++;
	glBindTexture(GL_TEXTURE_2D, 0);
	*/return 0;
	
}

int Texturizer::Bind(int objectNumber, int localIndex)
{
	int bind = textureList[objectNumber][localIndex - 1];
	glBindTexture(GL_TEXTURE_2D, bind);
	return bind;
}

int Texturizer::getBind(int objectNumber, int localIndex)
{
	return textureList[objectNumber][localIndex - 1];
}

void Texturizer::Color(float r, float g, float b, float a)
{
	glColor4f(r, g, b, a);
}

void Texturizer::setDefaultColor(float r, float g, float b, float a)
{
	defaultColor[0] = r;
	defaultColor[1] = g;
	defaultColor[2] = b;
	defaultColor[3] = a;
}

const float * Texturizer::getDefaultColor()
{
	return defaultColor;	
}

void Texturizer::Release()
{
	CurrentBinding = 0;
	Texturizer::globalTextureIndex = 0;
	textureList = Vector<Vector<int> >(64);
}
