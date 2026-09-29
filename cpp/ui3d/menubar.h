//Written by Mark Tulewicz.
#ifndef MENUBAR_H
#define MENUBAR_H

#include "wlist.h"
#include "wstring.h"
#include "cursor3d.h"
#include "userinterface.h"

class Menu
{
	friend class MenuBar;
	public:
		Menu(String ItemName, int x, int y);
		
		//Function pointing
		void AssignActionEvent(MenuEvent function);
		void DisplayItem(int menuAcross, int menuDown, int itemSelect, int width);
		bool isEnabled();
		bool isChecked();
		void Disable();
		void Enabled();
		void GenerateClick();
		void setCaption(String caption);
		void setCheckMark(bool state);
		
		//Returns the final state of the menu checkmark
		bool ToggleCheckMark();
		String getCaption();
		
		bool menuChecked;
			
	private:
		void (EventHandlers::*Function)(Menu * menu);

		int x;
		int y;
		int width;
		bool active;
		int menuRankX;
		int menuRankY;
		String caption;
		List < Menu > menuDown;
		bool enabled;
};

class MenuBar
{
	friend class UserInterface;
	friend class Menu;
	public:
		MenuBar();
		MenuBar(int x, int y);
		enum MenuState { Off, On, Engaging, Retracting, Min };

		bool ToggleCheckMark(String menuCaption);
		void UnCheck(String menuCaption);
		void CheckMark(String menuCaption);
		bool GetCheckState(String menuCaption);

		MenuState getMenuState();
		Menu * FindMenu(String caption);
		void addMenuAcross(String itemName);
		Menu * addMenuDown(String itemName, int rank, bool enabled = true);
		Menu * wireToggle;
		bool ClickMenu(Cursor3d & mouse);
		void DisplayMenu();

	private:
		MenuState state;
		int x; 
		int y;
		int drawPlace;
		int menuSelect;
		int itemSelect;
		List < Menu > menuRight;
		static int MenuWidth;
		static int MenuHeight;
};

#endif
