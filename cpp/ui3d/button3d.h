#ifndef BUTTON3D_H
#define BUTTON3D_H

#include "eventhandlers.h"

class Button3d
{
	public:
		Button3d(Window3d * window, unsigned int fontListID);
		~Button3d();
		bool CheckIfClicked(Cursor3d & mouse);
		void DisplayButton();
		void AssignActionEvent(ButtonEvent function);
		void OnClick();

		void actionPerformed();

		enum ButtonType { Action, CLOSE, RESIZE, OK, CANCEL, SCROLLDOWN, SCROLLUP, Rotate, Scale, Transform, Connect};
		void setPos(int x, int y);
		void setType(ButtonType type);
	
		Window3d * getParent();
		String getCaption();
		void setCaption(String text);
		int getWidth();
		int getHeight();
		int getX();
		int getY();
		ButtonType getType();

	private:
		Button3d();
		void (EventHandlers::*ActionPerformed)(Button3d * button);
		int x;
		int y;
		int * yPtr;
		int * xPtr;
		int width;
		int height;
		String caption;
		Window3d * window;
		unsigned int fontListID;

		ButtonType type;
};

#endif