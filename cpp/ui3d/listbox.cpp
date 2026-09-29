
#include <iostream>
using namespace std;
#include "listbox.h"
#include "wstring.h"


ListBox3d::ListBox3d(Window3d * window, unsigned int fontListID, int x, int y, int w, int h, bool fileList)
{
	count = 0;
	clickCount = 0;
	this->clickLock = false;
	this->window = window;
	this->fontListID = fontListID;
	this->offset = 0;
	this->height = h;
	this->width = w;
	this->selected = 0;
	this->x = x;
	this->y = y;
	this->fileList = fileList;

	Button3d * up = window->addButton(Button3d::SCROLLUP, x + width - 20 , y);
	Button3d * down = window->addButton(Button3d::SCROLLDOWN, x + width - 20, y + height - 20);
	
	up->AssignActionEvent(EventHandlers::ScrollUp);
	down->AssignActionEvent(EventHandlers::ScrollDown);
	
	Slider3d * scrollBar = window->addSlider(x + width - 20, y + 20, 20, height - 40);
	scrollBar->setList(this);
	setSlider(scrollBar);

	textBox = window->addText(x, y + height, width, 25);
	pathBox = window->addText(x, y - 20,width, 20);

	//Load up the list.
	if ( fileList )
	{
		ReadDir("*");
	}
}

ListBox3d::~ListBox3d()
{
}

void ListBox3d::AddItem(String item)
{	
	this->items.PushBack(item);
	this->count++;
}


void ListBox3d::RemoveItem(String item)
{
	if ( this->selected > this->count - 1 || selected < 0)
	{
		return;
	}
	items.Remove(this->selected);
	this->count --;
	
	/*List< String >::Iterator it = items.Begin();

	while (!it.isNull())
	{
		if ( (*it) == item)
		{
			this->items.Remove(it);
			return;
		}
		it++;
	}*/
}


