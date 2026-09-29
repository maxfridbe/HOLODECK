/*********************************************************************
	TextBox
	@desc = The text box class holds text and allows for input. 
	And manipulations.
**********************************************************************/
#include "userinterface.h"
#include "wstring.h"

TextBox3d::TextBox3d(Window3d * window, unsigned int fontID, int id)
{
	this->ID = id;
	this->height = 24;
	this->width = 64;
	this->x = 0;
	this->y = 0;
	this->cursor = 0;
	this->isNumber = false;
	this->lock = false;
	this->isPassword = false;
	this->window = window;
	window->textFocus = id;
	this->fontListID = fontID;
	data = "";
	
	//Text Color
	this->redTxt = 0;
	this->greenTxt = 0;
	this->blueTxt = 225;

	//HighlightBox color
	this->redHighlite = 0;
	this->greenHighlite = 0;
	this->blueHighlite = 225;

	this->redLowlite = 125;
	this->greenLowlite = 125;
	this->blueLowlite = 125;
}

TextBox3d::~TextBox3d()
{	

}

void TextBox3d::DisplayTextBox()
{
	glEnable(GL_SCISSOR_TEST);
	int newX = window->getX() + x;
	int newY = window->getY() + y;
	newY = 768 - newY - height;

	glScissor(newX - 2, newY - 2, width + 4, height + 4);

	int xBase = window->getX() + x;
	int yBase = window->getY() + y;

	if ( !isLocked() )
	{
		if ( ID == window->getTextID() )
		{

			glColor3ub(redHighlite, greenHighlite, blueHighlite);
			glBegin(GL_QUADS);
			{
				glVertex2d(xBase - 2, yBase - 2);
				glVertex2d(xBase + width + 2, yBase - 2);
				glVertex2d(xBase + width + 2, yBase + height + 2);
				glVertex2d(xBase - 2, yBase + height + 2);
			}	
			glEnd();


			glColor3ub(220,220,220);
		}
		else
		{
			glColor3ub(redLowlite, greenLowlite, blueLowlite);
			glBegin(GL_QUADS);
			{
				glVertex2d(xBase - 2, yBase - 2);
				glVertex2d(xBase + width + 2, yBase - 2);
				glVertex2d(xBase + width + 2, yBase + height + 2);
				glVertex2d(xBase - 2, yBase + height + 2);
			}	
			glEnd();

			glColor3ub(170,170,170);
		}

		glBegin(GL_QUADS);
			glVertex2d(xBase, yBase);
			glVertex2d(xBase + width, yBase);
			glVertex2d(xBase + width, yBase + height);
			glVertex2d(xBase, yBase + height);
		glEnd();

		if ( ID == window->getTextID() )
		{
			//draw cursor
			glColor3ub(220,0,0);
			glBegin(GL_QUADS);
				glVertex2d(xBase + cursor * 11 + 3, yBase + 2);
				glVertex2d(xBase + cursor * 11 + 5, yBase + 2);
				glVertex2d(xBase + cursor * 11 + 5, yBase + height - 2);
				glVertex2d(xBase + cursor * 11 + 3, yBase + height - 2);
			glEnd();
		}
	}

	glColor3ub(redTxt,greenTxt,blueTxt);
	glRasterPos2i(window->getX() + this->x + 4, window->getY() + this->y + 16);
	glListBase(this->fontListID);
	if ( !isPass() )
	{
		glCallLists(data.getLength(), GL_UNSIGNED_BYTE, (GLvoid*)data.getCString());
	}
	else
	{
		char * blank = new char[data.getLength()];
		memset(blank,'*',data.getLength());
		glCallLists(data.getLength(), GL_UNSIGNED_BYTE, (GLvoid*)blank);	
	}
	glDisable(GL_SCISSOR_TEST);
}

void TextBox3d::setPos(int x, int y)
{
	this->x = x;
	this->y = y;
}

void TextBox3d::setText( String text )
{
	cursor = 0;
	data = String(text);
	MoveCursor( 0 );
}
void TextBox3d::appendText( int pos,  String text )
{
	pos;
	String temp = data;

	if ( cursor > 0 )
	{
		data = data.Substring(0,cursor - 1);
		data += text;
	}		
	else
	{
		data = "";
		data += text;
	}
	if ( cursor != temp.getLength() )
	{
		data += temp.Substring(cursor, temp.getLength() - 1 );
	}
	
	MoveCursor(1);
}

void TextBox3d::MoveCursor(int amount)
{
	if ( (cursor + amount) >= 0 && (cursor + amount) <= this->data.getLength())
	{
		cursor += amount;
	}
}

