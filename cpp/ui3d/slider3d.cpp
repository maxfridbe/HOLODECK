//Used as a scroll bar, it can slide a list around for you.
#include "userinterface.h"
#include "slider3d.h"


Slider3d::Slider3d(Window3d * window, unsigned int fontListID)
{
	this->window = window;
	this->fontListID = fontListID;

	this->mode = Slider3d::verticle;

	this->myList = NULL;
	this->value = 0;
	this->caption = String("Slide");
	this->x = 20;
	this->y = 20;
	this->width = 300;
	this->height = 20;
	this->ticks = 10;
	this->state = Slider3d::Idle;
	this->min = -10;
	this->max = 100;
}


void Slider3d::DisplaySlider()
{
	//Safety:
	if ( value > max )
		value = max;
	if ( value < min )
		value = min;

	glColor3ub(200,200,200);
	glBegin(GL_QUADS);
	{
		glVertex2i(window->x + x, window->y + y);
		glVertex2i(window->x + x + width, window->y + y);
		glVertex2i(window->x + x + width, window->y + y + height);
		glVertex2i(window->x + x, window->y + y + height);
	}
	glEnd();

	if ( max == 0.0f )
		max = 1.0f;

	glColor3ub(50,50,50);

	if ( mode == horizontal )
	{
		glBegin(GL_QUADS);
		{
			glVertex2f((float)window->x + (float)x + (((float)value - min) * (float)width)/ (max - min), window->y + y - 5.0f);
			glVertex2f((float)window->x + (float)x + (((float)value - min) * (float)width)/ (max - min) + 2, window->y + y - 5.0f);
			glVertex2f((float)window->x + (float)x + (((float)value - min) * (float)width)/ (max - min) + 2, window->y + y + height + 5.0f);
			glVertex2f((float)window->x + (float)x + (((float)value - min) * (float)width)/ (max - min), window->y + y + height + 5.0f);
		}
		glEnd();
	}
	else
	{
		if ( max == min )
		{
			return;
		}

		glBegin(GL_QUADS);
		{
			glVertex2f((float)window->x + (float)x + 2.0f, window->y + (float)y + ((value - min) * (float)height)/ (max - min));
			glVertex2f((float)window->x + (float)x + 2.0f, window->y + (float)y + ((value - min) * (float)height)/ (max - min) + 4.0f );
			glVertex2f((float)window->x + (float)x + width - 2.0f, window->y + (float)y + ((value - min) * (float)height)/ (max - min) + 4.0f);
			glVertex2f((float)window->x + (float)x + width - 2.0f, window->y + (float)y + ((value - min) * (float)height)/ (max - min));
		}
		glEnd();		
	}
}


bool Slider3d::CheckIfClicked(Cursor3d & mouse )
{
	if ( state == Slider3d::Idle )
	{
		if ( mouse.onClick[0] && mouse.getX() > window->x + x && mouse.getX() < window->x + x + width
			&& mouse.getY() > window->y + y && mouse.getY() < window->y + y + height )
		{
			state = Slider3d::Dragging;
			
			if ( mode == horizontal )
			{
				value = (((float)(mouse.getX() - window->x - x )) / width) * (max-min) + min;
			}
			else
			{
				value = (((float)(mouse.getY() - window->y - y )) / height) * (max-min) + min;
			}


			if ( value > max )
				value = max;
			if ( value < min )
				value = min;

			if ( myList )
			{
				this->myList->SetScroll((int)value);
			}

			return true;
		}
		return false;
	}
	else if ( state == Slider3d::Dragging )
	{
		if (mouse.onRelease[0])
			state = Slider3d::Idle;
		if ( mode == horizontal )
		{
			value = (((float)(mouse.getX() - window->x - x )) / width) * (max-min) + min;
		}
		else
		{
			value = (((float)(mouse.getY() - window->y - y )) / height) * (max-min) + min;
		}
		if ( value > max )
			value = max;
		if ( value < min )
			value = min;

		if ( myList )
		{
			this->myList->SetScroll((int)value);
		}
		return true;
	}
	return false;
}

void Slider3d::setCaption(String text )
{
}

void Slider3d::setPos(int x , int y){ this->x = x; 	this->y = y; }
void Slider3d::setSize(int w, int h ) { this->width = w; this->height = h;  }
void Slider3d::setMax(float max) { this->max = max; }
void Slider3d::setMin(float min) { this->min = min; }
void Slider3d::setValue(float val) { this->value = val; }
void Slider3d::setList(ListBox3d * list) { this->myList = list; }
float Slider3d::getMax() { return this->max; }
float Slider3d::getMin() { return this->min; }
float Slider3d::getValue() { return this->value; }


