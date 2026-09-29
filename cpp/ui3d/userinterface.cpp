/*
	Mark Tulewicz
	Last Modified 04/26/04
*/


/******************************************************************
           UI 
	@desc = The UI Class ( UIC ) is a wrapper for the above
			classes. It holds the collection and performs
			the operations to all windows for you. Allowing the
			user to simply call its commands for it to responde
			accordingly and automatically performs all checks.
*****************************************************************/
#include "userinterface.h"

int UserInterface::windowCount;
int UserInterface::menuCount;
int UserInterface::selectedObject = -1;
int UserInterface::WindowHeight = 768;
int UserInterface::WindowWidth = 1024;
EventHandlers UserInterface::eventHandler;
unsigned int UserInterface::fontID = 0;

//UserInterface::Operation UserInterface::operation;
UserInterface::Command UserInterface::command;
List< UserInterface::Msg > UserInterface::msgQue;


Window3d * UserInterface::windows[101];
MenuBar * UserInterface::menu = NULL;
MenuBar * UserInterface::context = NULL;

HGDIOBJ UserInterface::font;
HWND UserInterface::hWnd;
Cursor3d UserInterface::mouse;

//Colors
UserInterface::UIColor UserInterface::textColor = UserInterface::UIColor(30,30,30);
UserInterface::UIColor UserInterface::folderColor = UserInterface::UIColor(0,0,230);
UserInterface::UIColor UserInterface::windowColor = UserInterface::UIColor(255, 255, 255);
UserInterface::UIColor UserInterface::mouseColor = UserInterface::UIColor(255, 255, 255);
UserInterface::UIColor UserInterface::windowShadowColor = UserInterface::UIColor(20, 20, 20);
UserInterface::UIColor UserInterface::mouseShadowColor = UserInterface::UIColor(20, 20, 20);
UserInterface::UIColor UserInterface::menuBorderColorUpperOn = UserInterface::UIColor(255, 255, 255);
UserInterface::UIColor UserInterface::menuBorderColorLowerOn = UserInterface::UIColor(255 / 2, 255 / 2, 255 /2);
UserInterface::UIColor UserInterface::menuBorderColorUpperOff = UserInterface::UIColor(128, 128, 128);
UserInterface::UIColor UserInterface::menuBorderColorLowerOff = UserInterface::UIColor(64, 64, 64);
UserInterface::UIColor UserInterface::menuTextColor = UserInterface::UIColor(30, 30, 30);
UserInterface::UIColor UserInterface::menuSelectedTextColor = UserInterface::UIColor(255, 255, 255);

/***************************************************************
			User Interface Message structure vital!
***************************************************************/
UserInterface::Msg::Msg() 
{
	message = UserInterface::NoOp;
	dataInfo = NULL;
}

UserInterface::Msg::Msg(UserInterface::Operation newOperation, void * newDataInfo, String string)
{
	message = newOperation;
	specialString = string;

	switch (newOperation)
	{
		case UserInterface::ModelModReady:
			dataInfo = new float[9];
			memcpy(dataInfo, newDataInfo, sizeof(float) * 9 );
			break;		
		default:
			break;
	}
}

//copy constuct
UserInterface::Msg::Msg(const UserInterface::Msg & source)
{
	message = source.message;
	specialString = source.specialString;


	switch (source.message)
	{
		case UserInterface::ModelModReady:
			if (source.dataInfo)
			{
				dataInfo = new float[9];
				memcpy(dataInfo, source.dataInfo, sizeof(float) * 9 );
			}
			else
			{
				dataInfo = NULL;
			}
			break;		
		case UserInterface::SetFOV:
			if ( source.dataInfo )
			{
				dataInfo = new float[1];
				memcpy(dataInfo, source.dataInfo, sizeof(float) );
			}
			else
			{
				dataInfo = NULL;
			}
			break;
		default:
			dataInfo = source.dataInfo;
			break;
	}
}

