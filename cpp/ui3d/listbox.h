#ifndef LISTBOX_H
#define LISTBOX_H

#include "window3d.h"
#include "textBox3d.h"
#include "wlist.h"
#include "wstring.h"

class Cursor3d;


class ListBox3d
{
	public:
		friend class Slider3d;
		ListBox3d(Window3d * window, unsigned int fontListID, int x, int y, int w, int h, bool fileList = true);
		~ListBox3d();

		void AddItem(String item);
		void RemoveItem(String item);

		bool CheckIfClicked(Cursor3d & mouse);
		void MoveIn( int index );

		void DisplayListBox();
		void ReadDir(String path);
		List < String > items;

		int getWidth();
		int getHeight();
		int getX();
		int getY();
		int getOffset();
		int	getSelectedIndex();
		String getSelectedItem();
		String getItem(int index);

		String getFilename();
		void Scroll(int amount);
		void SetScroll(int amount);
		void setSize( int width, int height );
		void setPos (int x, int y);
		void setSelected( int selectedIndex );
		void setSlider ( Slider3d * slider);
		void setClickLock(bool state);

private:
		int x;
		int y;
		int sUpX;
		int sUpY;
		int sDownX;
		int sDownY;
		int width;
		int height;
		int selected;
		int count;
		int clickCount;
		int offset;
		bool fileList;
		bool clickLock;
		unsigned int fontListID;
		String pathname;
		String filename;
		Slider3d * mySlider;
		Window3d * window;
		TextBox3d * textBox;
		TextBox3d * pathBox;
		void ClickCount(int indexClickedOn);
};

#endif

