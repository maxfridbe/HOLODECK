#ifndef SLIDER_H
#define SLIDER_H

#include "listbox.h"

class Slider3d
{
	public:
		friend class ListBox3d;
		Slider3d(Window3d * window, unsigned int fontListID);
		~Slider3d();

		enum SlideState
		{
			Idle,
			Dragging,
		};

		SlideState state;

		enum Mode { verticle, horizontal };

		bool CheckIfClicked(Cursor3d & mouse);
		void DisplaySlider();	

		void setPos(int x, int y);
		void setSize(int w, int h);
	
		Mode mode;
		void setCaption(String text);
		void setList(ListBox3d * myList);
		void setTicks(int amount);
		void setMax(float max);
		void setMin(float min);
		void setValue(float val);
		float getMax();
		float getMin();
		float getValue();

		int getWidth();
		int getHeight();
		int getX();
		int getY();
	
	private:
		int x;
		int y;
		int width;
		int height;
		float max;
		float min;
		float value;
		int ticks;
		ListBox3d * myList;

		String caption;
		Window3d * window;
		unsigned int fontListID;
};

#endif