//assignment
const UserInterface::Msg & UserInterface::Msg::operator = (const UserInterface::Msg & rhs)
{		
	if (this == &rhs)
	{
		return *this;
	}

	switch (message)
	{
		case UserInterface::ModelModReady:
			if (dataInfo)
			{
				delete [] dataInfo;
				dataInfo = NULL;
			}
			break;
		case UserInterface::SetFOV:
			if ( dataInfo )
			{
				delete [] dataInfo;
				dataInfo = NULL;
			}
			break;
		default:			
			break;
	}

	switch (rhs.message)
	{
		case UserInterface::ModelModReady:
			if (rhs.dataInfo)
			{
				dataInfo = new float[9];
				memcpy(dataInfo, rhs.dataInfo, sizeof(float) * 9 );
			}
			break;
		case UserInterface::SetFOV:
			{
				dataInfo = new float[1];
				memcpy(dataInfo, rhs.dataInfo, sizeof(float) );
			}
			break;
		default:
			dataInfo = rhs.dataInfo;
			break;		
	}

	message = rhs.message;
	specialString = rhs.specialString;
	return (*this);
}

//Destructor
UserInterface::Msg::~Msg()
{
	switch(message)
	{
		case UserInterface::ModelModReady:
			if (dataInfo)
			{
				delete [] dataInfo;
			}
			break;
		case UserInterface::SetFOV:
			if ( dataInfo )
			{
				delete [] dataInfo;
			}
			break;
		default:
			break;
	}
}

/*************************************************************
					User Interface Class!!!!!!!!!!
*************************************************************/

UserInterface::UserInterface() {}

void UserInterface::Init(HWND handle)
{	
	//Windows Handle
	hWnd = handle;

	//Command is the current state table for the internal controls.
	command = UserInterface::None;

	//Put the font into the graphix card.
	fontID = glGenLists(256);

	//Make the font.
	HFONT hFont = CreateFont( 18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, 
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		FF_DONTCARE | DEFAULT_PITCH, "Courier");

	font = SelectObject(GetDC(handle),hFont);
	//make bitmaps from the font.
	wglUseFontBitmaps(GetDC(handle),0,255, fontID);

	//Init all the windows.
	for (int i = 0; i < 101; i++ )
	{
		windows[i] = NULL;
	}
	
	command = UserInterface::None;

	selectedObject = -1;
	menuCount = 0;
	AddMenuLayout1();	
}

void UserInterface::AddMenuLayout1()
{
	//Syntax.  Assign the bar
	menu = new MenuBar();
	int menuNum = 0;
	//Add a menu across like file edit view project.
	//then fill the downward menus whenever u want just put the index across to put it to.
	menu->addMenuAcross("File");
	menu->addMenuDown("Connect",menuNum) -> AssignActionEvent(EventHandlers::Connect);
	menu->addMenuDown("Exit",menuNum++) -> AssignActionEvent(EventHandlers::Quit);

	menu->addMenuAcross("Edit");
	menu->addMenuDown("Duplicate", menuNum++) -> AssignActionEvent(EventHandlers::DuplicateSelectedObject);

	menu->addMenuAcross ("Settings");
	Menu * x1 = menu->addMenuDown("Manipulator",menuNum);
	menu->addMenuDown("Grid", menuNum) -> AssignActionEvent(EventHandlers::ColorSettings);
	Menu * x3 = menu->addMenuDown("World", menuNum++);


	x1->Disable();
	
	x3->Disable();

	menu->addMenuAcross("View");
	menu->addMenuDown("3d",menuNum) -> AssignActionEvent(EventHandlers::SetView3d);
	menu->addMenuDown("Front", menuNum) -> AssignActionEvent(EventHandlers::SetViewFront);
	menu->addMenuDown("Back", menuNum) -> AssignActionEvent(EventHandlers::SetViewBack);
	menu->addMenuDown("Left", menuNum) -> AssignActionEvent(EventHandlers::SetViewLeft);
	menu->addMenuDown("Right", menuNum) -> AssignActionEvent(EventHandlers::SetViewRight);
	menu->addMenuDown("Top", menuNum) -> AssignActionEvent(EventHandlers::SetViewTop);
	menu->addMenuDown("Bottom", menuNum) -> AssignActionEvent(EventHandlers::SetViewBottom);
	menu->addMenuDown(" ", menuNum);

	Menu * holo = menu->addMenuDown("HoloGrid", menuNum);
	holo -> AssignActionEvent(EventHandlers::ToggleHoloGrid);
	holo -> menuChecked = true;

	menu->addMenuDown("Wireframe", menuNum) -> AssignActionEvent(EventHandlers::SetWireframeMode);
	menu->addMenuDown("Inner Grid", menuNum++)  -> AssignActionEvent(EventHandlers::ToggleInnerGrid);

	UserInterface::menu->ToggleCheckMark("3d");

	menu->addMenuAcross("Scene");
	menu->addMenuDown("New",menuNum) -> AssignActionEvent(EventHandlers::NewScene);
	menu->addMenuDown("Save", menuNum) -> AssignActionEvent(EventHandlers::SaveScene);
	menu->addMenuDown("Open", menuNum) -> AssignActionEvent(EventHandlers::LoadScene);
	menu->addMenuDown("Add World Force", menuNum++)->AssignActionEvent(EventHandlers::AddWorldForcesMenu);

	menu->addMenuAcross("Model");
	menu->addMenuDown("Load",menuNum) -> AssignActionEvent(EventHandlers::LoadModel);
	menu->addMenuDown("Unload",menuNum++) -> AssignActionEvent(EventHandlers::UnloadModel);
//	menu->addMenuDown("Properties",menuNum++) -> AssignActionEvent(EventHandlers::ObjectProperties);

}