void ListBox3d::ReadDir( String path)
{
	if ( !this->fileList )
	{
		return;
	}

	//Load drive letters.
	DWORD mask = GetLogicalDrives();
	for (int letter = 0, i = 1; i < 0x02000000; i <<=1)
	{
		if (i & mask)
		{		
			String driveLetter = String(1, static_cast<char>(letter + 'A'));
			driveLetter += ":\\";
			items.PushBack(driveLetter);
			count ++;
			cout << "Drive " << (char)(letter + 'a')<< " exists" << endl;	
		}
		letter++;
	}

	//2 pass reader, once for dir, once for files.
	for (int i = 0 ; i < 2; i++ )
	{
		String dirPath;
		String dirNewPath;
		
		WIN32_FIND_DATA data;
		HANDLE hFind;
		BOOL bContinue = TRUE;

		dirPath = path;

		hFind = FindFirstFile(dirPath.getCString(), &data);


		if (INVALID_HANDLE_VALUE == hFind) {
			return;
		}

		// If we have no error, loop thru the files in this dir
		while (hFind && bContinue) {
			// Check if this entry is a directory
			if (i == 0 && data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// Make sure this dir is not . or ..
				if ( strcmp(data.cFileName , "..") == 0 )
				{
					String up = String("<Up Dir>");
					items.PushBack(up);
					this->count++;
				}
				else if ( strcmp(data.cFileName, ".") == 0)
				{}
				else 
				{
					String dirr = String("<");
					dirr += data.cFileName;
					dirr += ">";
					items.PushBack(dirr);
					this->count++;
				}
			}
			else if ( i == 1 && !(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				items.PushBack(data.cFileName);
				this->count++;
			}

			bContinue = FindNextFile(hFind, &data);
		}	

		FindClose(hFind); // Free the dir structure

		char directory[1024];
		GetCurrentDirectory(sizeof(directory),directory);
		pathname = String(directory);

		String dir = String(directory);
		if (dir.getLength() > 21)
			dir = dir.Substring(dir.getLength() - 21, dir.getLength() - 1);

		pathBox->setText((char * )dir.getCString());

	}
}
void ListBox3d::DisplayListBox()
{
	int xbase = window->x + x;
	int ybase = window->y + y;
	
	if ( mySlider )
	{
		mySlider->setMax(this->count - ((height /16 ) - 1 ) - 1); 
		mySlider->setMin( 0 );
		mySlider->value = (float)offset;
	}

	//Border
	glColor3ub(54,54,54);
	glBegin(GL_QUADS);
		glVertex2d(xbase - 2, ybase - 2);
		glVertex2d(xbase + width + 2, ybase - 2);
		glVertex2d(xbase + width + 2, ybase + height + 2);
		glVertex2d(xbase - 2, ybase + height + 2);
	glEnd();

	glEnable(GL_SCISSOR_TEST);
	int newX = window->getX() + x;
	int newY = window->getY() + y;
	newY = UserInterface::WindowHeight - newY - height;

	glScissor(newX, newY, width, height);

	glColor3ub(210,210,210);
	glBegin(GL_QUADS);
		glVertex2d(xbase, ybase);
		glVertex2d(xbase + width, ybase );
		glVertex2d(xbase + width, ybase + height);
		glVertex2d(xbase, ybase + height);
	glEnd();

	glColor3ub(0,210,210);
	glBegin(GL_QUADS);
		glVertex2d(xbase, ybase + (selected * 16) - (offset * 16) + 3);
		glVertex2d(xbase + width, ybase + (selected * 16) - (offset * 16) + 3);
		glVertex2d(xbase + width, ybase + 16 + (selected * 16) - (offset * 16) + 3);
		glVertex2d(xbase, ybase + 16 + (selected * 16) - (offset * 16) + 3);
	glEnd();

	for( int i = 0; i < items.getSize(); i ++ )
	{
		if ( items[i][0] == '<' )
		{
			glColor3ub(UserInterface::folderColor.r,UserInterface::folderColor.g,UserInterface::folderColor.b);
		}

		else
		{		
			glColor3ub(UserInterface::textColor.r, UserInterface::textColor.g, UserInterface::textColor.b);
		}
		glRasterPos2i( xbase + 5, ybase + (i * 16) + 16 - (offset * 16) );
		glListBase(this->fontListID);
		glCallLists((GLsizei)strlen(this->items[i].getCString() ), GL_UNSIGNED_BYTE, (GLvoid*)items[i].getCString() );
	}

	glColor3ub(180,180,180);
	glBegin(GL_QUADS);
		glVertex2d(xbase + width - 20, ybase + 20);
		glVertex2d(xbase + width, ybase + 20);
		glVertex2d(xbase + width, ybase + height - 20);
		glVertex2d(xbase + width - 20, ybase + height - 20);
	glEnd();

	glDisable(GL_SCISSOR_TEST);
}

bool ListBox3d::CheckIfClicked(Cursor3d & mouse)
{
	static time_t last = time(NULL);
	if ( difftime(time(NULL),last) > 1.5 )
	{
		last = time(NULL);
		clickCount = 0;
	}

	if ( mouse.onRelease[0] && mouse.getX() > window->x + x && mouse.getX() < window->x + x + width - 20 &&
			mouse.getY() > window->y + y && mouse.getY() < window->y + y + height)
	{
		int index = mouse.getY() - window->getY() - y;
		index = (int) (index / 16) + offset;

		if ( index >= count )
		{
			index = count - 1;
			clickCount = 0;
		}

		if ( index < 0 )
		{
			index = 0;
			clickCount = 0;
		}

		if ( fileList)
		{
			this->MoveIn( index ) ;
		}
		else
		{
			this->ClickCount(index);
		}
		return true;
	}
	else
	{
		return false;
	}
}

void ListBox3d::ClickCount(int index )
{
	if ( index == selected )
	{
		clickCount++;
	}
	else
	{
		selected = index;
		textBox->setText((char * )items[selected].getCString());
		clickCount = 1;
	}

	if ( clickCount == 2)
	{
		if ( !clickLock )
		{
			window->okButton->OnClick();
		}
		else
		{
			clickCount = 0;
		}
	}
}

void ListBox3d::MoveIn( int index )
{

	if ( index == selected )
	{
		clickCount++;
	}
	else
	{
		selected = index;
		textBox->setText((char * )items[selected].getCString());
		clickCount = 1;
	}

	if ( clickCount == 2)
	{

		char directory[1024];
		GetCurrentDirectory(sizeof(directory),directory);

		if ( items[selected] == "<Up Dir>")
		{
			offset = 0;
			selected = 0;
			SetCurrentDirectory("..");
		}
		else if ( items[selected][items[selected].getLength() - 1] == '\\' )
		{
			SetCurrentDirectory( items[selected].getCString() );
			offset = 0;
			selected = 0;
		}
		//selecting a file
		else if ( items[selected][items[selected].getLength() - 1] != '>' && items[selected][items[selected].getLength() - 1] != '\\')
		{
			if ( items[selected].getLength() > 0 )
			{
				UserInterface::Msg m;
				
				if ( UserInterface::command == UserInterface::RequestLoadModel )
					m.message = UserInterface::ModelLoadReady;
				else if (  UserInterface::command == UserInterface::RequestSaveScene )
					m.message = UserInterface::SceneSaveReady;
				else if ( UserInterface::command == UserInterface::RequestLoadScene )
					m.message = UserInterface::SceneLoadReady;


				UserInterface::command = UserInterface::None;
		
				char directory[1024];
				GetCurrentDirectory(sizeof(directory),directory);
				pathname = String(directory);

				filename = String(pathname + String("\\") + textBox->getCaption());
				
				window->okButton->OnClick();
				
				//m.specialString = window->lists[0]->getFilename();

				//window->state.setState(WindowState::WindowStates::CLOSING );
				//window->state.setKillFlag();

				//UserInterface::EventEnque(m);
			}
			else 
			{
				return;
			}

		}
		//Directory double clicked:
		else
		{
			String gotoDir = this->items[selected].getCString();
			try
			{
				gotoDir.TrimEnd(1);
				gotoDir.TrimFront(1);
			}
			catch (...){}

			offset = 0;
			selected = 0;

			SetCurrentDirectory( gotoDir.getCString());
		}

		clickCount = 0;
		count = 0;

		String path = "*";
		items = List<String>();

		this->ReadDir(path);
	}
}

void ListBox3d::Scroll(int amount)
{
	int finalAmount = offset + amount;

	if ( finalAmount < (count - ((height /16 ) - 2)) && finalAmount >= 0)
		offset += amount;
}

void ListBox3d::SetScroll(int amount)
{
	if ( (amount) < (count - ((height /16 ) - 2)) && (amount) >= 0)
		this->offset = amount;
}

String ListBox3d::getSelectedItem() 
{ 
	if ( this->selected > this->count - 1 || selected < 0)
		return String("NULL"); 
	
	return this->items[this->selected]; 
}

String ListBox3d::getItem(int index) {  if ( index < this->count ) return this->items[index]; else return String("NULL"); }
String ListBox3d::getFilename(){  return pathname + String("\\") + textBox->getCaption(); } 
void ListBox3d::setClickLock(bool state) { clickLock = state; }
void ListBox3d::setPos(int x, int y ){ this->x = x; this->y = y; }
void ListBox3d::setSize(int w, int h ){ width = w; height = h; }
int ListBox3d::getSelectedIndex() { return selected; }
int ListBox3d::getOffset() { return offset; }
void ListBox3d::setSlider( Slider3d * slider ) { this->mySlider = slider; slider->setMax(this->count - ((height / 16.0 ) - 1.0 )) ; slider->setMin( 0.0 );} 
