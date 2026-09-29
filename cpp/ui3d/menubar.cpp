//MenuBar and Menu. Written and commented by Mark Tulewicz
//VRUPL PRoject Griffen == Holodeck Project.
#include "userinterface.h"
#include "menubar.h"
#include "wtime.h"

//Size stuff.
int MenuBar::MenuWidth = 128;
int MenuBar::MenuHeight = 20;


/***************************************************************
		MENU!!!!!!!!!  This is the class for each little menu
****************************************************************/

Menu::Menu(String name, int x, int y)
{
	//Make the menu, give it a name, and position it accordingly and by default, do nothing on click.
	this->menuChecked = false;
	caption = name;
	this->x = x;
	this->y = y;
	this->enabled = true;
	this->AssignActionEvent(EventHandlers::DefaultMenuEvent);
}

void Menu::DisplayItem(int menuAcross, int menuDown, int itemSelect, int width)
{
	this-> width = width;
	//Draw the menu.
	glBegin(GL_QUADS);
	{
		//Selected menu is a different color, the upper and lower are for Gradient same as MenuBar display.
		if ( menuDown == itemSelect )
			glColor3ub(UserInterface::menuBorderColorUpperOn.r, UserInterface::menuBorderColorUpperOn.g, UserInterface::menuBorderColorUpperOn.b);
		else
			glColor3ub(UserInterface::menuBorderColorUpperOn.r / 2, UserInterface::menuBorderColorUpperOn.g / 2, UserInterface::menuBorderColorUpperOn.b / 2);
		if ( !enabled)
		{
			glColor3ub(UserInterface::menuBorderColorUpperOff.r, UserInterface::menuBorderColorUpperOff.g, UserInterface::menuBorderColorUpperOff.b);
		}

		//X and Y are zero, so really you don't need them, since the MenuAcross is more important.
		glVertex2i(menuAcross * MenuBar::MenuWidth + x, menuDown * MenuBar::MenuHeight + MenuBar::MenuHeight + y);
		glVertex2i(menuAcross * MenuBar::MenuWidth + width + x, menuDown * MenuBar::MenuHeight + MenuBar::MenuHeight + y);
		
		if ( menuDown == itemSelect )
			glColor3ub(UserInterface::menuBorderColorLowerOn.r, UserInterface::menuBorderColorLowerOn.g, UserInterface::menuBorderColorLowerOn.b);
		else
			glColor3ub(UserInterface::menuBorderColorLowerOn.r / 2, UserInterface::menuBorderColorLowerOn.g / 2, UserInterface::menuBorderColorLowerOn.b / 2);
		
		if ( !enabled )
		{
			glColor3ub(UserInterface::menuBorderColorLowerOff.r / 2, UserInterface::menuBorderColorLowerOff.g / 2, UserInterface::menuBorderColorLowerOff.b / 2);
		}
		
		//Times two, since we ant to go down that much and account for the top menu.
		glVertex2i(menuAcross * MenuBar::MenuWidth +  width + x, menuDown * MenuBar::MenuHeight + MenuBar::MenuHeight * 2 + y);
		glVertex2i(menuAcross * MenuBar::MenuWidth + x, menuDown * MenuBar::MenuHeight + MenuBar::MenuHeight * 2 + y);
	}
	glEnd();
	
	if ( menuChecked )
	{
		glColor3ub(0, 255, 0);
		glBegin( GL_QUADS );
		{
			glVertex2i ( menuAcross * MenuBar::MenuWidth + x + 2, menuDown * MenuBar::MenuHeight + MenuBar::MenuHeight + y + 6);
			glVertex2i ( menuAcross * MenuBar::MenuWidth + x + 10, menuDown * MenuBar::MenuHeight + MenuBar::MenuHeight + y + 6);
			glVertex2i ( menuAcross * MenuBar::MenuWidth + x + 10, menuDown * MenuBar::MenuHeight + MenuBar::MenuHeight + y + 14);
			glVertex2i ( menuAcross * MenuBar::MenuWidth + x + 2, menuDown * MenuBar::MenuHeight + MenuBar::MenuHeight + y + 14);
		}
		glEnd();
	}


	if ( menuDown == itemSelect )
	{
		glColor3ub(UserInterface::menuSelectedTextColor.r, UserInterface::menuSelectedTextColor.g, UserInterface::menuSelectedTextColor.b);
	}
	else
	{
		glColor3ub(UserInterface::menuTextColor.r, UserInterface::menuTextColor.g, UserInterface::menuTextColor.b);	
	}

	if ( !enabled )
			glColor3ub(60,60,60);

	UserInterface::PrintGL( menuAcross * MenuBar::MenuWidth + 10 + x, menuDown * MenuBar::MenuHeight + MenuBar::MenuHeight + 14 + y, caption);
}

