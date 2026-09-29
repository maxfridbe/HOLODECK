#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

class Button3d;
class Menu;

class EventHandlers
{
	public:
		struct ForceInfo
		{
			double mass;
			double x;
			double y;
			double z;
			double duration;
		};
	public:
		EventHandlers();
		~EventHandlers();

		//Add Event controls here.
		//To implement when you create a button ( usually in window3d.cpp's constructor
		//you add the line button->AssignActionEvent(EventHandlers::FunctionBelow);
		//When that button is clicked, the function you define below, is the one that is executed.
		//You will have the button that sent the OnClick(), which has a Parent who is the containing window.
		//Use button->getParent() for the window containing the button.
		
		void DebugClick(Button3d * button);
		
		void DefaultCloseWindow(Button3d * button);
		void AddForce(Button3d * button);
		void RemoveForce(Button3d * button);
		void ApplyTransform(Button3d * button);
		void GridSettings(Button3d * button);
		void LoadScene(Button3d * button);
		void LoadModel(Button3d * button);
		void NetConnect(Button3d * button);
		void SaveScene(Button3d * button);
		void ScrollUp(Button3d * button);
		void ScrollDown(Button3d * button);
		void UnloadModel(Button3d * button);
		void UnloadScene(Button3d * button);
		void QuitProgram(Button3d * button);
		void AddWorldForce(Button3d * button);

		//MenuEvents defined in MenuHandler.cpp
		//Menu Operations
		
		void DefaultMenuEvent(Menu * menu);
		void LoadScene(Menu * menu);
		void SaveScene(Menu * menu);
		void LoadModel(Menu * menu);
		void UnloadModel(Menu * menu);
		void NewScene(Menu * menu);
		void Connect(Menu * menu);
		void Quit(Menu * menu);

		//Menu Object Settings
		void ObjectProperties(Menu * menu);
		void RotateObject(Menu * menu);
		void TranslateObject(Menu * menu);
		void ScaleObject(Menu * menu);
		void DuplicateSelectedObject(Menu * menu);
		void DeselectSelectedObject(Menu * menu);

		//Menu Settings
		void ToggleInnerGrid(Menu * menu);
		void ToggleHoloGrid(Menu * menu);
		void SetWireframeMode(Menu * menu);
		void SetTexturedMode(Menu * menu);
		void ColorSettings(Menu * menu);

		//View settings
		void SetView3d(Menu * menu);
		void SetViewFront(Menu * menu);
		void SetViewBack(Menu * menu);
		void SetViewLeft(Menu * menu);
		void SetViewRight(Menu * menu);
		void SetViewTop(Menu * menu);
		void SetViewBottom(Menu * menu);

		void LookFromCamera(Menu * menu);
		void ControlCamera(Menu * menu);

		//Physics events:
		void ModifyForces(Menu * menu);
		void AddWorldForcesMenu(Menu * menu);

};

typedef void (EventHandlers::*MenuEvent)(Menu * menu);
typedef void (EventHandlers::*ButtonEvent)(Button3d * button);

#endif
