/**********************************************************************
	BUTTONS
	@desc = The button class is a subclass of the window class.
			Buttons are placed inside of the button array 
			found in the window class.  The position for a button
			is relative to the window. Buttons have different types
			that can be assigned and will perform accordingly.
  *********************************************************************/

#include "userinterface.h"
#include "menubar.h"
#include "listbox.h"
#include "eventhandlers.h"

Button3d::Button3d()
{
	this->window = NULL;
	this->caption = "OK";
}

Button3d::Button3d(Window3d * window, unsigned int fontId)
{
	this->x = window->x + window->width;
	this->y = window->y + window->height;
	this->width = 16;
	this->height = 20;
	this->type = Button3d::OK;
	this->caption = "OK";
	this->window = window;
	this->fontListID = fontId;
	this->xPtr = NULL;
	this->yPtr = NULL;
	this->AssignActionEvent(EventHandlers::DefaultCloseWindow);
}

Button3d::~Button3d()
{
	this->ActionPerformed = 0;
	this->window;

}

//Event Handler stuff
void Button3d::AssignActionEvent(ButtonEvent target )
{

	this->ActionPerformed = target;
}


//Has been replaced with function pointers to assigned functions in the EventHandlers Class
//Usually triggered by a CLICK event controlled by the cursor class.
void Button3d::actionPerformed()
{
//	if ( type == Button3d::OK )
//	{
//		//x
//		if ( window->type == Window3d::WindowType::NetConnect)
//		{
//			UserInterface::Msg m;
//			m.message = UserInterface::ConnectionRequest;
//			m.specialString = window->ip->getCaption();
//			UserInterface::EventEnque(m);
//
//			//window->state.setState(WindowState::WindowStates::CLOSING );
//			//window->state.setKillFlag();
//		}
//
//		else if ( window->type == Window3d::WindowType::FileOpen )
//		{
//			//x
//			if ( UserInterface::command == UserInterface::RequestLoadModel )
//			{
//				String filename = window->lists[0]->getFilename();
//				//Safety checks for file consistancy:
//				if ( filename.getLength() > 0 )
//				{
//					if ( filename[filename.getLength() - 1] == '>' || filename[filename.getLength() - 1] == '\\')
//					{
//						UserInterface::command = UserInterface::None;
//						window->state.setState(WindowState::WindowStates::CLOSING );
//						window->state.setKillFlag();
//						return;
//					}
//				}
//				else 
//				{
//					UserInterface::command = UserInterface::None;
//					window->state.setState(WindowState::WindowStates::CLOSING );
//					window->state.setKillFlag();
//					return;
//				}
//				
//				UserInterface::Msg m;
//				UserInterface::command = UserInterface::None;
//				m.message = UserInterface::ModelLoadReady;
//				m.specialString = window->lists[0]->getFilename();
//				UserInterface::EventEnque(m);
//			}
//			//x
//			else if ( UserInterface::command == UserInterface::RequestLoadScene )
//			{
//				String filename = window->lists[0]->getFilename();
//				
//				//Safety checks for filename consistancy:
//				if ( filename.getLength() > 0 )
//				{
//					if ( filename[filename.getLength() - 1] == '>' || filename[filename.getLength() - 1] == '\\')
//					{
//						UserInterface::command = UserInterface::None;
//						window->state.setState(WindowState::WindowStates::CLOSING );
//						window->state.setKillFlag();
//						return;
//					}
//				}
//				else 
//				{
//					UserInterface::command = UserInterface::None;
//					window->state.setState(WindowState::WindowStates::CLOSING );
//					window->state.setKillFlag();
//					return;
//				}
//
//				UserInterface::Msg m;
//				UserInterface::command = UserInterface::None;
//
//				m.message = UserInterface::SceneLoadReady;
//				m.specialString = window->lists[0]->getFilename();
//				UserInterface::EventEnque(m);
//			}
//			//x
//			else if ( UserInterface::command == UserInterface::RequestSaveScene )
//			{
//				String filename = window->lists[0]->getFilename();
//				
//				//Safety checks for filename consistancy:
//				if ( filename.getLength() > 0 )
//				{
//					if ( filename[filename.getLength() - 1] == '>' || filename[filename.getLength() - 1] == '\\')
//					{
//						UserInterface::command = UserInterface::None;
//						window->state.setState(WindowState::WindowStates::CLOSING );
//						window->state.setKillFlag();
//						return;
//					}
//				}
//				else 
//				{
//					UserInterface::command = UserInterface::None;
//					window->state.setState(WindowState::WindowStates::CLOSING );
//					window->state.setKillFlag();
//					return;
//				}
//
//				UserInterface::Msg m;
//				UserInterface::command = UserInterface::None;
//
//				m.message = UserInterface::SceneSaveReady;
//				m.specialString = window->lists[0]->getFilename();
//				UserInterface::EventEnque(m);
//
//			}
//		}
//		else if( window->type == Window3d::WindowType::Confirm )
//		{
//			//x
//			if ( UserInterface::command == UserInterface::RequestExit )
//			{							
//				//exit(0)								
//				UserInterface::Msg msg;
//				msg.message = UserInterface::Exit;
//				msg.dataInfo = reinterpret_cast<void *>(0);
//				UserInterface::EventEnque(msg);
//			}
////x
//			else if ( UserInterface::command == UserInterface::RequestUnloadModel )
//			{
//				UserInterface::Msg m;
//				m.message = UserInterface::ModelUnloadReady;
//				UserInterface::EventEnque (m );
//				UserInterface::command = UserInterface::None;
//			}
//
//			//x
//			else if ( UserInterface::command == UserInterface::RequestUnloadScene )
//			{
//				UserInterface::Msg m;
//				m.message = UserInterface::SceneUnloadReady;
//				UserInterface::EventEnque (m );
//				UserInterface::command = UserInterface::None;
//			}
//			
//			//x
//			else if ( UserInterface::command == UserInterface::RequestGridSettings )
//			{
//				UserInterface::command = UserInterface::None;
//				if ( caption == "White BG")
//				{
//					UserInterface::Msg m;
//					m.message = UserInterface::SelectGridColorScheme;
//					m.specialString = String("White");
//					UserInterface::EventEnque(m);
//				}
//				else if ( caption == "Black BG")
//				{
//					UserInterface::Msg m;
//					m.message = UserInterface::SelectGridColorScheme;
//					m.specialString = String("Black");
//					UserInterface::EventEnque(m);
//				}
//			}
//
//
//
//		}
//
//		window->state.setState(WindowState::WindowStates::CLOSING );
//		window->state.setKillFlag();
//	}
//	else if ( type == Button3d::CANCEL )
//	{
//		if ( window->type == Window3d::WindowType::NetConnect || window->type == Window3d::WindowType::Confirm || window->type == Window3d::WindowType::ObjectProp )
//		{
//			UserInterface::command = UserInterface::None;
//			window->state.setState(WindowState::WindowStates::CLOSING );
//			window->state.setKillFlag();
//		}
//
//	}
//	else if ( type == Button3d::Transform)
//	{
//		//x
//		if ( window->type == Window3d::WindowType::ObjectProp )
//		{			
//			float * temp = new float[9];
//			
//			temp[0] = (window->xRotate->getCaption() != NULL) ? (window->xRotate->getCaption()).ToFloat() : 0.0f;
//			temp[1] = (window->yRotate->getCaption()!= NULL) ? (window->yRotate->getCaption()).ToFloat() : 0.0f;
//			temp[2] = (window->zRotate->getCaption()!= NULL) ? (window->zRotate->getCaption()).ToFloat() : 0.0f;
//			temp[3] = (window->xScale->getCaption()!= NULL) ? (window->xScale->getCaption()).ToFloat() : 0.0f;
//			temp[4] = (window->yScale->getCaption()!= NULL) ? (window->yScale->getCaption()).ToFloat() : 0.0f;
//			temp[5] = (window->zScale->getCaption()!= NULL) ? (window->zScale->getCaption()).ToFloat() : 0.0f;
//			temp[6] = (window->xTrans->getCaption()!= NULL) ? (window->xTrans->getCaption()).ToFloat() : 0.0f;
//			temp[7] = (window->yTrans->getCaption()!= NULL) ? (window->yTrans->getCaption()).ToFloat() : 0.0f;
//			temp[8] = (window->zTrans->getCaption()!= NULL) ? (window->zTrans->getCaption()).ToFloat() : 0.0f;
//
//			UserInterface::Msg m(UserInterface::ModelModReady, temp);
//						
//			UserInterface::EventEnque(m);
//
//			delete [] temp;
//		}
//	}
//
//	else if ( type == Button3d::CLOSE )
//	{
//		//x
//		UserInterface::command = UserInterface::None;
//		window->state.setState(WindowState::WindowStates::CLOSING );
//		window->state.setKillFlag();
//	}
//	else if ( type == Button3d::SCROLLDOWN )
//	{
//		if ( window->lists[0] )
//			window->lists[0]->Scroll(1);
//	}
//	else if ( type == Button3d::SCROLLUP )
//	{
//		if ( window->lists[0] )
//			window->lists[0]->Scroll(-1);
//	}
//
//	UserInterface::command = UserInterface::None;
}