//Context menu is the same as a menu menu, but it has an X and Y location != to 0.
MenuBar * UserInterface::AddContextMenuObject()
{
	if (context)
	{
		delete context;
		context = NULL;
	}

	if ( UserInterface::getMouseX() < 128 && getMouseY() < 20  )
	{
		return context;
	}
	else
	{
		context = new MenuBar(UserInterface::getMouseX(), UserInterface::getMouseY() );

		return context;
		
	}
}

//Destructor for the static class.
void UserInterface::Release()
{
	DeleteObject(font);	
	
	for (int i = 0; i < 101; i++ )
	{
		if ( windows[i] )
			delete windows[i];
	}
}

// Adding a window ( UP TO 300 of them )
// Windows can be added, new windows are auto focused.
void UserInterface::AddOpenWindow(char * title)
{
	if ( windowCount == 99 ) 
		return;

	if ( Window3d::winID >= 99 )
		Window3d::winID = 0;

	while ( windows[Window3d::winID] != NULL )
	{
		Window3d::winID++;

		if ( Window3d::winID >= 99 )
			Window3d::winID = 0;
	}

	Window3d * window = windows[Window3d::winID - 1] = new Window3d(fontID, Window3d::WindowType::FileOpen);
	window->setCaption(String(title));
	window->okButton->setCaption(String(title) );
}


void UserInterface::AddObjPropWindow()
{
	if ( windowCount == 99 ) 
		return;

	if ( Window3d::winID >= 99 )
		Window3d::winID = 0;

	while ( windows[Window3d::winID] != NULL )
	{
		Window3d::winID++;

		if ( Window3d::winID >= 99 )
			Window3d::winID = 0;
	}

	windows[Window3d::winID - 1] = new Window3d(fontID, Window3d::WindowType::ObjectProp);
	
	//This is the same as OnLoad of this window, request the information for the model;
	Msg m;
	m.message = UserInterface::RequestModelInfo;
	EventEnque(m);
}
void UserInterface::AddWorldPropWindow()
{
	if ( windowCount == 99 ) 
		return;

	if ( Window3d::winID >= 99 )
		Window3d::winID = 0;

	while ( windows[Window3d::winID] != NULL )
	{
		Window3d::winID++;

		if ( Window3d::winID >= 99 )
			Window3d::winID = 0;
	}

	windows[Window3d::winID - 1] = new Window3d(fontID, Window3d::WindowType::WorldProp);
}

Window3d * UserInterface::AddVideoWindow(int x, int y, int w, int h)
{
	if ( windowCount == 99 ) 
		return NULL;

	if ( Window3d::winID >= 99 )
		Window3d::winID = 0;

	while ( windows[Window3d::winID] != NULL )
	{
		Window3d::winID++;

		if ( Window3d::winID >= 99 )
			Window3d::winID = 0;
	}

	Window3d * window = new Window3d(fontID, Window3d::WindowType::Generic);

	windows[Window3d::winID - 1] = window;
	window->setCaption("Video Camera");
	window->setWidth(w);
	window->setHeight(h);
	window->setPos(x, y);


	return window;
}

Window3d * UserInterface::AddNetConnectWindow(char * title, char * defIP, char * defPort)
{
	if ( windowCount == 99 ) 
		return NULL;

	if ( Window3d::winID >= 99 )
		Window3d::winID = 0;

	while ( windows[Window3d::winID] != NULL )
	{
		Window3d::winID++;

		if ( Window3d::winID >= 99 )
			Window3d::winID = 0;
	}

	Window3d * window = new Window3d(fontID, Window3d::WindowType::NetConnect);

	windows[Window3d::winID - 1] = window;
	window->setCaption(String(title));
	window->ip->setText(String(defIP));
	window->port->setText(String(defPort));
	return window;
}

