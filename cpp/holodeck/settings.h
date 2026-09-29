#ifndef SETTINGS_H
#define SETTINGS_H

#include "states.h"
#include "wstring.h"

class Settings
{
	public:
	//	static GetObject(int objectWorldId);
		
		static bool isDepthTestOn();

		static bool ScreenLock;

		static States::World::System system;
		static States::World::System oldSystemState;
		static String fileName;
		static int mouseOnObject;
		static int WindowWidth;
		static int WindowHeight;
		
		static bool InnerGrid;
		static bool WireframeMode;
		static float maxPlayerSpeed;
		static float accelSpeed;
		static float currentSpeed;

		static float gravity;

		class Grid
		{		
			private:
				Grid();
			public:
				enum GridColor { Red, Green, Blue, Yellow, Cyan, Purple, Orange, White, Black };
				static void SetGridSpacing( int thickness, int spacing);
				static void SetGridColor ( unsigned char red, unsigned char gren, unsigned char blue);
				static void SetGridBackColor ( unsigned char red, unsigned char gren, unsigned char blue);
				static void SetGridColor ( GridColor color );
				static States::Grid::GridState gridState;		
				static int thickness;
				static int length;
				static int width;
				static int spacing;
				static int height;
				static int finalW;
				static int finalH;
				static float scale;
				static unsigned char red;
				static unsigned char green;
				static unsigned char blue;

				static unsigned char bred;
				static unsigned char bgreen;
				static unsigned char bblue;

				static float dynamicScale;

				static int finalD;
				static float opacity;
		};

	private:
		Settings();


};
#endif