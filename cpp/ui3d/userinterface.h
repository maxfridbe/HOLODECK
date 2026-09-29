#ifndef USERINTERFACE_H
#define USERINTERFACE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

#include "wstring.h"
#include "wlist.h"
#include <vector>
#include <time.h>

using namespace std;

class Window3d;

#include "cursor3d.h"
#include "textbox3d.h"
#include "button3d.h"
#include "Window3d.h"
#include "menubar.h"
#include "slider3d.h"

//	<summary> The UI Class ( UIC ) is a wrapper for the above
//			classes. It holds the collection and performs
//			the operations to all windows for you. Allowing the
//			user to simply call its commands for it to responde
//			accordingly and automatically performs all checks.
//			Written by Mark Tulewicz UserInterface Class
//			Required functions for funcationality:
//			
//			UserInterface::Init(handle to the window ( used to make fonts ) );
//
//			In a openGL display function:
//				
//				void DisplayWindows();
//				CheckForMessages( Msg & Event);
//			
//			For mouse support:
//				UserInterface::PerformMouseEvents(relX, relY, MouseWheel change, x, y, buttons);
//		    
//			For keyboard Support:
//				void MoveTextCursor(int amount); ( usually on the left and right arrows by 1 or -1 );
//				UserInterface::sendKeyPress(key, bool shiftHeld);
//				Note passing tab, enter, esc, or backspace, will activate the function of those keys.
//	</summary>
//	<remarks>Functions to edit to extend functionality: 
//				AddMenuLayout1, AddxXxWindow(), sendKeyPress.
//	</remarks>
class UserInterface
{
	friend class Window3d;
	friend class MenuBar;
	friend class Menu;
	public:
		class UIColor
		{
		public:
			UIColor();
			UIColor(unsigned char red, unsigned char green, unsigned char blue);
			unsigned char r;
			unsigned char g;
			unsigned char b;
			enum Colors
			{
				Red,
				Green,
				Blue, 
				Yellow,
				Cyan,
				Purple,
				Orange,
				White,
				Black,
				Custom,
			};
		};

		//<summary>Initialize the rendering context and window</summary>
		static void Init(HWND);
		//<summary>Destructor used to clean up ram.</summary>
		static void Release();

		//<summary>Command is all internal proccess commands I will need to know for internal work.</summary>
		enum Command 
		{ 
			RequestExit,
			RequestWorldForces,
			RequestNew , 
			RequestUnloadModel, 
            RequestUnloadScene, 
			RequestLoadModel, 
			RequestLoadScene, 
			RequestSaveScene, 
			RequestEditObject,
			RequestGridSettings,
			None,
		};
			
		//<summary>Operations are all external info. For example, telling the parser to load a model, or open the scene. Please only use this as the Msg::message enum.</summary>
		enum Operation 
		{ 
			AddForce,
			AddWorldForce,
			RemoveForce,
			WindowClosed,
			ControlCamera,
			ViewCamera,
			SendWorldForces,
			ConnectionRequest,
			ModelLoadReady, 
			ModelUnloadReady, 
			ModelModReady, 
			SceneLoadReady, 
			SceneSaveReady,
			SceneUnloadReady,
			SetManipState,
			RequestModelInfo,
			SetFOV,
			SetView,
			EnableInnerGrid,
			DisableInnerGrid,
			EnableHoloGrid,
			DisableHoloGrid,
			CloneObject,
			GotoWireframe,
			GotoTextured,
			SelectGridColorScheme,
			Deselect,
			Exit,
			NoOp,
		};

		//<summary> Current command issued</summary>
		static Command command;

		//<summary>Structure used to pass messages.</summary>
		struct Msg
		{
			Msg();
			Msg(Operation message, void * dataInfo, String specialString = "");
			Msg(const Msg & source);
			const Msg & operator = (const Msg & rhs);
			~Msg();

			//<summary>Which event is occuring</summary>
			Operation message;
			//<summary>String that can hold character data for passing into and out of the class.</summary>
			String specialString;
			//<summary>Void pointer can be used to represent anything!</summary>
			void * dataInfo;
		};

		/*Window type creations*/

		//<summary>Creates a window which can access the file system.</summary>
		//<remarks>This function is adaptive, and will create the most appropriate
		//window type for the command issued.  E.g. Save will have the save file features in it
		//while open will have the open file features in it.</remarks>
		//<param name='title'>Window Title at the top.</param>
		static void AddOpenWindow(char * title);

		//<summary>Creates a window that will generate an event asking for all information about
		// the object which is currently selected.</summary>
		//<remarks>This function does not have safeties for invalid selected objects. 
		// It is the reponsibility of the class who catches the event to account for that.</remarks>
		static void AddObjPropWindow();