Window3d * UserInterface::AddObjectPhysics()
{
	if ( windowCount == 99 ) 
		return NULL;

	if ( Window3d::winID >= 99 )
		Window3d::winID = 0;

	while ( windows[Window3d::winID] != NULL )
	{
		Window3d::winID++;

		if ( Window3d::winID >= 99 )
			Window3d::winID = 0;
	}

	Window3d * window = new Window3d(fontID, Window3d::WindowType::AddObjectPhysics);

	windows[Window3d::winID - 1] = window;
	return window;
}

Window3d * UserInterface::AddWorldForceWindow()
{
	if ( windowCount == 99 ) 
		return NULL;

	if ( Window3d::winID >= 99 )
		Window3d::winID = 0;

	while ( windows[Window3d::winID] != NULL )
	{
		Window3d::winID++;

		if ( Window3d::winID >= 99 )
			Window3d::winID = 0;
	}

	Window3d * window = new Window3d(fontID, Window3d::WindowType::AddWorldForce);

	windows[Window3d::winID - 1] = window;
	return window;
}




/*********************************************************************************
Use:  This will add a message box to the windows array, and set up all the stuff
**********************************************************************************/
void UserInterface::AddMessageBox(char * Title, char * msg)
{
	if ( windowCount == 99 ) 
		return;

	if ( Window3d::winID >= 99 )
		Window3d::winID = 1;

	while ( windows[Window3d::winID] != NULL )
	{
		Window3d::winID++;

		if ( Window3d::winID >= 99 )
			Window3d::winID = 0;
	}

	Window3d * window = windows[Window3d::winID - 1] = new Window3d(fontID, Window3d::WindowType::Message);
	String title = String(Title);
	String message = String(msg);
	window->setCaption(title);
	window->setMsg ( message );
}

Window3d * UserInterface::AddCameraWindow()
{
	if ( windowCount == 99 ) 
		return NULL;

	if ( Window3d::winID >= 99 )
		Window3d::winID = 0;

	while ( windows[Window3d::winID] != NULL )
	{
		Window3d::winID++;

		if ( Window3d::winID >= 99 )
			Window3d::winID = 0;
	}

	Window3d * window = windows[Window3d::winID - 1] = new Window3d(fontID, Window3d::WindowType::CameraWindow);
	return window;
}

/***************************************************************************
Use:	Useful for any yes or no answer
		You have to specify the UserInterface::Command
		Seperately in order to check the answer
		Check the answer in Button3d.cpp Under the action performed
		heading.  And then just check the command and reset it when done.
***************************************************************************/
Window3d * UserInterface::AddConfirmWindow(char * Title, char * msg, char * ok, char * cancel)
{
	if ( windowCount == 99 ) 
		return NULL;

	if ( Window3d::winID >= 99 )
		Window3d::winID = 0;

	while ( windows[Window3d::winID] != NULL )
	{
		Window3d::winID++;

		if ( Window3d::winID >= 99 )
			Window3d::winID = 0;
	}

	Window3d * window = new Window3d(fontID, Window3d::WindowType::Confirm);

	windows[Window3d::winID - 1] = window;
	String title = String(Title);
	String message = String(msg);
	window->setCaption(title);
	window->setMsg ( message );
	window->SetButtonCaptions(ok, cancel);
	return window;
}

//Used to adjust the position of the cursor in the focal window and focal textbox.
void UserInterface::MoveTextCursor(int amount)
{
	if ( windows[Window3d::focusID] )
		windows[Window3d::focusID]->MoveTextCursor(amount);	
}

//Move the focal window. 
void UserInterface::MoveWindow()
{
	if ( windows[Window3d::focusID] )
		windows[Window3d::focusID]->moveWindow(mouse);
}

//Control the UI's mouse.
void UserInterface::setMousePos(int x, int y)
{
	mouse.PositionMouse(x, y);
}

