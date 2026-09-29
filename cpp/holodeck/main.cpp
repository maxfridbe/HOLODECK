#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "controller.h"
#include "inputhandler.h"
#include "keyboarddelegate.h"
#include "display.h"
#include "object.h"
#include "view.h"
#include "console.h"
#include "modelmanip.h"
#include "matrix.h"
#include "wmath.h"
#include "client.h"
#include "header.h"
#include "utility.h"

/*#include "glovedelegate.h"*/
/*#pragma comment(lib, "P5DLL.lib")*/

#pragma comment(lib, "win32.lib")
#pragma comment(lib, "objects.lib")
#pragma comment(lib, "adt.lib")
#pragma comment(lib, "math.lib")
#pragma comment(lib, "ui3d.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "physics.lib")
#pragma comment(lib, "net.lib")
#pragma comment(lib, "ws2_32.lib")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{			
	Console::Init();
	Client::Init();
	
	Model model;
	ViewPort view;
	InputHandler inputHandler;
	Client client;
			
	InputHandler::setModel(model);	
	InputHandler::setClient(client);
	InputHandler::setView(view);

	if (!Controller<InputHandler>::Init(hInstance, "Controller"))
	{
		MessageBox(NULL, "could not initialize controller", "Error", MB_OK);
	}
	
	if (!Display::Init(Controller<InputHandler>::getHandle(), 0, 0, 1024, 768))
	{
		MessageBox(NULL, "Could not initialize display", "Error", MB_OK);
		return 2;
	}
	
	inputHandler.Init(Controller<InputHandler>::getHandle());

	//The main camera is not a visible object in the world.
	Camera * mainCam = inputHandler.GetCameras()->AddCamera(&inputHandler.getModel(), false);
	
	//Main camera is special.
	CameraManager::SetWorldCamera(mainCam);

	mainCam->Pos() = MVector3(10.0f, 0.0f, 10.0f);
	mainCam->View()= MVector3(0.0f, 0.0f, 1.0f);
	mainCam->Up() = MVector3(0.0f,1.0f,0.0f);
	MVector3 position = mainCam->Pos();
	MVector3 viewPosition = mainCam->View();
	mainCam->View()= position + MVector3::rotate(viewPosition, 0.0f, 3.14f / 2);

//	MovableObject mainObject(MVector3(10.0f, 0.0f, 10.0f), MVector3(0.0f, 0.0f, 1.0f));	
//	mainObject.View() = mainObject.Pos() + MVector3::rotate(mainObject.View() - mainObject.Pos(), 0.0f, 3.14f / 2);

	view.init();
	view.setCamera(*mainCam);
	inputHandler.GetCameras()->setCamera(mainCam->GetModel());

	ModelManipulator::SetPlayer(position);

//	controller.AddHandler(GloveDelegate<InputHandler>(&inputHandler, InputHandler::HandleGlove));

	Controller<InputHandler>::AddHandler(KeyDelegate<InputHandler>(&inputHandler, InputHandler::HandleKey, WM_KEYDOWN));
	Controller<InputHandler>::AddHandler(KeyDelegate<InputHandler>(&inputHandler, InputHandler::HandleChar));
	Controller<InputHandler>::AddHandler(KeyboardDelegate<InputHandler>(Controller<InputHandler>::getInstance(), Controller<InputHandler>::getHandle(), &inputHandler, InputHandler::HandleKeys));	
	Controller<InputHandler>::AddHandler(MouseDelegate<InputHandler>(Controller<InputHandler>::getInstance(), Controller<InputHandler>::getHandle(), &inputHandler, InputHandler::HandleMouse));
	
	ShowWindow(Controller<InputHandler>::getHandle(), nCmdShow);
	UpdateWindow(Controller<InputHandler>::getHandle());

	bool active(true);

	MSG msg;
	memset(&msg, 0, sizeof(msg));

	while (active)
	{			
		if (Controller<InputHandler>::PeekMessage(&msg))
		{ 
			if (msg.message == Controller<InputHandler>::QUIT)
			{
				active = false;
			}
			else
			{
				TranslateMessage(&msg);						
				DispatchMessage(&msg);
			}
		}
				
		inputHandler.HandleDisplay();
	}

	Console::Release();
	Controller<InputHandler>::Release();
	model.CloseScene();
	
	return static_cast<int>(msg.wParam);
}

