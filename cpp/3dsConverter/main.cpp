#include <stdio.h>
#include <iostream>
#include <wstring.h>
#include <whashtable.h>
#include "3dsConvert.h"
#include <windows.h>

#pragma comment(lib, "adt.lib")
#pragma comment(lib, "objects.lib")
#pragma comment(lib, "math.lib")

using namespace std;

int textureCount = 0;

/*
"C:\Documents and Settings\wiktor\Desktop\vrupl - 9-29-04 1230 mark\Vrupl\3dsConverter\Debug\3dsConverter.exe" "-lC:/Documents and Settings/wiktor/Desktop/3DSMaxNWN_Minotaur/3DSMaxSampleCreature/c_minotaur.max" "-dC:/Documents and Settings/wiktor/Desktop/vrupl - 9-29-04 1230 mark/Vrupl/converter/data/out" "-ominotaur"
*/


bool ParseArgs(int argc, char * argv[], String & outfile, String & infile)
{
	char * path = NULL;
	char * output = NULL;
	char * lin = NULL;
	char * min = NULL;
	char * hin = NULL;
		
	for (int i = 1; i < argc; i++)
	{
		switch(argv[i][1])
		{
			case 'l':
				lin = &argv[i][2];
				break;
			case 'm':
				min = &argv[i][2];
				break;
			case 'h':
				hin = &argv[i][2];
				break;
			case 'd':
				path = &argv[i][2];
				break;
			case 'o':
				output = &argv[i][2];
				break;
			default:
				break;
		}
	}

    if ( (output == NULL) || (lin == NULL) || (path == NULL) )
	{
		return false;
	}

	infile += lin;
	outfile += path;
	outfile += "/";
	outfile += output;
	outfile += ".3dbin";
	return true;
}

void ConvertColorUBToF(unsigned char r, unsigned char g, unsigned char b, float & fr, float & fg, float & fb)
{
	fr = r / 255.0f;
	fg = g / 255.0f;
	fb = b / 255.0f;
}

void CreateTextureTable(Hashtable<int, int> & textureTable, Converter3ds::Data3ds & model)
{
	int index(1);
	
	textureTable = Hashtable<int, int>(model.numOfMaterials * 2 + 1);

//	DebugBreak();
	for (int i = 0; i < model.numOfMaterials; i++)
	{
		if (model.materials[i].fileName.getCString()!= NULL)
		{	
			textureTable[model.materials[i].textureId] = index++;
		}
	}
}

bool Write3dbin(String & outfile, Converter3ds::Data3ds & model)
{
	FILE * out = fopen(outfile.getCString(), "ab");
	FILE * asciiout = fopen((outfile + "ascii").getCString(), "w+");

	if (!out)
	{
		return false;
	}

	float one(1.0f);
	float zero(0.0f);

	Hashtable<int, int> textureTable;
	CreateTextureTable(textureTable, model);

	fwrite(&model.numOfObjects, sizeof(int), 1, out);
	
	for (int i = 0; i < model.numOfObjects; i++)
	{
		int materialIndex = model.meshes[i].materialID;

		fwrite(&model.meshes[i].numOfFaces, sizeof(int), 1, out);
		
		for (int j = 0; j < model.meshes[i].numOfFaces; j++)
		{
			int * vertexIndex = model.meshes[i].faces[j].vertIndex;
			
			for (int k = 0; k < 3; k++)
			{
				int bind(0);
				float r(0.0f), g(0.0f), b(0.0f);
								
				ConvertColorUBToF(model.materials[materialIndex].color.r, model.materials[materialIndex].color.g, model.materials[materialIndex].color.b, r, g, b);
				
				fwrite(&model.meshes[i].vertices[vertexIndex[k]], sizeof(float) * 3, 1, out);
				fwrite(&r, sizeof(float), 1, out);
				fwrite(&g, sizeof(float), 1, out);
				fwrite(&b, sizeof(float), 1, out);
				fwrite(&one, sizeof(float), 1, out);
				
				if (model.meshes[i].bHasTexture)
				{
					bind = textureTable[model.materials[materialIndex].textureId];
					fwrite(&bind, sizeof(int), 1, out);
					fwrite(&model.meshes[i].uvs[vertexIndex[k]].u, sizeof(float), 1, out);
					fwrite(&model.meshes[i].uvs[vertexIndex[k]].v, sizeof(float), 1, out);
					
					fprintf(asciiout, "UV: Index: %d U: %f V: %f\n", vertexIndex[k], model.meshes[i].uvs[vertexIndex[k]].u, model.meshes[i].uvs[vertexIndex[k]].v);
				}
				else
				{
					fwrite(&bind, sizeof(int), 1, out);
					fwrite(&zero, sizeof(float), 2, out);
				}

				fwrite(&model.meshes[i].normals[vertexIndex[k]].x, sizeof(float), 1, out);
				fwrite(&model.meshes[i].normals[vertexIndex[k]].y, sizeof(float), 1, out);
				fwrite(&model.meshes[i].normals[vertexIndex[k]].z, sizeof(float), 1, out);
			}
		}
	}

	outfile.TrimEnd(5);
	outfile += "txt.lo";

	FILE * txtOut = fopen(outfile.getCString(), "w");
	fprintf(txtOut, "%d\n", textureTable.getCount());	

	//DebugBreak();
	for (int i = 0; i < textureTable.getCount(); i++)
	{
		String s = model.materials[i].fileName;
		fprintf(txtOut, "<texture id=\"%d\">\"%s\"</texture>\n", textureTable[model.materials[i].textureId], model.materials[i].fileName);
	}
	
	fflush(txtOut);


	return true;
}

int main(int argc, char * argv[])
{	

//	DebugBreak();

	if (argc < 4)
	{
		cerr << "Usage: " << argv[0] << " <infile> <outputpath> <objectname>\n" << endl;
		return 1;
	}	    		

	String infile;
	String outfile;
	
	if (!ParseArgs(argc, argv, outfile, infile))
	{
		return 2;
	}

	Converter3ds conv;
	Converter3ds::Data3ds * model = conv.ConvertToData3ds(infile);

	if (!model)
	{
		return 3;
	}
	
	if (!Write3dbin(outfile, *model))
	{
		return 4;
	}

	return 0;
}

