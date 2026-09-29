#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "wstring.h"

class TextBox3d
{
	public:
		TextBox3d(Window3d * window, unsigned int fontListID, int ID);
		~TextBox3d();

		void DisplayTextBox();
		void setPos(int x, int y);
		void setSize(int w, int h);
		void setText( String text );
		void appendText( int pos, String text );
		void MoveCursor(int amount);
		void MoveCursorTo(int pos);

		bool CheckIfClicked(Cursor3d & mouse);
		void insertLetter(char letter);

		String getCaption();
		int getWidth();
		int getHeight();
		int getX();
		int getY();
		void setReadOnly(bool state);
		void setPass(bool state);
		void setDigit(bool state);
		bool isLocked();
		bool isDigit();
		bool isPass();

private:
		int x;
		int y;
		int ID;
		int width;
		int height;
		int maxLine;
		int cursor;
		bool lock;
		bool isNumber;
		bool isPassword;
		unsigned char redTxt;
		unsigned char greenTxt;
		unsigned char blueTxt;
		unsigned char redHighlite;
		unsigned char greenHighlite;
		unsigned char blueHighlite;

		unsigned char redLowlite;
		unsigned char greenLowlite;
		unsigned char blueLowlite;

		String data;
		unsigned int fontListID;
		Window3d * window;
};


#endif