//Event handling menus
void Menu::AssignActionEvent(MenuEvent target)
{
	this->Function = target;
}

//Toggle check! 
bool Menu::ToggleCheckMark()
{
	if ( this->menuChecked )
	{
		this->menuChecked = false;
	}
	else
	{
		this->menuChecked = true;
	}
	return menuChecked;
}

//Function pointing event.
void Menu::GenerateClick()
{
	(UserInterface::eventHandler.*Function)(this);
	return;
}

//Access Mutate
bool Menu::isEnabled() { return enabled; }
void Menu::Disable() { enabled = false; }
void Menu::Enabled() { enabled = true; }



/***********************************************
					MENU BAR
	  Holds and manages the individual menus.
***********************************************/

Menu * MenuBar::FindMenu(String caption)
{
	//Look thru all menus to find the one we want
	for (int i = 0; i < this->menuRight.getSize(); i++ )
	{
		for ( int j = 0; j < this->menuRight[i].menuDown.getSize(); j++)
		{
			if ( menuRight[i].menuDown[j].caption == caption )
			{
				return &menuRight[i].menuDown[j];
			}
		}
	}
	return NULL;
}

MenuBar::MenuBar()
{
	//Main menu constructor
	this->wireToggle = NULL;
	this->itemSelect = -1;
	this->menuSelect = -1;
	x = 0;
	y = 0;
	state = MenuBar::Min;
	drawPlace = 70;
}

MenuBar::MenuBar(int x, int y )
{
	//Context Menu Construction.
	this->itemSelect = -1;
	this->menuSelect = -1;
	this->x = x;
	this->y = y;
	state = MenuBar::Engaging;
	drawPlace = 0;	
}

Menu * MenuBar::addMenuDown(String itemName, int menuAcross, bool enabled)
{
	//Once a menu across has been made, this will add a submenu to that column.
	//Will return the menu that was made so you can assign it an EventHandler.
	Menu menu(itemName, x, y);
	menu.active = enabled;
	if ( menuAcross < 0 || menuAcross >= menuRight.getSize() )
	{
		return NULL;
	}
	menuRight[menuAcross].menuDown.PushBack(menu);
	return const_cast<Menu*>(&(menuRight[menuAcross].menuDown.getBack()));
}

void MenuBar::addMenuAcross(String itemName)
{
	//Add a menu across.
	Menu menu(itemName, x, y);
	this->menuRight.PushBack(menu);
}


bool MenuBar::ClickMenu(Cursor3d & mouse)
{
	//Check to see if the menu was minimized and a click event has occured
	if ( state == MenuBar::Min && mouse.onClick[0] )
	{
		if ( mouse.getX() >= x + 0 && mouse.getX() < 70 + x &&
				mouse.getY() >= 0 + y && mouse.getY() < 20 - y)
		{
			state = MenuBar::Engaging;
			return true;
		}
	}
	//Check to see if the menu is already on.
	else if ( state == MenuBar::On)
	{
		static bool onMenu = false;

		//This occurs if you have clicked off the menu with any button we close the menu
		if ( !onMenu && (mouse.onClick[0] || mouse.onClick[1]  || mouse.onClick[2] ) )
		{
			menuSelect = -1;
			itemSelect = -1;
			state = MenuBar::Retracting;
			return false;
		}
		//You have not clicked yet, you are moving the mouse around
		else
		{
			//The mouse is on one of the menu items, so select it, and make it known that we are on the menu.
			if ( mouse.getX() >= menuSelect * MenuBar::MenuWidth + x && mouse.getX() < menuSelect * MenuBar::MenuWidth + MenuBar::MenuWidth + x &&
					mouse.getY() >= 0 + y && mouse.getY() < MenuBar::MenuHeight * menuRight[menuSelect].menuDown.getSize() + MenuBar::MenuHeight + y)
			{
				onMenu = true;
				itemSelect = (mouse.getY() - y) / MenuBar::MenuHeight - 1;
			}
			//The mouse is not on any menus, so mark it as off the menu.
			else
			{
				itemSelect = -1;
				onMenu = false;
			}
		}
		
		//Check to see which menu we are over.
		for ( int i = 0; i < menuRight.getSize(); i++)
		{
			if ( mouse.getX() >= i * MenuBar::MenuWidth + x && mouse.getX() < i * MenuBar::MenuWidth + MenuBar::MenuWidth + x &&
					mouse.getY() >= 0 + y && mouse.getY() < MenuBar::MenuHeight + y )
			{
				menuSelect = i;
				onMenu = true;
			}
		}

		//Check if clicked with the left button, and you are on a valid item. Close menu
		if ( mouse.onRelease[0] && menuSelect > -1 && menuSelect < menuRight.getSize() && itemSelect > -1)
		{
			state = MenuBar::Retracting;
			menuRight[menuSelect].menuDown[itemSelect].GenerateClick();
			menuSelect = -1;
			itemSelect = -1;
		}

		return true;
	}
	else if ( state == MenuBar::Engaging )
	{
		//This will grow the menu to the right by a scaled time about.
		//The only menu at 0,0 is the main menu, the other is the context menu.
		if ( (x >= 0) && (y >= 0) )
		{
			drawPlace += static_cast<int>(512.0 * menuRight.getSize() * Time::getInterval() * 2.5f);
		}
		//Context open slower.
		else
		{
			drawPlace += static_cast<int>(512.0 * menuRight.getSize() * Time::getInterval());
		}

		//In the event that we have fully drawn the menubar, switch to the on state.
		if (drawPlace >= MenuBar::MenuWidth * menuRight.getSize())
		{
			drawPlace = MenuBar::MenuWidth * menuRight.getSize();
			state = MenuBar::On;
		}
		return false;
	}
	else if ( state == MenuBar::Retracting )
	{
		/* Same as above but subtracting */
		if ( (x >= 0) && (y >= 0) )
		{
			drawPlace -= static_cast<int>(512.0 * menuRight.getSize() * Time::getInterval() * 2.5f);
		}
		else
		{
			drawPlace -= static_cast<int>(512.0 * menuRight.getSize() * Time::getInterval());
		}

		if (drawPlace <= 0)
		{
			drawPlace = 0;
			if ( x == 0 && y == 0 )
			{
				//the main menu becomes "menu" this is known as minimized mode.
				state = MenuBar::Min;
			}
			else
			{
				//Context menus disappear and turn OFF.
				state = MenuBar::Off;
			}
		}
		return false;
	}
	return false;
}

