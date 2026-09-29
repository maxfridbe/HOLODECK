#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <gl/gl.h>
#include <gl/glu.h>

#include "Window3d.h"
#include "listbox.h"

int Window3d::winID = 0;
int Window3d::focusID = 0;

/**************************
	Support State Class
	@desc = This class is the finite state machine for the
			windowing class. It controls if the window is 
			in a moving state, or closing state.
***************************/

void WindowState::setState(WindowStates setState )
{
	state = setState;
}

WindowState::WindowStates WindowState::getState( ) 
{
	return state;
}

void WindowState::setKillFlag()
{
	state = WindowState::CLOSING;
	UserInterface::Msg m;
	m.message = UserInterface::Operation::WindowClosed;
	m.dataInfo = this;
	UserInterface::EventEnque(m);
	this->killFlag = true;
}

bool WindowState::IsDead()
{
	return killFlag;
}


/************************
        windows 
*************************/

Window3d::Window3d() {}

//Pulls down the count of windows open. and deallocates buttons.
Window3d::~Window3d()
{
	UserInterface::windowCount--;
	for ( int i = 0; i < 300; i++)
	{
		if ( buttons[i] )
			delete buttons[i];
	}
}

//Window constructor.  It sets everything up!
Window3d::Window3d(unsigned int fontID, WindowType wType) : xTrans(NULL), yTrans(NULL), zTrans(NULL), xRotate(NULL), yRotate(NULL), zRotate(NULL), xScale(NULL), yScale(NULL), zScale(NULL)
{
	//Redundant code:
	UserInterface::windowCount++;
	id = winID;
	winID++;

	focusID = id;

	memset(buttons,0, sizeof(buttons));
	memset(textBoxes,0, sizeof(textBoxes));
	memset(sliders,0, sizeof(sliders));
	memset(lists,0, sizeof(lists));

	this->curveRad = 2;
	this->textFocus = 0;
	this->applyButton = NULL;
	this->okButton = NULL;
	this->closeButton = NULL;
	this->cancelButton = NULL;
	this->state.killFlag = false;
	this->fontID = fontID;
	this->buttonCount = 0;
	this->textCount = 0;
	this->slideCount = 0;
	this->listCount = 0;
	this->x = 0;
	this->y = 0;
	this->width = 200;
	this->height = 200;
	state.setState ( WindowState::WindowStates::IDLE );
	caption = String("Window ") + String::ToString(this ->id);

	this->xRotate = NULL;
	this->yRotate = NULL;
	this->zRotate = NULL;
	this->xScale = NULL;
	this->yScale = NULL;
	this->zScale = NULL;
	this->xTrans = NULL;
	this->yTrans = NULL;
	this->zTrans = NULL;
	this->objName = NULL;


	//Specific code:
	type = wType;
	if ( wType == WindowType::FileOpen )
	{
		this->width = 350;
		this->height = 400;
		this->x = 50;
		this->y = 50;
		this->curveRad = (width / 10 + height / 10) / 2 ;
		this->setCaption("Load");
	
		// Add an OK button.
		okButton = addButton(Button3d::OK,(width / 2) - 80, height - curveRad - 5);

		switch ( UserInterface::command )
		{
			case UserInterface::RequestLoadModel:
				okButton->AssignActionEvent((ButtonEvent)EventHandlers::LoadModel);				
				
				break;
			case UserInterface::RequestLoadScene:
				okButton->AssignActionEvent((ButtonEvent)EventHandlers::LoadScene);
				
				break;
			case UserInterface::RequestSaveScene:
				okButton->AssignActionEvent((ButtonEvent)EventHandlers::SaveScene);
				break;
		}

		addList(50,50,250,height - 125);
	}
	else if ( wType == WindowType::CameraWindow )
	{
		this->width = 550;
		this->height = 250;
		this->x = 75;
		this->y = 75;
		this->curveRad = (width / 10 + height / 10) / 2 ;
		this->setCaption("Add Camera");
		
		this->addLabel(30, 30, 200, 20, "Camera Name: ");
		this->forceName = addText(30, 55, 200,20);

		this->addLabel(30, 130, 200, 20, "Position: ");


		


	}

	else if ( wType == WindowType::AddWorldForce )
	{
		this->width = 600;
		this->height = 550;
		this->x = 75;
		this->y = 75;
		this->curveRad = (width / 10 + height / 10) / 2 ;
		this->setCaption("World Forces");
	
		//The rest is in the MenuHandler.. it builds the window. Cleaner...

	}

	else if ( wType == WindowType::AddObjectPhysics )
	{
		this->width = 600;
		this->height = 550;
		this->x = 75;
		this->y = 75;
		this->curveRad = (width / 10 + height / 10) / 2 ;
		this->setCaption("Attach Physics");

		// Add an OK button.
		okButton = addButton(Button3d::OK, 260, 480);
		okButton->setCaption("Close");
		okButton->AssignActionEvent(EventHandlers::DebugClick);
		//okButton->AssignActionEvent(EventHandlers::DefaultCloseWindow);

		int heightL0 = 75;
		int heightL1 = 215;
		int heightL2 = 275;

		//Left List ( forces )
		ForceList = addList(30, heightL0, 200, 350, false);
		//Right List
		AppliedList = addList(375, heightL0, 200, 350, false);
		AppliedList->setClickLock(true);

		Button3d * transRight = addButton(Button3d::OK, 260, heightL1);
		Button3d * transLeft = addButton(Button3d::OK, 260, heightL2);

		okButton = transRight;

		transRight->AssignActionEvent(EventHandlers::AddForce );
		transRight->setCaption(" >>  ");
		transLeft->AssignActionEvent(EventHandlers::RemoveForce );
		transLeft->setCaption(" <<  ");

		okButton = transRight;
	}


	else if ( wType == WindowType::NetConnect )
	{
		this->width = 550;
		this->height = 250;
		this->x = 75;
		this->y = 75;
		this->curveRad = (width / 10 + height / 10) / 2 ;
		this->setCaption("Network Connect");
		
		// Add an OK button.
		okButton = addButton(Button3d::OK,(width * 3/4 + 2), height - curveRad - 5);
		okButton->AssignActionEvent(&EventHandlers::NetConnect);
		
		int heightL0 = 30;
		int heightL1 = 70;
		int heightL2 = 115;
		int heightL3 = 150;

		//Labels:
		TextBox3d * lblTitle = addText(15, heightL0, 250, 18);
		lblTitle->setReadOnly(true);
		lblTitle->setText("Connect To Server:");

		TextBox3d * lblIP = addText(15, heightL1, 50, 18);
		lblIP->setReadOnly(true);
		lblIP->setText("IP:");

		TextBox3d * lblPort = addText(300,heightL1, 100, 18);
		lblPort->setReadOnly(true);
		lblPort->setText("Port:");

		TextBox3d * lblUser = addText(15,heightL2, 100, 18);
		lblUser->setReadOnly(true);
		lblUser->setText("Username:");

		TextBox3d * lblPass = addText(15,heightL3, 100, 18);
		lblPass->setReadOnly(true);
		lblPass->setText("Password:");

		//Main text boxes
		ip = addText(100, heightL1, 175, 18);
		port = addText(400, heightL1, 100, 18);
		user = addText(150, heightL2, 200, 18);
		pass = addText(150, heightL3, 200, 18);
		pass->setPass(true);

	}

	else if ( wType == WindowType::ObjectProp )
	{
		this->width = 550;
		this->height = 350;
		this->x = 50;
		this->y = 50;
		this->curveRad = (width / 10 + height / 10) / 2 ;
		this->setCaption("Transformations");

		// Add an OK button.
		cancelButton = addButton(Button3d::CANCEL, ((width / 2) - 50), height - 25);
		cancelButton->setCaption("Done");

		applyButton = addButton(Button3d::OK, ((width / 2) - 50), height - 65);
		applyButton->setCaption("Apply");
		applyButton->AssignActionEvent(&EventHandlers::ApplyTransform );
	

		//Labels and Info
		TextBox3d * Info = addText(200, 30, 200, 18);
		Info->setReadOnly(true);
		Info->setText("Object Information");

		TextBox3d * Name = addText(15, 55, 100, 18);
		Name->setReadOnly(true);
		Name->setText("Name:");

		TextBox3d * Comment = addText(15, 75, 100, 18);
		Comment->setReadOnly(true);
		Comment->setText("Caption:");

		this->objName = this->addText(150, 55, 400, 20);
		this->objCaption = this->addText(150, 75, 400, 20);
		objCaption->setReadOnly(true);
		objName->setReadOnly(true);

		//Translate Control boxes

		int heightTitle = 150;
		int heightCap = 175;
		int heightL1 = 200;
		int heightL2 = 225;
		int heightL3 = 250;

		TextBox3d * transform = addText(200, heightTitle, 200, 18);
		transform->setReadOnly(true);
		transform->setText("Transformations");

		TextBox3d * Tran = addText(15, heightCap, 100, 18);
		Tran->setReadOnly(true);
		Tran->setText("Translate");


		TextBox3d * labelX1 = addText(15, heightL1, 15, 18);
		TextBox3d * labelY1 = addText(15, heightL2, 15, 18);
		TextBox3d * labelZ1 = addText(15, heightL3, 15, 18);
		labelX1->setReadOnly(true);
		labelX1->setText("X:");
		labelY1->setReadOnly(true);
		labelY1->setText("Y:");
		labelZ1->setReadOnly(true);
		labelZ1->setText("Z:");

		xTrans = this->addText(40, heightL1,103,18);
		yTrans = this->addText(40, heightL2,103,18);
		zTrans = this->addText(40, heightL3,103,18);
		

		//Scale Control boxes

		TextBox3d* Sca = addText(200, heightCap, 75, 18);
		Sca->setReadOnly(true);
		Sca->setText("Scale");


		TextBox3d * labelX2 = addText(200, heightL1, 15, 18);
		TextBox3d * labelY2 = addText(200, heightL2, 15, 18);
		TextBox3d * labelZ2 = addText(200, heightL3, 15, 18);
		labelX2->setReadOnly(true);
		labelX2->setText("X:");
		labelY2->setReadOnly(true);
		labelY2->setText("Y:");
		labelZ2->setReadOnly(true);
		labelZ2->setText("Z:");

		xScale = this->addText(225, heightL1, 103,18);
		yScale = this->addText(225, heightL2, 103,18);
		zScale = this->addText(225, heightL3, 103,18);


		//Rotate Controls
		TextBox3d* Rot = addText(385, heightCap, 75, 18);
		Rot->setReadOnly(true);
		Rot->setText("Rotate By");


		TextBox3d * labelX3 = addText(385, heightL1, 15, 18);
		TextBox3d * labelY3 = addText(385, heightL2, 15, 18);
		TextBox3d * labelZ3 = addText(385, heightL3, 15, 18);
		labelX3->setReadOnly(true);
		labelX3->setText("X:");
		labelY3->setReadOnly(true);
		labelY3->setText("Y:");
		labelZ3->setReadOnly(true);
		labelZ3->setText("Z:");

		xRotate = this->addText(410, heightL1, 103, 18);
		yRotate = this->addText(410, heightL2, 103, 18);
		zRotate = this->addText(410, heightL3, 103, 18);

		xTrans->setDigit(true);
		yTrans->setDigit(true);
		zTrans->setDigit(true);
		xScale->setDigit(true);
		yScale->setDigit(true);
		zScale->setDigit(true);
		xRotate->setDigit(true);
		yRotate->setDigit(true);
		zRotate->setDigit(true);

	}

	else if ( wType == WindowType::Message )
	{	
		this->x = 250;
		this->y = 250;
		this->width = 400;
		this->height = 220;
		this->curveRad = (width / 10 + height / 10) / 2 ;
	
		// Add an OK button.
		okButton = addButton(Button3d::OK, width * 1/2, height * 5/7);
	}

	else if ( wType == WindowType::Confirm )
	{
		this->width = 400;
		this->height = 220;
		this->x = 30;
		this->y = 30;
		this->curveRad = (width / 10 + height / 10) / 2 ;
	

		// Add an OK button.
		okButton = addButton(Button3d::OK, x + width * 1/2 - width / 4, y + height * 5/7);
		cancelButton = addButton(Button3d::CANCEL, x + width * 1/2, y + height * 5/7);
		
		switch ( UserInterface::command )
		{
			case UserInterface::RequestExit:
				okButton->AssignActionEvent(&EventHandlers::QuitProgram);
				break;
			case UserInterface::RequestUnloadModel:
				okButton->AssignActionEvent(&EventHandlers::UnloadModel);
				break;
			case UserInterface::RequestUnloadScene:
				okButton->AssignActionEvent(&EventHandlers::UnloadScene);
				break;
			case UserInterface::RequestGridSettings:
				okButton->AssignActionEvent(&EventHandlers::GridSettings);
				break;
		}

		if ( UserInterface::command == UserInterface::RequestUnloadModel )
		{
			okButton->setCaption("Remove");
		}
		else if ( UserInterface::command == UserInterface::RequestUnloadScene )
		{
			okButton->setCaption("Create");
		}
	}		

	//Close Button
	this->closeButton = addButton(Button3d::CLOSE, width - 30, 0);
	this->closeButton->setCaption("X");
}

