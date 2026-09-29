#include "cameramanager.h"
#include "inputhandler.h"

Camera * CameraManager::worldCam = NULL;

CameraManager::CameraManager(void)
{
	this->activeCam = -1;
}

CameraManager::~CameraManager(void)
{
}

void CameraManager::DisplayQuickCams(ViewPort * view, Model * model)
{
	for ( int i = 0; i < cameras.getSize(); i++)
	{
		if ( cameras[i]->ViewportOn() && cameras[i] != this->worldCam )
		{
			cameras[i]->GetModel()->setVisible(false);
			view->QuickCamView( cameras[i] , *model, 100, 100, 200, 200);
			cameras[i]->GetModel()->setVisible(true);
		}
	}
}

void CameraManager::DisconnectWindows(void * addressOfWindow)
{
	for ( int i = 0; i < cameras.getSize(); i++)
	{
		if ( cameras[i]->BorderWindow == addressOfWindow)
		{
			cameras[i]->ViewportOn(false);
			cameras[i]->BorderWindow = NULL;
			return;
		}
	}
}


void CameraManager::SetWorldCamera(Camera * worldCam )
{
	CameraManager::worldCam = worldCam;
}

Camera * CameraManager::GetWorldCamera()
{
	return CameraManager::worldCam;
}


Camera * CameraManager::AddCamera(Model * model, bool visible)
{
	Camera * camera = new Camera();
	camera->SetCameraModel( dynamic_cast<RenderableObject *>(model->Open("camera","3dbin", "data\\") ) );
	camera->visible = visible;
	camera->model->setVisible(visible);
	this->cameras.PushBack( camera );
	return camera;
}

Camera * CameraManager::GetCamera(int number)
{
	if ( number < cameras.getSize() && number >= 0)
	{
		return cameras[number];
	}
	else
	{
		return NULL;
	}
}

Camera * CameraManager::GetActive()
{
	if ( cameras.getSize() > 0  && this->activeCam < cameras.getSize() && activeCam >= 0 )
	{
		return cameras[activeCam];
	}

	return NULL;
}

void CameraManager::setCamera(RenderableObject * obj)
{
	//Checks to see if the obj being sent is one of the camera's models. if so 
	//it will become that camera.
	for ( int i = 0; i < cameras.getSize(); i++)
	{
		if ( obj == cameras[i]->model)
		{
			this->activeCam = i;
			break;
		}
	}

    for ( int i = 0; i < this->cameras.getSize(); i ++ )
	{
		if ( cameras[i]->visible)
		{
			cameras[i]->model->setVisible(true);
		}
	}

	
//	cameras[activeCam]->model->setVisible(false);

}

bool CameraManager::IsCamera(Object * obj)
{
	for ( int i = 0; i < cameras.getSize(); i++)
	{
		if ( obj == cameras[i]->model)
		{
			return true;
		}
	}
	return false;
}
