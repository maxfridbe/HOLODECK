/*
	Viewport class.  This class controls the way the world looks as far as Perspective and
	screen size is concerned.  Use is as follows:
	* Create an instance of the view class and run the 
		Init(int w = 1024, int h = 768, double newFov = 45.0, double near = 1.0, double far = 8000.0)
		method once.
	
	* Assign a MovableObject type to the class using the setCamera(MovableObject & camera) method.
		this object's position and view will be the camera eye and view center for the viewport.

	* In the event that you change either the w, h, fov, or the clipping planes, besure to call the
		UpdateProjectionMatrix() function as to reset the view to the new settings.

	* Call the view->update() function before you begin drawing anything in the world.  
		this function is responsible for positioning the camera object associated with the view.

	* To create a small view of the world in a smaller viewport call the 
		QuickCamView( Camera, Model, x Position from left, y Position from bottom, width ( to right ) and height ( upwards ) )
		This will create a camera which uses the world as its render.  Model contains all objects in
		the world, these are the objects that will be visible. The grid also is drawn. Blank space, will
		be rendered as transparent. To prevent this, place a window behind the quickcam.
*/

#ifndef VIEW_H
#define VIEW_H

#include "model.h"
#include "mvector3.h"
#include "cameramanager.h"
#include "window3d.h"

class ViewPort
{
	public:
		ViewPort();
		enum ViewPlane { Perspective, Front, Back, Left, Right, Top, Bottom };
		ViewPlane viewType;
		void init(int w = 1024, int h = 768, double newFov = 45.0, double near = 1.0, double far = 8000.0);
		void UpdateProjectionMatrix();
		void RenderView();

		void QuickCamView(Camera * camera, Model & model, int x, int y, int width, int height);

		void MakeOrthoFront();
		void MakeOrthoBack();
		void MakeOrthoTop();
		void MakeOrthoBottom();
		void MakeOrthoRight();
		void MakeOrthoLeft();

		void MakePerspective();
		void ZoomIn(float amount);
		void ZoomOut(float amount);
		float ZoomPercent(float percentageZoom);
		void setCamera(Camera &);
		void setFocus(MovableObject &);
		void DrawGrid();
		void DrawGridBody();
		void DrawGridLines();
		void DrawAxis();
		void DrawColorCube(float x, float y, float z);
		double getFov() const;
		void unFocus();
		void update();
		Camera & getCamera();
		double theta;
		double phi;


	private:		
		double fov;
		double zoomLevel;
		bool perspectiveMode;
		int x;
		int y;
		int width;
		int height;
		
		MVector3 posLastPos;
		double oldTheta;
		double oldPhi;

		MVector3 posFront;
		MVector3 posBack;
		MVector3 posLeft;
		MVector3 posRight;
		MVector3 posTop;
		MVector3 posBottom;

		double startRange;
		double endRange;
		Camera * camera;
		MovableObject * objectToFocus;
};

#endif