//Sets up the mouse properties for you.
//Takes the change in x, the change in y, the amount the mouse wheel moved, position of mouse, an array of button states
void UserInterface::PerformMouseEvents(int relX, int relY, int relZ, int x, int y, unsigned char buttons[])
{
	UserInterface::setRelX(relX);
	UserInterface::setRelY(relY);
	
	static bool buttonPressed[3] = { false, false, false };

	for ( int i = 0; i < 3; i++ )
	{
		//If the left click is down
		if ( buttons[i] )
		{
			UserInterface::setMouseDown(i, true);
			//Then it was not just released.
			UserInterface::setMouseRelease(i, false);

			//If the button was just up, and now its down, then it was just clicked.
			if ( buttonPressed[i] == false )
			{
				UserInterface::setMouseClick(i, true);
				buttonPressed[i] = true;
			}
			else
			{
				UserInterface::setMouseClick(i, false );
			}
		}

		//If the left click is not pressed.
		if ( !buttons[i])
		{
			UserInterface::setMouseDown(i, false);
			//if the button was just pressed and now its not pressed, then it was just released and not clicked.
			if ( buttonPressed[i] == true )
			{
				UserInterface::setMouseRelease(i, true);
				UserInterface::setMouseClick(i, false);
				buttonPressed[i] = false;
			}
			//If the button was just unpressed and now its still unpressed, then its not clicked or released.
			else
			{
				UserInterface::setMouseRelease(i, false);
				UserInterface::setMouseClick(i, false);
			}
		}
	}
	//On first clicking, try to give some control focus there.
	if ( relZ != 0 )
	{
		if ( !UserInterface::UserInterfaceActive() )
		{
			UserInterface::Msg msg;
			msg.message = UserInterface::SetFOV;
			float * subtract = new float[1];
			subtract[0] = (float)relZ / 120.0f;
			msg.dataInfo = subtract;
			memcpy(msg.dataInfo, subtract, sizeof(float) * 1 );
			UserInterface::EventEnque( msg );
		}

		UserInterface::setMouseWheel(-relZ / 120);
	}

	//do final checks for mouse overs.
	UserInterface::CheckButtonClick();

	UserInterface::setMousePos(x, y);
	UserInterface::MoveWindow();
	ShowCursor(false);
	return;
}


// This will remove any window that is finished.
void UserInterface::KillFocalWindow()
{
	if ( windows[Window3d::focusID] )
	{
		if ( windows[Window3d::focusID]->getType() == Window3d::Generic )
		{
			windows[Window3d::focusID]->setType( Window3d::Message );
			return;
		}
		delete windows[Window3d::focusID];
		windows[Window3d::focusID] = NULL;
	}
}

void UserInterface::UpdateMessageBox(char * msg)
{
	if ( windows[Window3d::focusID])
		if ( windows[Window3d::focusID]->type == Window3d::WindowType::Message)
			windows[Window3d::focusID]->setMsg(String(msg));
}

int UserInterface::getWindowCount()
{
	return windowCount;
}

bool UserInterface::MenuActive( )
{
	if ( menu )
	{
		if ( menu->getMenuState() != MenuBar::Off && menu->getMenuState() != MenuBar::Min)
			return true;
	}
	if ( context ) 
	{
		if ( context->getMenuState() != MenuBar::Off && context->getMenuState() != MenuBar::Min   )
		{		
			return true;
		}
	}
	return false;
}

void UserInterface::ToggleMainMenu()
{
	if ( menu )
	{
		if ( !UserInterface::MenuActive() )
		{
			menu->state = MenuBar::Engaging;
		}
		else
		{
			menu->state = MenuBar::Retracting;
		}
	}
}

/******************************************************
						Events
******************************************************/
//User input

// Will check if a button was clicked.
void UserInterface::CheckButtonClick()
{	
	//Menus are always checked first.
	if ( menu )
	{
		if ( menu->ClickMenu(mouse) == true)
		{
			//The wheel data MUST be reset to 0 after every draw, so we will do that here now.
			UserInterface::setMouseWheel(0);
			return;
		}
	}
	if ( context )
	{
		if ( context->ClickMenu(mouse) == true )
		{
			//wheel is done.
			UserInterface::setMouseWheel(0);
			return;
		}
	}

	//Then the focal window is always checked.
	if ( windows[Window3d::focusID] )
	{
		if ( windows[Window3d::focusID]->CheckMouseInWindow(mouse) )
		{
			//Try to give focus this will know if a click happen or not;
			windows[Window3d::focusID]->giveFocus();
			//Try clicking in the window.
			windows[Window3d::focusID]->ClickInWindow(mouse);
			//wheel is done.
			UserInterface::setMouseWheel(0);
			return;
		}
	}

	//Last check windows, they inturn will check the buttons and textboxes etc.
	for ( int i = 0; i < 101; i++)
	{
		if ( windows[i] )
		{
			if ( windows[i]->CheckMouseInWindow(mouse) )
			{
				windows[i]->giveFocus();
				windows[i]->ClickInWindow(mouse);
				//Wheel is done.
				UserInterface::setMouseWheel(0);
				return;
			}
		}
	}
}