void MenuBar::DisplayMenu()
{
	//Start OGL Drawing.
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	{
		glLoadIdentity();

		//Code::
		if ( state == MenuBar::On )
		{
			//Draw the menus across.
			for ( int i = 0; i < menuRight.getSize(); i++)
			{
				glBegin(GL_QUADS);
				{
					if ( i == menuSelect )
					{
						//Selected menu uses colors defined in UserInterface setMenuBorderColorUpper / Lower.					
						glColor3ub(UserInterface::menuBorderColorUpperOn.r, UserInterface::menuBorderColorUpperOn.g, UserInterface::menuBorderColorUpperOn.b);
					}
					else
					{
						//All unselected menus.
						glColor3ub(UserInterface::menuBorderColorUpperOn.r / 2, UserInterface::menuBorderColorUpperOn.g / 2, UserInterface::menuBorderColorUpperOn.b / 2);
					}

					glVertex2i(i * MenuBar::MenuWidth + x, 0 + y);
					glVertex2i(i * MenuBar::MenuWidth + MenuBar::MenuWidth + x, 0 + y);
					
					//Same as above, but the lower colors used for gradient effect.
					if ( i == menuSelect )
					{
						glColor3ub(UserInterface::menuBorderColorLowerOn.r, UserInterface::menuBorderColorLowerOn.g, UserInterface::menuBorderColorLowerOn.b);
					}
					else
					{
						glColor3ub(UserInterface::menuBorderColorLowerOn.r / 2, UserInterface::menuBorderColorLowerOn.g / 2, UserInterface::menuBorderColorLowerOn.b / 2);
					}
	
					glVertex2i(i * MenuBar::MenuWidth + MenuBar::MenuWidth + x, MenuBar::MenuHeight + y);
					glVertex2i(i * MenuBar::MenuWidth + x, MenuBar::MenuHeight + y);
					glEnd();
				}

				//Text color same as above.
				if ( i == menuSelect )
				{
					glColor3ub(UserInterface::menuSelectedTextColor.r, UserInterface::menuSelectedTextColor.g, UserInterface::menuSelectedTextColor.b);
				}
				else
				{
					glColor3ub(UserInterface::menuTextColor.r, UserInterface::menuTextColor.g, UserInterface::menuTextColor.b);	
				}

				UserInterface::PrintGL( i * MenuBar::MenuWidth + 10 + x, 14 + y, menuRight[i].caption);
			}	//End across menus

				
			//Draw menus Downward if valid menu selection.
			if ( menuSelect > -1 && menuSelect < menuRight.getSize() )
			{
				/* This uses a two pass draw, first pass it goes across and finds the longest menu based
					on the length of its caption. Next it goes across drawing all menus the same size across,
					thus giving it a consistant size feeling.	*/
				int newWidth = MenuBar::MenuWidth;
								
				//First pass
				for ( int j = 0; j < menuRight[menuSelect].menuDown.getSize(); j++)
				{
					//find the max width.
					if ( ( ( menuRight[menuSelect].menuDown[j].caption.getLength() + 2) * 11) > newWidth )
					{
						newWidth = (menuRight[menuSelect].menuDown[j].caption.getLength() + 2 )* 11;
					}
				}

				/*	Draw each menu individually, they must know if they are selected, how
					far over they are, and how far down the list they are so they can be drawn
					in the proper location.*/
				for ( int j = 0; j < menuRight[menuSelect].menuDown.getSize(); j++)
				{
					menuRight[menuSelect].menuDown[j].DisplayItem(menuSelect,j,itemSelect, newWidth);
				}
			}
		}
		//Draw the menus as they are either retracting or expanding.
		else if ( state == MenuBar::Retracting || state == MenuBar::Engaging )
		{
			glBegin(GL_QUADS);
			{
				glColor3ub(UserInterface::menuBorderColorUpperOn.r, UserInterface::menuBorderColorUpperOn.g, UserInterface::menuBorderColorUpperOn.b);
				glVertex2i(0 + x, 0 + y);
				glVertex2i(drawPlace + x, 0 + y);
				
				glColor3ub(UserInterface::menuBorderColorLowerOn.r, UserInterface::menuBorderColorLowerOn.g, UserInterface::menuBorderColorLowerOn.b);
				glVertex2i(drawPlace + x, MenuBar::MenuHeight + y);
				glVertex2i(0 + x, MenuBar::MenuHeight + y);
				glEnd();
			}			
		}

		//Just the little square at the upper left.
		else if ( state == MenuBar::Min )
		{			
			glBegin(GL_QUADS);
			{
				glColor3ub(UserInterface::menuBorderColorUpperOn.r, UserInterface::menuBorderColorUpperOn.g, UserInterface::menuBorderColorUpperOn.b);
				glVertex2i(0 + x, 0 + y);
				glVertex2i(65 + x, 0 + y);
				
				glColor3ub(UserInterface::menuBorderColorLowerOn.r, UserInterface::menuBorderColorLowerOn.g, UserInterface::menuBorderColorLowerOn.b);
				glVertex2i(65 + x, MenuBar::MenuHeight + y);
				glVertex2i(0 + x, MenuBar::MenuHeight + y);
				glEnd();
			}		

			glColor3ub(UserInterface::menuTextColor.r, UserInterface::menuTextColor.g, UserInterface::menuTextColor.b);					
			UserInterface::PrintGL(10,14,String("Menu"));
		}

		//Close the matrix stack off and return to normal view.
	}
	glPopMatrix();
}