void Window3d::SetButtonCaptions(char * ok, char * cancel)
{
	if ( okButton )
	{
		okButton->setCaption(ok);
	}
	if ( cancelButton)
	{
		cancelButton->setCaption(cancel);
	}
}

//Check for mouse in window.
bool Window3d::CheckMouseInWindow(Cursor3d & mouse)
{
	if ( mouse.getX() <= x + width && mouse.getX() >= x && mouse.getY() <= y + height && mouse.getY() >= y)
		return true;
	else
		return false;
}

//See if the mouse is in the title bar.
bool Window3d::CheckMouseInTitle(Cursor3d & mouse)
{
	//Used to check if the window goes into drag mode.
	if ( mouse.getX() <= x + width && mouse.getX() >= x - 5 && mouse.getY() <= y + 32 && mouse.getY() >= y - 5)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//Perform a window CLICK and run any actions in there.
void Window3d::ClickInWindow(Cursor3d & mouse)
{
	//check if this is a click, or a scroll.
	if ( mouse.getWheel( ) == 0 )
	{

		if ( (mouse.onRelease[0] || mouse.onClick[0] || mouse.onDown [0]) && mouse.getX() <= x + width && mouse.getX() >= x && mouse.getY() <= y + height && mouse.getY() >= y)
		{
			for ( int l = 0; l < 300; l++ )
			{
				if ( sliders[l] )
				{
					if ( sliders[l]->CheckIfClicked(mouse))
						return;
				}
			}
		}

		//check if its a mouse release inside of this window.
		if ( (mouse.onRelease[0] || mouse.onClick[0]) && mouse.getX() <= x + width && mouse.getX() >= x && mouse.getY() <= y + height && mouse.getY() >= y)
		{
			//Find out if we're clicking a BUTTON, textbox, or listbox.
			for ( int i = 0; i < 300; i++ )
			{
				//if a button exists check if we're in it.
				if ( buttons[i] )
				{
					//Mouse in window, So check if any buttons were hit.
					if ( buttons[i]->CheckIfClicked(mouse) )
						return;
				}
			}

			// now check for textBoxes
			for ( int j = 0; j < 300; j++ )
			{
				if ( textBoxes[j] )
				{
					if ( textBoxes[j]->CheckIfClicked(mouse) )
					{
						return;
					}
				}
			}

			// Last check the list boxes
			for ( int j = 0; j < 5; j++ )
			{
				if ( lists[j] )
				{
					if ( lists[j]->CheckIfClicked(mouse) )
					{
						return;
					}
				}
			}
		}
		
		
	}
	//This is just a scroller so scroll whatever window is in focus.
	else
	{
		for ( int l = 0; l < 5; l++ )
		{
			if ( lists[l] )
			{
				lists[l]->Scroll ( mouse.getWheel() );
				return;
			}
		}	
	}
}

//Creates a new button inside the window.
Button3d * Window3d::addButton(Button3d::ButtonType type, int x, int y)
{
	buttons[buttonCount] = new Button3d( this, fontID );
	buttons[buttonCount]->setType(type);
	buttons[buttonCount]->setPos(x,y);
	
	buttonCount ++;
	return buttons[buttonCount - 1];
}

/*void Window3d::addButton(Button3d::ButtonType type, int * x, int * y)
{
	buttons[buttonCount] = new Button3d( this, fontID );
	buttons[buttonCount]->setType(type);
	buttons[buttonCount]->setPos(x,y);
	
	buttonCount ++;
}
*/

ListBox3d * Window3d::addList(int x, int y, int w, int h, bool fileList)
{
	ListBox3d * list1 = lists[listCount] = new ListBox3d(this,this->fontID,x,y,w,h, fileList);
	listCount ++;
	return list1;
}


TextBox3d * Window3d::addText(int x, int y, int width, int height)
{
	textBoxes[textCount] = new TextBox3d(this, fontID, textCount);
	textBoxes[textCount]->setPos(x,y);
	textBoxes[textCount]->setSize(width, height);
	textCount ++;
	return textBoxes[textCount - 1];
}

TextBox3d * Window3d::addLabel(int x, int y, int width, int height, String text)
{
	textBoxes[textCount] = new TextBox3d(this, fontID, textCount);
	textBoxes[textCount]->setPos(x,y);
	textBoxes[textCount]->setSize(width, height);
	textBoxes[textCount]->setReadOnly(true);
	textBoxes[textCount]->setText(text);
	textCount ++;
	return textBoxes[textCount - 1];
}

Slider3d * Window3d::addSlider(int x, int y, int width, int height)
{
	Slider3d * slide = sliders[slideCount] = new Slider3d(this, fontID);
	sliders[slideCount]->setPos(x,y);
	sliders[slideCount]->setSize(width, height);
	slideCount ++;
	return slide;
}

void Window3d::MoveTextCursor(int amount)
{
	textBoxes[textFocus]->MoveCursor(amount);
}

/*******************************
	@desc = This will set this 
			window to be the 
			focal window now.
*******************************/
void Window3d::giveFocus()
{
	if ( UserInterface::OnMouseClick())
		focusID = id;
}

/***************************************************************
	@desc = The move window class will check to see the 
			mouse was clicked and if its inside the title bar
			then will set the window state accordingly and move
			the window to the cursor's position.
****************************************************************/
void Window3d::moveWindow(Cursor3d & mouse)
{
	//Check if this window is the focus window ( it should be );
	if ( focusID == id && CheckMouseInTitle( mouse ) && mouse.onClick[0] )
	{
		state.setState( WindowState::WindowStates::DRAGGING );
	}
	else if ( focusID == id && mouse.onRelease[0] )
	{
		state.setState( WindowState::WindowStates::IDLE );
	}

	// perform the movement:
	if ( state.getState() == WindowState::WindowStates::DRAGGING )
	{
		x += mouse.getX() - mouse.getOldX();
		y += mouse.getY() - mouse.getOldY();
	}
}

//Draw the windows.
void Window3d::Display()
{
	//Safety::
	if (width < 99 )
		width = 100;
	if ( height < 99 )
		height = 100;

	//Start OGL Drawing.

	//Adjust your matrix.
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	{
	
		glLoadIdentity();

		//SHADOW
		DrawShadow(0,0,0);

		if ( focusID == id )
		{
			glColor4ub(245,245,245, 255);
		}
		else
		{
			glColor4ub(222,222,222,255);
		}
	
		//Window frame ( left panel )
		glBegin(GL_QUADS);					
			glVertex2d(x, y);
			glVertex2d(x + curveRad, y);
			glVertex2d(x + curveRad, y + height - curveRad );
			glVertex2d(x , y + height - curveRad);
			
			//Bottom panel
			glVertex2d(x + curveRad, y + height - curveRad);
			glVertex2d(x + width - curveRad, y + height - curveRad);
			glVertex2d(x + width - curveRad, y + height );
			glVertex2d(x + curveRad , y + height);
		glEnd();
	
		//Make left bottom window arc
		glBegin(GL_POLYGON);
			glVertex2d(x, y + height - curveRad );
			glVertex2d(x + curveRad, y + height - curveRad );
			glVertex2d(x + curveRad, y + height );
			glVertex2d(x + curveRad - curveRad/2, y + height - curveRad /6);
			glVertex2d(x + curveRad / 6, y + height - curveRad + curveRad / 2);
			glVertex2d(x, y + height - curveRad );
		glEnd();

		//Right pane
		glBegin(GL_QUADS);
			glVertex2d(x + width - curveRad, y);
			glVertex2d(x + width, y);
			glVertex2d(x + width, y + height - curveRad );
			glVertex2d(x + width - curveRad, y + height - curveRad);

			glVertex2d(x + curveRad, y);
			glVertex2d(x + width - curveRad, y);
			glVertex2d(x + width - curveRad, y + height - curveRad);
			glVertex2d(x + curveRad, y + height - curveRad);
		glEnd();
	
		//Make left bottom window arc
		glBegin(GL_POLYGON);
			glVertex2d(x + width - curveRad, y + height - curveRad );
			glVertex2d(x + width, y + height - curveRad );
			glVertex2d(x + width - curveRad/6, y + height - curveRad /2);
			glVertex2d(x + width - curveRad + curveRad / 2, y + height - curveRad / 6);
			glVertex2d(x + width - curveRad, y + height);
			glVertex2d(x + width, y + height - curveRad );		
		glEnd();


		//TitleBar.
		glBegin(GL_QUADS);
			glColor3ub(255,255,255);
			glVertex2d(x + curveRad + 20, y  + 1);
			glColor3ub(255,255,255);
			glVertex2d(x + width - curveRad - 20, y + 1);
			glColor3ub(100,100,100);
			glVertex2d(x + width - curveRad - 20, y + 20);
			glColor3ub(100,100,100);
			glVertex2d(x + curveRad + 20, y + 20);
		glEnd();

		//Right toolArch
		glBegin(GL_POLYGON);
			glColor3ub(255,255,255);
			glVertex2d(x + width - curveRad - 20, y + 1);
			glColor3ub(100,100,100);
			glVertex2d(x + width - curveRad , y + 1);
			glVertex2d(x + width - curveRad  - 3, y + 1 + 12);
			glVertex2d(x + width - curveRad  - 8 , y + 1 + 17);
			glVertex2d(x + width - curveRad - 20, y + 20);

			glColor3ub(255,255,255);
			glVertex2d(x + width - curveRad - 20, y + 1);
		glEnd();

	
		//LeftTool Arch
	
		glBegin(GL_POLYGON);
			glColor3ub(255,255,255);
			glVertex2d(x + curveRad + 20, y + 1);

			glColor3ub(100,100,100);
			glVertex2d(x + curveRad + 20, y + 20);
	
			glColor3ub(100,100,100);
			glVertex2d(x + curveRad + 8, y + 17);
			glVertex2d(x + curveRad + 3, y + 12);

			glColor3ub(100,100,100);
			glVertex2d(x + curveRad, y + 1);
			glVertex2d(x + curveRad + 20, y + 1);

		glEnd();

		glColor3ub(UserInterface::textColor.r, UserInterface::textColor.g, UserInterface::textColor.b);

		int baseWidth = width - 2 * curveRad;
		
		glRasterPos2i( (GLint) (x + curveRad + ( baseWidth - strlen(caption.getCString()) * 8 ) / 2 ), (GLint) (y + 16 ));
		glListBase(fontID);
		glCallLists(caption.getLength(), GL_UNSIGNED_BYTE, caption.getCString());

		int textSet = 0;

		if( message.getSize() > 0 && ( type == WindowType::Message || type == WindowType::Confirm) )
		{
			for ( int i = 0; i < message.getSize(); i++ )
			{
				glRasterPos2i( (GLint)( x + 10), (GLint) (y + height / 2 ) + textSet * 16 );
				glListBase(fontID);
				glCallLists(message[i].getLength(), GL_UNSIGNED_BYTE, message[i].getCString());
				textSet++;
			}
		}

		for ( int zz = 0; zz < 5; zz++ )
		{
			if ( lists[zz] )
			{
				lists[zz]->DisplayListBox();
			}
		}

		//buttons
		for ( int j = 0; j < 300; j++ )
		{
			if ( buttons[j] )
			{
				buttons[j]->DisplayButton();
			}
		}

		for ( int k = 0; k < 300; k++ )
		{
			if ( textBoxes[k] )
			{
				textBoxes[k]->DisplayTextBox();
			}
		}
	
		for ( int l = 0; l < 300; l++ )
		{
			if ( sliders[l] )
			{
				sliders[l]->DisplaySlider();
			}
		}

	}
	glPopMatrix();
}

void Window3d::DrawShadow(unsigned char red, unsigned char blue, unsigned char green )
{
	glColor4ub(red, blue, green, 100);

	//left edge )
	glBegin(GL_QUADS);
						
		glVertex2d(x - 1, y - 1);
		glVertex2d(x + curveRad, y - 1);
		glVertex2d(x + curveRad, y + height - curveRad );
		glVertex2d(x - 1 , y + height - curveRad);
		
		//Bottom Shadow
		glVertex2d(x + curveRad, y + height - curveRad);
		glVertex2d(x + width - curveRad, y + height - curveRad);
		glVertex2d(x + width - curveRad, y + height + 1 );
		glVertex2d(x + curveRad , y + height + 1);
	glEnd();

	//Make left bottom window arc
	glBegin(GL_POLYGON);
		glVertex2d(x - 1, y + height - curveRad );
		glVertex2d(x + curveRad, y + height - curveRad );
		glVertex2d(x + curveRad, y + height + 1 );
		glVertex2d(x + curveRad - curveRad/2 - 1, y + height - curveRad /6 + 1);
		glVertex2d(x + curveRad / 6 - 1, y + height - curveRad + curveRad / 2 + 1);
		glVertex2d(x - 1, y + height - curveRad );
	glEnd();


	//Right pane shadow
	glBegin(GL_QUADS);
		glVertex2d(x + width - curveRad, y - 1);
		glVertex2d(x + width + 1, y - 1);
		glVertex2d(x + width + 1, y + height - curveRad );
		glVertex2d(x + width - curveRad, y + height - curveRad);

		//top shade
		glVertex2d(x + curveRad, y - 1);
		glVertex2d(x + width - curveRad, y - 1);
		glVertex2d(x + width - curveRad, y + height - curveRad);
		glVertex2d(x + curveRad, y + height - curveRad);
	glEnd();

	//Make left bottom Shadow arc
	glBegin(GL_POLYGON);
		glVertex2d(x + width - curveRad, y + height - curveRad );
		glVertex2d(x + width + 1, y + height - curveRad );
		glVertex2d(x + width - curveRad/6 + 1, y + height - curveRad /2 + 1);
		glVertex2d(x + width - curveRad + curveRad / 2 + 1, y + height - curveRad / 6 + 1);
		glVertex2d(x + width - curveRad, y + height + 1);
		glVertex2d(x + width + 1, y + height - curveRad );		
	glEnd();


}

String Window3d::getFileSelect()
{
	if ( lists[0] )
		return lists[0]->items[lists[0]->getSelectedIndex()];
	else 
		return String("");
}

void Window3d::setMsg(String msg) 
{ 	
	if ( msg.getLength() > 0)
	{
		message = List<String>();

		int firstLine = msg.FindChar('\n');
		while ( firstLine >= 1 )
		{
			firstLine = msg.FindChar('\n');
			if ( firstLine == -1 )
				break;
			message.PushBack(msg.Substring(0,firstLine - 1));
			msg.TrimFront(firstLine + 1);
		}

		if (firstLine == -1 )
			firstLine = 0;

		message.PushBack(msg.Substring(firstLine, msg.getLength() - 1 ));

	}
}

void Window3d::selectNextBox()
{

	int count = 0;

	for(;;)
	{
		textFocus += 1;
		textFocus %= 300;
		count++;

		if ( count > 400)
		{
			break;
		}

		if ( textBoxes[textFocus] != NULL)
		{
			if ( !textBoxes[textFocus]->isLocked())
			{
				break;
			}
		}
	}

	textBoxes[textFocus]->MoveCursorTo(0);

}

void Window3d::selectPreviousBox()
{

	int count = 0;

	for(;;)
	{
		textFocus -= 1;
		textFocus = ((textFocus < 0) ? 299 : textFocus);
		
		count++;

		if ( count > 400)
		{
			break;
		}

		if ( textBoxes[textFocus] != NULL)
		{
			if ( !textBoxes[textFocus]->isLocked())
			{
				break;
			}
		}
	}
	textBoxes[textFocus]->MoveCursorTo(0);
}


void Window3d::passNumberToBox(char letter)
{
	if ( letter == 27 )
	{
		if ( cancelButton )
		{
			cancelButton->OnClick();
			closeButton->OnClick();
		}
	}
	else if ( letter == 13 )
	{
		if ( applyButton )
		{
			applyButton->OnClick();
			//applyButton->ActionPerformed(this);
		}
		else if ( okButton )
		{
			okButton->OnClick();
			//okButton->ActionPerformed(this);
		}
		
	}
	else if ( textBoxes[textFocus] )
	{
		textBoxes[textFocus]->insertLetter(letter);
	}
}


// Accessors:
int Window3d::getTextID() { return textFocus; }
int Window3d::getID() { return id; }
int Window3d::getX() { return x; }
int Window3d::getY() { return y; }
int Window3d::getWidth() { return width; }
int Window3d::getHeight() { return height; }
int Window3d::getCurveRad() { return curveRad; }
Window3d::WindowType Window3d::getType(){  return type; }

//Mutators:
void Window3d::setPos(int x, int y){ this->x = x; this->y = y; }
void Window3d::setHeight(int h){ this->height = h; this->curveRad = (width / 10 + height / 10) / 2 ;if ( closeButton) { this->closeButton->setPos(width - curveRad + curveRad / 6, 0) ;} }
void Window3d::setWidth(int w){ this->width = w; this->curveRad = (width / 10 + height / 10) / 2 ;if ( closeButton) { this->closeButton->setPos(width - curveRad + curveRad / 6, 0); } }
void Window3d::setTextID(int id) { textFocus = id; }
void Window3d::setCaption(String caption){ this->caption = caption; }
void Window3d::setType(Window3d::WindowType wType){ type = wType; }
