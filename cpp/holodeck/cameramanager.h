#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include "camera.h"
#include "wlist.h"
#include "model.h"

class ViewPort;

class CameraManager
{
	public:
		CameraManager();
		~CameraManager();
		Camera * AddCamera(Model * model, bool visible = true);
		void DisplayQuickCams(ViewPort * view, Model * model);
		void DisconnectWindows(void * addressOfWindow);
		void setCamera(int number);
		void setCamera(RenderableObject * obj);
		Camera * GetCamera(int number);
		Camera * GetActive();
		bool IsCamera(Object * object);

		static void SetWorldCamera(Camera *);
		static Camera * GetWorldCamera();

	private:
		List < Camera * > cameras;	
		int activeCam;
		static Camera * worldCam;
};

#endif