//This will draw the different kinds of buttons in openGL code.
void Button3d::DisplayButton()
{
	if ( xPtr )
		x = *xPtr;
	if ( yPtr )
		y = *yPtr;
	if ( type == ButtonType::OK || type == ButtonType::CANCEL || type == ButtonType::Transform || type == ButtonType::Connect)
	{
		int xbase = window->x + x;
		int ybase = window->y + y;
		int buf = 5;

		glBegin(GL_QUADS);
			glColor3ub(210,210,210);
			glVertex2d(xbase, ybase);
			glVertex2d(xbase + width / 7, ybase );
			
			glColor3ub(175, 175, 175);
			glVertex2d(xbase + width / 7, ybase + height);
			glVertex2d(xbase, ybase + height);

			glColor3ub(210,210,210);
			glVertex2d(xbase + width / 7 + buf , ybase);
			glVertex2d(xbase + width, ybase );
			
			glColor3ub(175, 175, 175);
			glVertex2d(xbase + width, ybase + height);
			glVertex2d(xbase + width / 7 + buf, ybase + height);

		glEnd();

		int xbase2 = xbase + width / 7 + buf ;
		int ybase2 = ybase;
		int baseWidth = width - width / 7 - buf;
		
		glColor3ub(UserInterface::textColor.r, UserInterface::textColor.g, UserInterface::textColor.b);
		glRasterPos2i( xbase2 + (baseWidth - caption.getLength() * 10) / 2, ybase2 + height - 4 );
		glListBase(this->fontListID);
		glCallLists(caption.getLength(), GL_UNSIGNED_BYTE, (GLvoid*)caption.getCString());
	}
	else if ( type == ButtonType::CLOSE )
	{
		glBegin(GL_QUADS);
			glColor3ub(50,0,25);
			glVertex2d(window->getX() + this->x , window->getY() + this->y);
			glVertex2d(window->getX() + this->x + width , window->getY());

			glColor3ub(200,0,100);
			glVertex2d(window->getX() + this->x + width , window->getY() + this->height);
			glVertex2d(window->getX() + this->x , window->getY() + this->height);
		glEnd();

		glColor3ub(255,0,0);
		glRasterPos2i( window->getX() + x  + ((width / 2) - 4 ) , window->getY() + y + height - 4 );
		glListBase(this->fontListID);
		glCallLists(caption.getLength(), GL_UNSIGNED_BYTE, (GLvoid*)caption.getCString());
	}
	else if ( type == ButtonType::SCROLLDOWN )
	{
		glBegin(GL_QUADS);
			glColor3ub(200,200,200);
			glVertex2d(window->getX() + this->x , window->getY() + y);
			glVertex2d(window->getX() + this->x + width , window->getY() + y );

			glColor3ub(100,100,100);
			glVertex2d(window->getX() + this->x + width , window->getY()+ y + height);
			glVertex2d(window->getX() + this->x , window->getY() + y + height);
		glEnd();

		glBegin(GL_TRIANGLES);
			glColor3ub(50,50,50);
			glVertex2d(window->getX() + x + 5, window->getY() + y + 5);
			glVertex2d(window->getX() + x + 15, window->getY() + y + 5);
			glColor3ub(50,50,50);
			glVertex2d(window->getX() + x + 10, window->getY() + y + 15);
		glEnd();

	}
	else if ( type == ButtonType::SCROLLUP )
	{
		glBegin(GL_QUADS);
			glColor3ub(100,100,100);
			glVertex2d(window->getX() + this->x , window->getY() + this->y);
			glVertex2d(window->getX() + this->x + width , window->getY() + y);

			glColor3ub(200,200,200);
			glVertex2d(window->getX() + this->x + width , window->getY() + y + this->height);
			glVertex2d(window->getX() + this->x , window->getY() + y + this->height);
		glEnd();

		glBegin(GL_TRIANGLES);
			glColor3ub(50,50,50);
			glVertex2d(window->getX() + x + 5, window->getY() + y + 15);
			glVertex2d(window->getX() + x + 15, window->getY() + y + 15);
			glColor3ub(50,50,50);
			glVertex2d(window->getX() + x + 10, window->getY() + y + 5);
		glEnd();

	}
	
	else if ( type == ButtonType::RESIZE)
	{

	}	
}

