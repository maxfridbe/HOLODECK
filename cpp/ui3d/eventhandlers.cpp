#include "eventhandlers.h"
#include "stdlib.h"

#include "userinterface.h"

EventHandlers::EventHandlers(void)
{
}

EventHandlers::~EventHandlers(void)
{
}

void EventHandlers::DebugClick(Button3d * button)
{
	if ( UserInterface::OnMouseRelease(0) )
	{	
		int xPos = UserInterface::getMouseX() - button->getParent()->getX() - button->getWidth() / 2;
		int yPos = UserInterface::getMouseY() - button->getParent()->getY() - button->getHeight() / 2;
		button->setPos(xPos, yPos);
		
		String cap = String::ToString(button->getX());
		cap += ",";
		cap += String::ToString(button->getY());
		button->setCaption(cap);
	}
}

void EventHandlers::AddForce(Button3d * button)
{
	UserInterface::Msg m;
	m.message = UserInterface::AddForce;
	m.specialString = button->getParent()->ForceList->getSelectedItem();
	button->getParent()->AppliedList->AddItem(m.specialString);
	button->getParent()->ForceList->RemoveItem(m.specialString);
	
	UserInterface::EventEnque(m);
}

void EventHandlers::AddWorldForce(Button3d * button)
{
	//Error checked
	UserInterface::Msg m;	
	m.message = UserInterface::Operation::AddWorldForce;	
	EventHandlers::ForceInfo * forceInfo = new ForceInfo();
	
	Window3d * win = button->getParent();
	
	//forceInfo -> duration = (win -> forceDur -> getCaption() != NULL ) ? win->forceDur -> getCaption().ToDouble() : 0.0f;
	forceInfo -> duration = win->forceDur -> getCaption().ToDouble();
	forceInfo -> mass = (win -> forceMass -> getCaption() != NULL ) ? win -> forceMass -> getCaption().ToDouble() : 0.0f;			
	forceInfo -> x = (win -> vX -> getCaption() != NULL ) ? win -> vX -> getCaption().ToDouble() : 0.0f;
	forceInfo -> y = (win -> vY -> getCaption() != NULL ) ? win -> vY -> getCaption().ToDouble() : 0.0f;
	forceInfo -> z = (win -> vZ -> getCaption() != NULL ) ? win -> vZ -> getCaption().ToDouble() : 0.0f;
	
	m.dataInfo = forceInfo;
	
	m.specialString = button -> getParent() -> forceName -> getCaption();


	UserInterface::EventEnque(m);
}

void EventHandlers::RemoveForce(Button3d * button)
{
	UserInterface::Msg m;
	m.message = UserInterface::Operation::RemoveForce;
	m.specialString = button->getParent()->AppliedList->getSelectedItem();
	button->getParent()->AppliedList->RemoveItem(m.specialString);
	button->getParent()->ForceList->AddItem(m.specialString);
	
	UserInterface::EventEnque(m);
}

void EventHandlers::ApplyTransform(Button3d * button)
{
	float * temp = new float[9];
	temp[0] = (button->getParent()->xRotate->getCaption() != NULL) ? (button->getParent()->xRotate->getCaption()).ToFloat() : 0.0f;
	temp[1] = (button->getParent()->yRotate->getCaption()!= NULL) ? (button->getParent()->yRotate->getCaption()).ToFloat() : 0.0f;
	temp[2] = (button->getParent()->zRotate->getCaption()!= NULL) ? (button->getParent()->zRotate->getCaption()).ToFloat() : 0.0f;
	temp[3] = (button->getParent()->xScale->getCaption()!= NULL) ? (button->getParent()->xScale->getCaption()).ToFloat() : 0.0f;
	temp[4] = (button->getParent()->yScale->getCaption()!= NULL) ? (button->getParent()->yScale->getCaption()).ToFloat() : 0.0f;
	temp[5] = (button->getParent()->zScale->getCaption()!= NULL) ? (button->getParent()->zScale->getCaption()).ToFloat() : 0.0f;
	temp[6] = (button->getParent()->xTrans->getCaption()!= NULL) ? (button->getParent()->xTrans->getCaption()).ToFloat() : 0.0f;
	temp[7] = (button->getParent()->yTrans->getCaption()!= NULL) ? (button->getParent()->yTrans->getCaption()).ToFloat() : 0.0f;
	temp[8] = (button->getParent()->zTrans->getCaption()!= NULL) ? (button->getParent()->zTrans->getCaption()).ToFloat() : 0.0f;

	UserInterface::Msg m(UserInterface::ModelModReady, temp);
	UserInterface::EventEnque(m);
	delete [] temp;
}


