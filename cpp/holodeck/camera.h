#ifndef CAMERA_H
#define CAMERA_H

#include "renderableobject.h"
#include "userinterface.h"

class Camera
{
	friend class CameraManager;
	public:
		Camera();
		Camera(RenderableObject * model);
		~Camera();
		void SetCameraModel(RenderableObject * obj);
		void SyncModel();
		RenderableObject * GetModel();

		MVector3 & Pos();
		MVector3 & View();
		MVector3 & Up();
		double & Phi();
		double & Theta();
		double & Fov();

		void setPos(MVector3 & pos);
		void setView(MVector3 & view);
		void setUp(MVector3 & up);
		void setControl(bool state);
		inline void ViewportOn(bool state) { viewportOn = state; }
		inline bool ViewportOn() { return viewportOn; }
		inline bool ViewportOnToggle() 
		{ 
			if ( viewportOn ) 
			{
				return (viewportOn = false);
			}
			else
			{
				return (viewportOn = true);
			}
		}

		void Name(String myName) { name = myName; }
		String Name() { return name; }

		Window3d * BorderWindow;

	private:
		bool control;
		bool viewportOn;
		bool visible;
		String name;
		MVector3 * pos;
		MVector3 * view;
		MVector3 * up;
		double phi;
		double theta;
		double fov;
		RenderableObject * model;
};

#endif