		//<summary>Creates a window that stores all the world settings. Fog, FOV, lighting, etc.</summary>
		static void AddWorldPropWindow();

		static Window3d * AddVideoWindow(int x, int y, int w, int h);

		static Window3d * AddObjectPhysics();
		static Window3d * AddCameraWindow();
		static Window3d * AddWorldForceWindow();
		static Window3d * AddNetConnectWindow(char * title, char * defIP, char * defPort);

		//<summary>Sends a Vector of strings of all world forces available</summary>
		static void SendForceList( vector < String > forces );

		//<summary>Used to answer any questions that may be asked which require an accept or decline</summary>
		//<param name='title'>Window Title name on bar.</param>
		//<param name='msg'>Message to be displayed in the text, '\n' is valid for new line.</param>
		static Window3d * AddConfirmWindow(char * title, char * msg, char * acceptButtonText = "Ok", char * cancelText = "Cancel");

		//<summary>Used to display any information. </summary>
		//<param name='title'>Window Title name on bar.</param>
		//<param name='msg'>Message to be displayed in the text, '\n' is valid for new line.</param>
		static void AddMessageBox(char * Title, char * msg);
		
		//<summary>Adds the default menu bar to the top. This should be customized to meet
		// the requirements of whatever project this class is used for.</summary>
		static void AddMenuLayout1();

		//<summary>Adds the default context menu bar upon right click. This should be customized to meet
		// the requirements of whatever project this class is used for.</summary>
		//<remarks>This class does not check mouse status. The calling class must check which mouse button
		// had been clicked on</remarks>
		static MenuBar * AddContextMenuObject();

		//<summary>Removes the window which currently holds the focus of the screen.</summary>
		static void KillFocalWindow();
	
		/*Display Functions*/
		//<summary>Draws all windows and places the focal window on top and the mouse last.</summary>
		//<remarks>This has its own matrix controls, so windows will float above all else.</remarks>
		static void DisplayWindows();
		static void DisplayMouse();

		//<summary> Will draw or not draw the mouse </summary>
		static void ShowMouse();
		static void HideMouse();

		static void SelectNextBox();
		static void SelectPrevBox();

		//<summary>Will turn on the main menu</summary>
		static void ToggleMainMenu();
		
		//Events:
		
		//<summary> Sends the float information about the model selected.</summary>
		//<remarks> Used in conjunction with the RequestModelInfo event</remarks>
		//<param name = 'rotate'>Array of rotation values</param>
		//<param name = 'scale'>Array of resize values</param>
		//<param name = 'trans'>Array of reposition values</param>
		static void SendModelInfo(const float * rotate,const float * scale,const float * trans, String & name, String & comment);

		//<summary> Sends the key in question into the window.  The window will then pass that in to its textboxes. </summary>
		//<remarks> This function should be modified to control which keys perform what actions.</remarks>
		static void sendKeyPress(char key, bool shift = false);
		
		//<summary> Add an event to the queue, this is the gatekeeper.  To keep the class independant,
		// all things must pass an event to send information in or out of the class. </summary>
		static void EventEnque(Msg event);

		//<summary> Will update the window if it is a focal message box type.
		// Things like FPS can be continously updated with this. </summary>
		//<param name='msg'>Window message to place.</param>
		static void UpdateMessageBox(char * msg);

		//<summary> Manages the event queue, This function MUST be called at some point outside the class.
		// The events made will be sent out by reference and allow the outside world to get information from
		// the class without knowing anything about the internal structure of the class. </summary>
		//<remarks> It is recommended that you use a switch on event.message to control events.</remarks>
		//<param name='event'>Event that will be loaded with the information of the event occuring.</param>
		static bool CheckForMessages( Msg & event );
		
		//<summary>Sends information to move the text cursor of a selected textbox.</summary>
		//<remarks>This function has built in safeties to prevent moving past the array</remarks>
		static void MoveTextCursor(int amount);

		//<summary> General purpose printing function. Will use the font and write the string in the position on screen.</summary>
		//<remarks> x is left to right, and y is bottom to top , orgin is bottom left </remarks>
		//<param name='x'>X position to write to.</param>
		//<param name='y'>Y position to write to.</param>
		//<param name='textToPrint'>String to write out.</param>
		static void PrintGL(int x, int y, String textToPrint);

		//<summary> General purpose printing function. Will use the font and write the string in the position on screen.</summary>
		//<remarks> x is left to right, and y is bottom to top , orgin is bottom left </remarks>
		//<param name='x'>X position to write to.</param>
		//<param name='y'>Y position to write to.</param>
		//<param name='z'>Z position to write to.</param>
		//<param name='textToPrint'>String to write out.</param>
		static void PrintGL(float x, float y, float z, String textToPrint);

