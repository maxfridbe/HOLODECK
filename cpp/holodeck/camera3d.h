#ifndef CAMERA3D_H
#define CAMERA3D_H

class MovableObject;
class Window3d;
class ViewPort;

class Camera3d
{
	public:
		Camera3d(MovableObject * cam, Window3d * window, int xLeft, int yTop, int ViewPortWidth, int ViewPortHeight, ViewPort * view);
		~Camera3d();
		Window3d * getParent();
		void Display();


	private:
		int x;
		int y;
		int yBottom;
		int width;
		int height;
		MovableObject * camera;
		Window3d * window;
		ViewPort * view;
};


#endif