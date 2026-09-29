#ifndef TEST_H
#define TEST_H

#include "display.h"
#include "view.h"
#include "model.h"
//#include "glovedelegate.h"
#include "Window3d.h"
//#include "light.h"
#include "client.h"
#include "cameramanager.h"

class InputHandler
{
	public:
		InputHandler();
		
		void Init(HWND handle);
		
		void HandleKey(unsigned char);
		void HandleChar(unsigned char);
		void HandleKeys(unsigned char []);
		void HandleMouse(int relX, int relY, int relZ, int x, int y, unsigned char buttons[]);
		//void HandleGlove(const GloveDelegate<InputHandler>::GloveState &);
		void HandleDisplay();	
		void PostDisplayCleanUp();
		void SelectObjectsWithMouse(int x, int y);

		static void setModel(Model &);
		static void setView(ViewPort &);
		static void setClient(Client &);
		static Model & getModel();
		static CameraManager * GetCameras();
		static ViewPort * GetViewport();
	private:
//		UserInterface ui;
//		Light light;
		static Model * model;
		static ViewPort * view;
		static Client * client;
		static CameraManager cameras;
};

#endif