//Will assign the settings for each button.
void Button3d::setType(Button3d::ButtonType type)
{
	this->type = type;

	if ( type == ButtonType::OK )
	{
		this->width = 85;
		this->height = 18;
		caption = "OK";
	}
	else if ( type == ButtonType::Transform )
	{
		this->width = 85;
		this->height = 18;
		caption = "Apply";

	}
	else if ( type == ButtonType::CANCEL)
	{
		this->width = 85;
		this->height = 18;
		caption = "Cancel";
	}

	else if ( type == ButtonType::CLOSE )
	{
		this->width = window->getCurveRad() * 2 / 3;
		this->height = 20;
		caption = "X";
	}
		
	else if ( type == ButtonType::SCROLLDOWN )
	{
		this->width = 20;
		this->height = 20;
		caption = "V";
		this->AssignActionEvent(&EventHandlers::ScrollDown);
	}

	else if ( type == ButtonType::SCROLLUP )
	{
		this->width = 20;
		this->height = 20;
		caption = "^";
		this->AssignActionEvent(&EventHandlers::ScrollUp);
	}

	else if ( type == ButtonType::RESIZE)
	{

	}
}

//Button position settings.
void Button3d::setPos(int x, int y )
{
	this->x = x;
	this->y = y;
}

/*void Button3d::setPos(int* x, int* y )
{
	this->xPtr = x;
	this->yPtr = y;
}
*/

//This function will check to see if the mouse is clicked on this button.
//The type of click is defined by the UI wrapper. ( on release )
void Button3d::OnClick()
{
	(UserInterface::eventHandler.*ActionPerformed)(this);
}

bool Button3d::CheckIfClicked(Cursor3d & mouse)
{
	if ( mouse.onRelease[0] && (mouse.getX() > window->x + x && mouse.getX() < window->x + x + width &&
			mouse.getY() > window->y + y && mouse.getY() < window->y + y + height) )
	{
		this->OnClick();
		//this->actionPerformed();
		return true;
	}
	else 
	{
		return false;
	}
}

String Button3d::getCaption() { return caption; }
Window3d * Button3d::getParent() { return window; }
void Button3d::setCaption(String text){ caption = text; this->width = caption.getLength() * 16 + 10; }
int Button3d::getX() { return this->x; }
int Button3d::getY() { return this->y; }
int Button3d::getHeight() { return this->height; }
int Button3d::getWidth() { return this->width; }