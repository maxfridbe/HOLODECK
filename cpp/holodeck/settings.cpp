#include "settings.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

/**************************************

   	    	World Settings

**************************************/

bool Settings::isDepthTestOn()
{
	int res = 0;
	glGetIntegerv(GL_DEPTH_TEST, &res);	
	if ( res )
	{
		return true;
	}
	return false;
}


//States::World::System Settings::system = States::World::Initalizing;
bool Settings::ScreenLock = false;
States::World::System Settings::system = States::World::Design;
States::World::System Settings::oldSystemState = Settings::system;
String Settings::fileName = String("");
int Settings::mouseOnObject = -1;
int Settings::WindowHeight = 768;
int Settings::WindowWidth = 1024;
bool Settings::WireframeMode = false;
bool Settings::InnerGrid = false;
float Settings::gravity = 10.0f;
float Settings::maxPlayerSpeed = 15.0;
float Settings::accelSpeed = 1.0f;
float Settings::currentSpeed = 0.0f;
//Object Settings::object;

//void Settings::GetObject(int objectWorldId)
//{

//}

/**************************************

   	    	Grid Settings

**************************************/
States::Grid::GridState Settings::Grid::gridState = States::Grid::Enable;
int Settings::Grid::height = 500;
int Settings::Grid::width = 0;
int Settings::Grid::finalD = 1000;
int Settings::Grid::finalH = 1000;
int Settings::Grid::finalW = 1000;
int Settings::Grid::length = 0;
float Settings::Grid::dynamicScale = 0.0f;
float Settings::Grid::opacity = 100;
float Settings::Grid::scale = 1;
int Settings::Grid::spacing = 100;
int Settings::Grid::thickness = 3;
unsigned char Settings::Grid::red = 255;
unsigned char Settings::Grid::green = 255;
unsigned char Settings::Grid::blue = 0;

unsigned char Settings::Grid::bred = 0;
unsigned char Settings::Grid::bgreen = 0;
unsigned char Settings::Grid::bblue = 0;

//Spacing settings
void Settings::Grid::SetGridSpacing(int thickness, int spacing)
{
	Settings::Grid::thickness = thickness;
	Settings::Grid::spacing = spacing;
}




//Color Settings
void Settings::Grid::SetGridColor( unsigned char r, unsigned char g, unsigned char b)
{
	Settings::Grid::red = r;
	Settings::Grid::green = g;
	Settings::Grid::blue = b;
}

void Settings::Grid::SetGridBackColor( unsigned char r, unsigned char g, unsigned char b)
{
	Settings::Grid::bred = r;
	Settings::Grid::bgreen = g;
	Settings::Grid::bblue = b;
}

void Settings::Grid::SetGridColor( Settings::Grid::GridColor color )
{
	if ( color == Settings::Grid::Red )
	{
		Settings::Grid::red = 255;
		Settings::Grid::green = 0;
		Settings::Grid::blue = 0;
	}
	else if ( color == Settings::Grid::Green)
	{
		Settings::Grid::red = 0;
		Settings::Grid::green = 255;
		Settings::Grid::blue = 0;
	}
	else if ( color == Settings::Grid::Blue )
	{
		Settings::Grid::red = 0;
		Settings::Grid::green = 0;
		Settings::Grid::blue = 255;
	}
	else if ( color == Settings::Grid::Cyan )
	{
		Settings::Grid::red = 0;
		Settings::Grid::green = 255;
		Settings::Grid::blue = 255;
	}
	else if ( color == Settings::Grid::Yellow )
	{
		Settings::Grid::red = 255;
		Settings::Grid::green = 255;
		Settings::Grid::blue = 0;
	}
    else if ( color == Settings::Grid::Purple)
	{
		Settings::Grid::red = 255;
		Settings::Grid::green = 0;
		Settings::Grid::blue = 255;
	}
	else if ( color == Settings::Grid::Orange )
	{
		Settings::Grid::red = 255;
		Settings::Grid::green = 128;
		Settings::Grid::blue = 0;
	}
	else if ( color == Settings::Grid::Black )
	{
		Settings::Grid::red = 0;
		Settings::Grid::green = 0;
		Settings::Grid::blue = 0;
	}
	else if ( color == Settings::Grid::White )
	{
		Settings::Grid::red = 255;
		Settings::Grid::green = 255;
		Settings::Grid::blue = 255;
	}
}