void EventHandlers::GridSettings(Button3d * button)
{
	DefaultCloseWindow(button);
	UserInterface::command = UserInterface::None;
	if ( button->getCaption() == "White Backround")
	{
		UserInterface::Msg m;
		m.message = UserInterface::SelectGridColorScheme;
		m.specialString = String("White");
		UserInterface::EventEnque(m);
	}
	else if ( button->getCaption() == "Black Backround")
	{
		UserInterface::Msg m;
		m.message = UserInterface::SelectGridColorScheme;
		m.specialString = String("Black");
		UserInterface::EventEnque(m);
	}
}


void EventHandlers::LoadModel(Button3d * button)
{
	String filename = button->getParent()->lists[0]->getFilename();
	//Safety checks for file consistancy:
	if ( filename.getLength() > 0 )
	{
		if ( filename[filename.getLength() - 1] == '>' || filename[filename.getLength() - 1] == '\\')
		{
			return;
		}
	}
	else 
	{
		return;
	}

	DefaultCloseWindow(button);
	UserInterface::Msg m;
	UserInterface::command = UserInterface::None;
	m.message = UserInterface::ModelLoadReady;
	m.specialString = button->getParent()->lists[0]->getFilename();
	UserInterface::EventEnque(m);
}

void EventHandlers::LoadScene(Button3d * button)
{
	String filename = button->getParent()->lists[0]->getFilename();
	
	//Safety checks for filename consistancy:
	if ( filename.getLength() > 0 )
	{
		if ( filename[filename.getLength() - 1] == '>' || filename[filename.getLength() - 1] == '\\')
		{
			return;
		}
	}
	else 
	{
		return;
	}

	DefaultCloseWindow(button);
	UserInterface::Msg m;
	UserInterface::command = UserInterface::None;

	m.message = UserInterface::SceneLoadReady;
	m.specialString = button->getParent()->lists[0]->getFilename();
	UserInterface::EventEnque(m);
	
}

void EventHandlers::SaveScene(Button3d * button)
{
	String filename = button->getParent()->lists[0]->getFilename();

	//Safety checks for filename consistancy:
	if ( filename.getLength() > 0 )
	{
		if ( filename[filename.getLength() - 1] == '>' || filename[filename.getLength() - 1] == '\\')
		{
			return;
		}
	}
	else 
	{
		return;
	}

	DefaultCloseWindow(button);
	UserInterface::Msg m;
	UserInterface::command = UserInterface::None;

	m.message = UserInterface::SceneSaveReady;
	m.specialString = button->getParent()->lists[0]->getFilename();
	UserInterface::EventEnque(m);
}

void EventHandlers::NetConnect(Button3d * button)
{
	DefaultCloseWindow(button);
	UserInterface::Msg m;
	m.message = UserInterface::ConnectionRequest;
	m.specialString = button->getParent()->ip->getCaption();
	UserInterface::EventEnque(m);
}


void EventHandlers::ScrollDown(Button3d * button)
{
	if ( button->getParent()->lists[0] )
		button->getParent()->lists[0]->Scroll(1);
}
void EventHandlers::ScrollUp(Button3d * button)
{
	if ( button->getParent()->lists[0] )
		button->getParent()->lists[0]->Scroll(-1);
}

void EventHandlers::UnloadModel(Button3d * button)
{
	DefaultCloseWindow(button);
	UserInterface::Msg m;
	m.message = UserInterface::ModelUnloadReady;
	UserInterface::EventEnque (m );
	UserInterface::command = UserInterface::None;
}

void EventHandlers::UnloadScene(Button3d * button)
{
	DefaultCloseWindow(button);
	UserInterface::Msg m;
	m.message = UserInterface::SceneUnloadReady;
	UserInterface::EventEnque (m );
	UserInterface::command = UserInterface::None;
}

void EventHandlers::QuitProgram(Button3d * button)
{
	DefaultCloseWindow(button);
	UserInterface::Msg msg;
	msg.message = UserInterface::Exit;
	msg.dataInfo = reinterpret_cast<void *>(0);
	UserInterface::EventEnque(msg);
}

void EventHandlers::DefaultCloseWindow(Button3d * button)
{
	UserInterface::command = UserInterface::None;
	button->getParent()->state.setState(WindowState::WindowStates::CLOSING );
	button->getParent()->state.setKillFlag();
}