		//<summary>Checks the state of the button click down.</summary>
		//<remarks>0 = left click, 1 = Right click, 2 = middle click.</remarks>
		//<param name = 'button'>Which button to check</param>
		//<returns>True if button is mouse button was just clicked, once per click.</returns>
		static bool OnMouseClick(int button = 0);

		//<summary>Checks the state of the button release.</summary>
		//<remarks>0 = left click, 1 = Right click, 2 = middle click </remarks.
		//<param name = 'button'>Which button to check</param>
		//<returns>True if button is mouse button was just released, once per release</returns>
		static bool OnMouseRelease(int button = 0);

		//<summary>Checks the state of the button.</summary>
		//<remarks>0 = left click, 1 = Right click, 2 = middle click </remarks.
		//<param name = 'button'>Which button to check</param>
		//<returns>True if button is mouse button is held down, Constant returns while down.</returns>
		static bool OnMouseDown(int button = 0);

		//<summary>Checks the state of the wheel on the mouse</summary>
		//<returns>Value of the wheel change, increments of 120 usually.</returns>
		static int OnMouseWheel();

		//Accessors:
		static void PerformMouseEvents(int relX, int relY, int relZ, int x, int y, unsigned char buttons[]);
		static bool MenuActive();
		static bool UserInterfaceActive();
		static int getWindowCount();
		static int getMouseX();
		static int getMouseY();
		static int getMouseOldX();
		static int getMouseOldY();
		static int getMouseRelX();
		static int getMouseRelY();
		
		static Menu * getMenuByCaption( String Caption );
		static MenuBar * getMenuBar();
		static void setSelectedObject(int x );

		static void setTextColor(unsigned char r, unsigned char g, unsigned char b);
		static void setFolderColor(unsigned char r, unsigned char g, unsigned char b);
		static void setWindowColor(unsigned char r, unsigned char g, unsigned char b);
		static void setMouseColor(unsigned char r, unsigned char g, unsigned char b);
		static void setWindowShadowColor(unsigned char r, unsigned char g, unsigned char b);
		static void setMouseShadowColor(unsigned char r, unsigned char g, unsigned char b);
		static void setMenuTextColor(unsigned char r, unsigned char g, unsigned char b);
		static void setMenuBorderColorUpperOn(unsigned char r, unsigned char g, unsigned char b);
		static void setMenuBorderColorLowerOn(unsigned char r, unsigned char g, unsigned char b);
		static void setMenuBorderColorUpperOff(unsigned char r, unsigned char g, unsigned char b);
		static void setMenuBorderColorLowerOff(unsigned char r, unsigned char g, unsigned char b);
		static void setMenuSelectedTextColor(unsigned char r, unsigned char g, unsigned char b);
		

		//Public variables
		static int WindowHeight;
		static int WindowWidth;
		//Colors
		static UserInterface::UIColor textColor;
		static UserInterface::UIColor folderColor;
		static UserInterface::UIColor windowColor;
		static UserInterface::UIColor mouseColor;
		static UserInterface::UIColor windowShadowColor;
		static UserInterface::UIColor mouseShadowColor;
		static UserInterface::UIColor menuBorderColorUpperOn;
		static UserInterface::UIColor menuBorderColorLowerOn;
		static UserInterface::UIColor menuBorderColorUpperOff;
		static UserInterface::UIColor menuBorderColorLowerOff;
		static UserInterface::UIColor menuTextColor;
		static UserInterface::UIColor menuSelectedTextColor;
		static EventHandlers eventHandler;


	private:
		//<summary>These variables control the color of the various elements.
		static UserInterface::UIColor getColorFromUIColor(UserInterface::UIColor::Colors);

		//<summary>Checks to see if a button has been clicked.</summary>
		//<remarks>This should be called after the mouse settings have been configured for this class 
		//or it will be one click behind.  Also, The order of detection is 
		//Menu, context, Focal window, any other window in ID order. Used in cases of overlap.</remarks>
		static void CheckButtonClick();
		
		//Controls:
		//<summary>Will reposition the focal window by current x - old x mouse coordinates.</summary>
		static void MoveWindow();


		//Accessors the user won't need.
		static void setMousePos(int x, int y);
		static void setMouseRelease(int button, bool setting);
		static void setMouseClick(int button, bool setting);
		static void setMouseDown(int button, bool setting);
		static void setMouseWheel(int amount);
		static void setRelX(int value);
		static void setRelY(int value);
		
		UserInterface();
		static List < Msg > msgQue;
		static Cursor3d mouse;
		static unsigned int fontID;
		static int windowCount;
		static Window3d * windows[101];
		static MenuBar * menu;
		static MenuBar * context;
		static int menuCount;
		static HGDIOBJ font;
		static HWND hWnd;

		// Non general but primitive info.
		static int selectedObject;
};

#endif