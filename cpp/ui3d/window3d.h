#ifndef WINDOW3D
#define WINDOW3D

#include "userinterface.h"
#include "wstring.h"
#include "wlist.h"



class WindowState
{
	friend class Window3d;

	public :
		enum WindowStates{DRAGGING, IDLE, RESIZE, CLOSING };
		
		WindowStates getState();
		void setState(WindowStates stateToSet);
		void setKillFlag();
		bool IsDead();

	private:
		WindowStates state;
		bool killFlag;

};

class Window3d
{
	friend class EventHandlers;
	friend class Button3d;
	friend class TextBox3d;
	friend class UserInterface;
	friend class ListBox3d;
	friend class Slider3d;

	public:
		enum WindowType { Generic, FileOpen, FileSave, ObjectProp, WorldProp, Message, Confirm, NetConnect, AddObjectPhysics, AddWorldForce, CameraWindow};
		WindowState state;

		Window3d();
		Window3d(unsigned int font, Window3d::WindowType typeOfWindow);
		~Window3d();
		void setWindowFont(unsigned int fontListID);

		static int winID;
		static int focusID;

		bool CheckMouseInWindow(Cursor3d & mouse);
		bool CheckMouseInTitle(Cursor3d & mouse);
		void ClickInWindow(Cursor3d & mouse);

		Button3d * addButton(Button3d::ButtonType type, int x, int y);
		Slider3d * addSlider(int x, int y, int width, int height);
		//void addButton(Button3d::ButtonType type, int * x, int * y);
		TextBox3d * addText(int x, int y, int width, int height);
		TextBox3d * addLabel(int x, int y, int width, int height, String text);
		ListBox3d * addList(int x, int y, int width, int height, bool fileList = true);
		void SetButtonCaptions(char * okButton, char * cancelButton);
		void MoveTextCursor(int amount);
		void moveWindow(Cursor3d & mouse);
		void giveFocus();
		void selectNextBox();
		void selectPreviousBox();
		
		void Display();
		void DrawShadow(unsigned char red, unsigned char blue, unsigned char green );
		
		void passNumberToBox(char number);
		void setTextID(int id);
		void setCaption(String caption);
		void setType( WindowType type );
		void setMsg(String msg);

		int getTextID();
		int getID();
		int getX();
		int getY();
		int getCurveRad();
		int getWidth();
		int getHeight();
		void setWidth(int w);
		void setHeight(int h);
		void setPos(int x, int y);

		WindowType getType();
		String getFileSelect();

		ListBox3d * ForceList;
		ListBox3d * AppliedList;

	protected:
		int x;
		int y;

		int curveRad;
		
		String caption;
		String fileName;
		List < String > message;

		WindowType type;

		Button3d * buttons[300];
		TextBox3d * textBoxes[300];
		Slider3d * sliders[300];
		ListBox3d * lists[5];

		//Text box ptrs:
		//Used by objProp window
		TextBox3d * xTrans;
		TextBox3d * yTrans;
		TextBox3d * zTrans;

		TextBox3d * xRotate;
		TextBox3d * yRotate;
		TextBox3d * zRotate;

		TextBox3d * xScale;
		TextBox3d * yScale;
		TextBox3d * zScale;

		TextBox3d * forceName;
		TextBox3d * forceDur;
		TextBox3d * forceMass;

		TextBox3d * objName;
		TextBox3d * objCaption;

		//Button Ptrs:
		Button3d * okButton;
		Button3d * applyButton;
		Button3d * cancelButton;
		Button3d * closeButton;

		//Texts for LogonWindow type ( why I didn't use inheritance.. i don't know, but its too late now.
		TextBox3d * ip;
		TextBox3d * port;
		TextBox3d * user;
		TextBox3d * pass;

		//Forces
		TextBox3d * vX;
		TextBox3d * vY;
		TextBox3d * vZ;


		//Slider Ptrs:
		Slider3d * slider;

		int buttonCount;
		int textCount;
		int slideCount;
		int listCount;
		int textFocus;
		int id;
		int width;
		int height;
		unsigned int fontID;

};


#endif