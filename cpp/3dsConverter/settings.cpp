#include "settings.h"

/**************************************

   	    	World Settings

**************************************/

States::World::System Settings::system = States::World::Initalizing;
String Settings::fileName = String("");
int Settings::mouseOnObject = -1;
int Settings::WindowHeight = 768;
int Settings::WindowWidth = 1024;

/**************************************

   	    	Grid Settings

**************************************/
States::World::GridState Settings::Grid::gridState = States::World::Enable;
int Settings::Grid::height = 500;
int Settings::Grid::width = 0;
int Settings::Grid::finalD = 1000;
int Settings::Grid::finalH = 500;
int Settings::Grid::finalW = 1000;
int Settings::Grid::length = 0;
float Settings::Grid::dynamicScale = 0.0f;
float Settings::Grid::opacity = 100;
float Settings::Grid::scale = 1;
int Settings::Grid::spacing = 100;
int Settings::Grid::tickness = 4;
unsigned char Settings::Grid::red = 255;
unsigned char Settings::Grid::green = 255;
unsigned char Settings::Grid::blue = 0;