MenuBar::MenuState MenuBar::getMenuState()
{
	return state;
}

bool MenuBar::GetCheckState(String menuCaption)
{
	//Check if a menu with the caption passed in, is checked or not.
	for (int i = 0; i < this->menuRight.getSize(); i++ )
	{
		for ( int j = 0; j < this->menuRight[i].menuDown.getSize(); j++)
		{
			if ( menuRight[i].menuDown[j].caption == menuCaption )
			{
				return menuRight[i].menuDown[j].menuChecked;
			}
		}
	}
	return false;
}

bool MenuBar::ToggleCheckMark(String menuCaption)
{
	//if a check mark is on, switch it, return the final value of the check mark
	for (int i = 0; i < this->menuRight.getSize(); i++ )
	{
		for ( int j = 0; j < this->menuRight[i].menuDown.getSize(); j++)
		{
			if ( menuRight[i].menuDown[j].caption == menuCaption )
			{
				if ( menuRight[i].menuDown[j].menuChecked == true )
				{
					menuRight[i].menuDown[j].menuChecked = false;
					return false;
				}
				else
				{
					menuRight[i].menuDown[j].menuChecked = true;
					return true;
				}
			}
		}
	}
	return false;
}

void MenuBar::CheckMark(String menuCaption)
{
	//Sets a check mark as on.
	for (int i = 0; i < this->menuRight.getSize(); i++ )
	{
		for ( int j = 0; j < this->menuRight[i].menuDown.getSize(); j++)
		{
			if ( menuRight[i].menuDown[j].caption == menuCaption )
			{
				menuRight[i].menuDown[j].menuChecked = true;
			}
		}
	}
}

void MenuBar::UnCheck(String menuCaption)
{
	//sets a check mark as off.
	for (int i = 0; i < this->menuRight.getSize(); i++ )
	{
		for ( int j = 0; j < this->menuRight[i].menuDown.getSize(); j++)
		{
			if ( menuRight[i].menuDown[j].caption == menuCaption )
			{
				menuRight[i].menuDown[j].menuChecked = false;
			}
		}
	}
}


//Accessors and mutators.
bool Menu::isChecked(){ return this->menuChecked; }
void Menu::setCheckMark(bool state){ this->menuChecked = state; }
void Menu::setCaption(String cap){ caption = cap; }
String Menu::getCaption() { return caption; }
