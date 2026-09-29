#ifndef CURSOR_H
#define CURSOR_H

class Cursor3d
{
	friend class UserInterface;
	public:
		Cursor3d();
		enum MouseType { normal };	
		int getX();
		int getY();
		int getOldX();
		int getOldY();
		int getRelX();
		int getRelY();
		void setRelX(int value);
		void setRelY(int value);
		void setWheel(int wheelValue);
		int getWheel();
		void Display();
		void DrawShadow(unsigned char red , unsigned char green, unsigned char blue );
		void PositionMouse(int x, int y );
		bool onRelease[3];
		bool onClick[3];
		bool onDown[3];
		void setCursor( bool setting );


	private:
		bool visible;
		Window3d * window;
		int x;
		int y;
		int oldx;
		int oldy;
		int relX;
		int relY;
		int wheelValue;
		MouseType type;
};

#endif