void TextBox3d::MoveCursorTo(int amount)
{
	if ( (amount) >= 0 && (amount) <= this->data.getLength())
	{
		cursor = amount;
	}

	else
	{
		cursor = this->data.getLength();
	}
}


bool TextBox3d::CheckIfClicked(Cursor3d & mouse )
{
	if ( mouse.getX() > window->x + x && mouse.getX() < window->x + x + width &&
			mouse.getY() > window->y + y && mouse.getY() < window->y + y + height)
	{
		window->setTextID(ID);
		int cursorPos = (mouse.getX() - window->x  - x - 1)/11;
		MoveCursorTo(cursorPos);
		return true;
	}
	else 
	{
		return false;
	}
}

void TextBox3d::insertLetter(char letter)
{
	//if the setReadOnly(true) method has been called on this text box, we cannot modify it.
	if ( isLocked() )
	{
		return;
	}
	//if the setDigit() method has been called, only digits will be taken as input.
	else if ( isDigit() )
	{
		if ( isdigit(letter) || letter == '.' || letter == '-' || letter == 8 || letter == 127)
		{
			//In the event of a backspace, we will remove text at cursor.
			if (letter == 8)
			{
				if ( data.getLength() > 0 )
				{
					String temp = data;

					if ( cursor > 1 )
					{
						data = data.Substring(0,cursor - 2);
					}		
					else
					{
						data = "";
					}
					if ( cursor != temp.getLength() )
					{
						data += temp.Substring(cursor, temp.getLength() - 1 );
					}

					MoveCursor(-1);
				}
			}
			//in the event of delete we will remove text infront of the cursor.
			else if (letter == 127)
			{
				if ( cursor == this->getCaption().getLength()  )
				{
					return;
				}

				MoveCursor(1);
				if ( data.getLength() > 0 )
				{
					String temp = data;

					if ( cursor > 1 )
					{
						data = data.Substring(0,cursor - 2);
					}		
					else
					{
						data = "";
					}
					if ( cursor != temp.getLength() )
					{
						data += temp.Substring(cursor, temp.getLength() - 1 );
					}

					MoveCursor(-1);
				}
			}
			//anything else that is a digit ( '0-9', '-', '.' ) is inserted.
			else
			{
				char * letterString = new char[2];
				letterString[0] = letter;
				letterString[1] = '\0';
				String text = String(letterString);
				appendText(cursor, text);
				delete [] letterString;
			}
		}
		else
			return;
	}
	//If this is a completely unrestricted textbox
	else
	{
		//in the event of a backspace take out text.
		if (letter == 8)
		{
			if ( data.getLength() > 0 )
			{
				String temp = data;

				if ( cursor > 1 )
				{
					data = data.Substring(0,cursor - 2);
				}		
				else
				{
					data = "";
				}
				if ( cursor != temp.getLength() )
				{
					data += temp.Substring(cursor, temp.getLength() - 1 );
				}

				MoveCursor(-1);
			}

		}
		//in the event of a delete remove text after cursor.
		else if (letter == 127)
		{
			if ( cursor == this->getCaption().getLength()  )
			{
				return;
			}

			MoveCursor(1);
			if ( data.getLength() > 0 )
			{
				String temp = data;

				if ( cursor > 1 )
				{
					data = data.Substring(0,cursor - 2);
				}		
				else
				{
					data = "";
				}
				if ( cursor != temp.getLength() )
				{
					data += temp.Substring(cursor, temp.getLength() - 1 );
				}

				MoveCursor(-1);
			}
		}
		//place text into the field.
		else
		{
			char * letterString = new char[2];
			letterString[0] = letter;
			letterString[1] = '\0';
			String text = String(letterString);
			appendText(cursor, text);
			delete [] letterString;
		}
	}
}


String TextBox3d::getCaption() 
{  
	if ( this->isDigit() )
	{
		if ( data == "" )
		{
			data = String("0");
		}
	}

	if ( data == NULL )
	{
		data = String(" ");
	}

	return data; 
} 

int TextBox3d::getWidth() { return width; } 
int TextBox3d::getHeight() { return height; }
int TextBox3d::getX() { return x; }
int TextBox3d::getY() { return y; }
void TextBox3d::setSize(int w, int h) { width = w; height = h;}

void TextBox3d::setReadOnly(bool state) { this->lock = state; }
void TextBox3d::setPass( bool state) { this->isPassword = state; }
void TextBox3d::setDigit(bool state) { this->isNumber = state; }
bool TextBox3d::isLocked() { return lock; }
bool TextBox3d::isDigit() { return isNumber; }
bool TextBox3d::isPass() { return isPassword;}