//This function occurs after selecting Properties.  A message is made, and the inputhandler takes the information
//of the selected object, and sends it into this function.  Which places it nicely on the screen.
void UserInterface::SendModelInfo(const float * rot, const float * scale, const float * trans, String & name, String & comments)
{
	//Goes to the focus window if it was made.
	if (UserInterface::windows[Window3d::focusID] )
	{
		if ( !UserInterface::windows[Window3d::focusID]->objCaption || !UserInterface::windows[Window3d::focusID]->objName)
		{
			return;
		}

		UserInterface::windows[Window3d::focusID]->xRotate->setText(String::ToString(rot[0]));
		UserInterface::windows[Window3d::focusID]->yRotate->setText(String::ToString(rot[1]));
		UserInterface::windows[Window3d::focusID]->zRotate->setText(String::ToString(rot[2]));

		UserInterface::windows[Window3d::focusID]->xScale->setText(String::ToString(scale[0]));
		UserInterface::windows[Window3d::focusID]->yScale->setText(String::ToString(scale[1]));
		UserInterface::windows[Window3d::focusID]->zScale->setText(String::ToString(scale[2]));
	
		UserInterface::windows[Window3d::focusID]->xTrans->setText(String::ToString(trans[0]));
		UserInterface::windows[Window3d::focusID]->yTrans->setText(String::ToString(trans[1]));
		UserInterface::windows[Window3d::focusID]->zTrans->setText(String::ToString(trans[2]));
		UserInterface::windows[Window3d::focusID]->setCaption(name);
		UserInterface::windows[Window3d::focusID]->objName->setText(name);
		UserInterface::windows[Window3d::focusID]->objCaption->setText(comments);
	}
}

void UserInterface::sendKeyPress(char key, bool shift)
{
	//Directs a key into the UI.. to a window, or to a text box.. depends on whats focused.
	if ( UserInterface::windows[Window3d::focusID] )
	{
		if ( key == 9)
		{
			if (shift)
			{
				UserInterface::windows[Window3d::focusID]->selectPreviousBox();
			}
			else
			{
				UserInterface::windows[Window3d::focusID]->selectNextBox();
			}
		}
		else
		{
			//Send it to the right window, they will handle it from there.
			UserInterface::windows[Window3d::focusID]->passNumberToBox(key);
		}
	}
}

void UserInterface::SelectNextBox()
{
	if ( UserInterface::windows[Window3d::focusID] )
		UserInterface::windows[Window3d::focusID]->selectNextBox();
}

void UserInterface::SelectPrevBox()
{
	if ( UserInterface::windows[Window3d::focusID] )
		UserInterface::windows[Window3d::focusID]->selectPreviousBox();
}

void UserInterface::EventEnque(UserInterface::Msg event)
{
	UserInterface::msgQue.PushBack(event);
}

bool UserInterface::CheckForMessages( Msg & event)
{
	if ( msgQue.getSize() == 0 )
	{
		return false;
	}
	else
	{
		msgQue.PopFront(event);
		return true;
	}
}


/******************************************************
						GL FUNCTIONS
******************************************************/

//Draws all windows and makes sure that the focal window is on top.
//Note, this pushes its own matrix on the stack to make it "float" 
//Above all else.
void UserInterface::DisplayWindows()
{
	//save wireframe mode, for later use
	GLint wireframe[2];
	glGetIntegerv(GL_POLYGON_MODE, wireframe);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	//test to see if Depth test is currently on
	int res = 0;
	glGetIntegerv(GL_DEPTH_TEST, &res);	
	if ( res )
	{
		glDisable(GL_DEPTH_TEST);
	}

	glMatrixMode ( GL_PROJECTION );
	glPushMatrix();
	{
		glLoadIdentity();
		glViewport(0,0,1024,768);
		glOrtho(0,1024, 768, 0,-300.0,300);

		//Done in order, lowest windows are always the lowest ID.
		for (int i = 0; i < 101; i++ )
		{
			if ( windows[i] )
			{
				if ( Window3d::focusID != i )
					windows[i]->Display();
				
				//Take this time to destroy any windows marked for closing.
				if ( windows[i]->state.getState() == WindowState::WindowStates::CLOSING || windows[i]->state.IsDead() )
				{
					KillFocalWindow();
				}
			}
		}

		//Focal window must be on top!! draw it again last.
		if (windows[Window3d::focusID])
			windows[Window3d::focusID]->Display();

		//Menus always go above windows.
		if (menu)
			menu->DisplayMenu();
		if ( context )
			context->DisplayMenu();
		//Mouse is always drawn above everything else.
		mouse.Display();
	}		
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	
	//if depth test was on, turn it back on now that we are done
	if ( res )
	{
		glEnable(GL_DEPTH_TEST);
	}

	//if wireframe mode was set, then turn it back on.
	glPolygonMode(GL_FRONT_AND_BACK, wireframe[1]);
}

void UserInterface::DisplayMouse()
{

}

void UserInterface::PrintGL(float x, float y, float z, String text)
{
	//Draws the text in 3space.
	//Adjust your matrix.
	glPushMatrix();
		glRasterPos3f(x, y , z);
		glListBase(fontID);
		glCallLists(text.getLength(), GL_UNSIGNED_BYTE, text.getCString());
	glPopMatrix();


}

void UserInterface::PrintGL(int x, int y, String text)
{	
	//test to see if Depth test is currently on
	int res = 0;
	glGetIntegerv(GL_DEPTH_TEST, &res);	
	if ( res )
	{
		glDisable(GL_DEPTH_TEST);
	}
	//Adjust your matrix.
	glMatrixMode ( GL_PROJECTION );
	glPushMatrix();
		glLoadIdentity();
		glViewport(0,0,1024,768);
		glOrtho(0, WindowWidth, WindowHeight, 0, -300,300);
	
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
			glLoadIdentity();
			glRasterPos2i( (GLint)( x), (GLint) (y ));
			glListBase(fontID);
			glCallLists(text.getLength(), GL_UNSIGNED_BYTE, text.getCString());
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);

	//NOTE:: You have to do this afterwards.
	if (res)
	{
		glEnable(GL_DEPTH_TEST);
	}

}

void UserInterface::SendForceList(vector < String > forces )
{
	if ( UserInterface::windows[Window3d::focusID] )
	{
		Window3d * window = UserInterface::windows[Window3d::focusID];
		
		for ( int i = 0; i < (int)forces.size(); i++ )
		{
			window->ForceList->AddItem(forces[i]);
		}
	}
}

void UserInterface::HideMouse()
{
	UserInterface::mouse.setCursor(false);
}

void UserInterface::ShowMouse()
{
	UserInterface::mouse.setCursor(true);
}

//Accessors:
bool UserInterface::UserInterfaceActive()
{
	if ( MenuActive() )
	{
		return true;
	}

	if ( windowCount == 0 )
	{
		return false;
	}
	else
	{
		for ( int i = 0; i < 101; i++)
		{
			if ( windows[i] )
			{
				if ( windows[i] -> state.getState() == WindowState::DRAGGING )
				{
					return true;
				}

				if ( windows[i]->type == Window3d::Message || windows[i]->type == Window3d::Generic)
				{
					continue;
				}
				else
					return true;
			}
		}
	}

	return false;
}

//Color
UserInterface::UIColor::UIColor()
{
}

UserInterface::UIColor::UIColor(unsigned char r, unsigned char g, unsigned char b)
{
	UserInterface::UIColor::r = r;
	UserInterface::UIColor::g = g;
	UserInterface::UIColor::b = b;
}

UserInterface::UIColor UserInterface::getColorFromUIColor(UserInterface::UIColor::Colors color)
{
	if (color == UIColor::Black )
	{
		return UIColor(0,0,0);
	}
	else if ( color == UIColor::Blue )
	{
		return UIColor(0,0,255);
	}
	else if ( color == UIColor::Cyan )
	{
		return UIColor(0,255,255);
	}
	else if ( color == UIColor::Green )
	{
		return UIColor(0,255,0);
	}
	else if ( color == UIColor::Orange )
	{
		return UIColor(255,128,0);
	}
	else if ( color == UIColor::Purple) 
	{
		return UIColor(255,0,255);
	}
	else if ( color == UIColor::Red )
	{
		return UIColor(255,0,0);
	}
	else if ( color == UIColor::White )
	{
		return UIColor(255,255,255);
	}
	else if ( color == UIColor::Yellow )
	{
		return UIColor(255,255,0);
	}
	else
	{
		return UIColor(0,0,0);
	}
}

void UserInterface::setTextColor(unsigned char r, unsigned char g, unsigned char b)
{
	UserInterface::textColor = UIColor(r,g,b);
}

void UserInterface::setFolderColor(unsigned char r, unsigned char g, unsigned char b)
{
	UserInterface::folderColor = UIColor(r,g,b);
}

void UserInterface::setWindowColor(unsigned char r, unsigned char g, unsigned char b)
{
	UserInterface::windowColor = UIColor(r, g, b);
}

void UserInterface::setMouseColor(unsigned char r, unsigned char g, unsigned char b)
{
	UserInterface::mouseColor = UIColor(r,g,b);
}

void UserInterface::setWindowShadowColor(unsigned char r, unsigned char g, unsigned char b)
{
	UserInterface::windowShadowColor = UIColor(r,g,b);
}

void UserInterface::setMouseShadowColor(unsigned char r, unsigned char g, unsigned char b)
{
	UserInterface::windowShadowColor = UIColor(r,g,b);
}

void UserInterface::setMenuTextColor(unsigned char r, unsigned char g, unsigned char b)
{
	UserInterface::menuTextColor = UIColor(r,g,b);
}

void UserInterface::setMenuBorderColorUpperOn(unsigned char r, unsigned char g, unsigned char b)
{
	UserInterface::menuBorderColorUpperOn = UIColor(r,g,b);
}

void UserInterface::setMenuBorderColorLowerOn(unsigned char r, unsigned char g, unsigned char b)
{
	UserInterface::menuBorderColorLowerOn = UIColor(r,g,b);
}

void UserInterface::setMenuBorderColorUpperOff(unsigned char r, unsigned char g, unsigned char b)
{
	UserInterface::menuBorderColorUpperOff = UIColor(r,g,b);
}

void UserInterface::setMenuBorderColorLowerOff(unsigned char r, unsigned char g, unsigned char b)
{
	UserInterface::menuBorderColorLowerOff = UIColor(r,g,b);
}

void UserInterface::setMenuSelectedTextColor(unsigned char r, unsigned char g, unsigned char b)
{
	UserInterface::menuSelectedTextColor = UIColor(r,g,b);
}

void UserInterface::setSelectedObject( int x ) { selectedObject = x ;}
void UserInterface::setMouseClick(int button, bool setting){  UserInterface::mouse.onClick[button] = setting; }
void UserInterface::setMouseRelease(int button, bool setting){  UserInterface::mouse.onRelease[button] = setting; }
void UserInterface::setMouseDown(int button, bool setting){  UserInterface::mouse.onDown[button] = setting; }
void UserInterface::setMouseWheel(int amount ){ UserInterface::mouse.setWheel(amount); }
void UserInterface::setRelX(int amount ){ UserInterface::mouse.setRelX(amount); }
void UserInterface::setRelY(int amount ){ UserInterface::mouse.setRelY(amount); }

Menu * UserInterface::getMenuByCaption(String caption ) { return UserInterface::menu->FindMenu( caption); }
MenuBar * UserInterface::getMenuBar( ) { return UserInterface::menu; }
int UserInterface::getMouseX() { return UserInterface::mouse.getX(); }
int UserInterface::getMouseY() { return UserInterface::mouse.getY(); }
int UserInterface::getMouseOldX() { return UserInterface::mouse.getOldX(); }
int UserInterface::getMouseOldY() { return UserInterface::mouse.getOldY(); }
int UserInterface::getMouseRelX() { return UserInterface::mouse.getRelX(); }
int UserInterface::getMouseRelY() { return UserInterface::mouse.getRelY(); }

bool UserInterface::OnMouseClick(int button ){ return UserInterface::mouse.onClick[button]; }
bool UserInterface::OnMouseRelease(int button){ return UserInterface::mouse.onRelease[button]; }
bool UserInterface::OnMouseDown(int button){ return UserInterface::mouse.onDown[button]; }
int UserInterface::OnMouseWheel(){ return UserInterface::mouse.getWheel() ;}

