#include "eventhandlers.h"
#include "userinterface.h"

/* By checking for Command == NONE before preforming actions, this
will allow you to pop up only 1 window at a time, this
makes the ui much cleaner more reliable.  */

//Menu Operations

void EventHandlers::LoadScene(Menu * )
{
	if ( UserInterface::command == UserInterface::None)
	{			
		UserInterface::command = UserInterface::RequestLoadScene;
		UserInterface::AddOpenWindow("Open Scene");
	}
}
void EventHandlers::SaveScene(Menu * )
{
	if ( UserInterface::command == UserInterface::None)
	{			
		UserInterface::command = UserInterface::RequestSaveScene;
		UserInterface::AddOpenWindow("Save Scene");
	}
}
void EventHandlers::LoadModel(Menu * )
{
	if ( UserInterface::command == UserInterface::None)
	{
		UserInterface::command = UserInterface::RequestLoadModel;
		UserInterface::AddOpenWindow("Load Model");
	}
}
void EventHandlers::UnloadModel(Menu * )
{
	if ( UserInterface::command == UserInterface::None)
	{
		UserInterface::command = UserInterface::RequestUnloadModel;
		UserInterface::AddConfirmWindow("Unload", "Are you sure you want to remove\nthe object from the scene?", "Remove", "Cancel" );
	}
}
void EventHandlers::NewScene(Menu * )
{
	if ( UserInterface::command == UserInterface::None)
	{
		UserInterface::command = UserInterface::RequestUnloadScene;
		UserInterface::AddConfirmWindow("Unload", "Are you sure you want to remove\nthe the scene?" );
	}
}
void EventHandlers::Connect(Menu * )
{
	if ( UserInterface::command == UserInterface::None)
	{
		UserInterface::command = UserInterface::Command::RequestNew;
		UserInterface::AddNetConnectWindow("Network", "127.0.0.1", "1307");
	}
}
void EventHandlers::Quit(Menu * )
{
	if ( UserInterface::command == UserInterface::None)
	{
		UserInterface::command = UserInterface::RequestExit;
		UserInterface::AddConfirmWindow("Exit", "Do you wish to exit?");
	}
}

//Menu Object Settings
void EventHandlers::ModifyForces(Menu * )
{
	if ( UserInterface::command == UserInterface::None)
	{
		UserInterface::command = UserInterface::Command::RequestWorldForces;
		UserInterface::Msg m;
		m.message = UserInterface::Operation::SendWorldForces;
		UserInterface::EventEnque(m);
		UserInterface::AddObjectPhysics();
	}
}
void EventHandlers::ObjectProperties(Menu * )
{
	if ( UserInterface::command == UserInterface::None)
	{
		UserInterface::command = UserInterface::RequestEditObject;
		UserInterface::AddObjPropWindow();			
	}
}
void EventHandlers::RotateObject(Menu * )
{
	UserInterface::Msg m;
	m.message = UserInterface::SetManipState;
	m.specialString = "Rotate";
	UserInterface::EventEnque (m );
}

void EventHandlers::TranslateObject(Menu * )
{
	UserInterface::Msg m;
	m.message = UserInterface::SetManipState;
	m.specialString = "Translate";
	UserInterface::EventEnque (m );
}

void EventHandlers::ScaleObject(Menu * )
{
	UserInterface::Msg m;
	m.message = UserInterface::SetManipState;
	m.specialString = "Scale";
	UserInterface::EventEnque (m );
}

void EventHandlers::DuplicateSelectedObject(Menu * )
{
	UserInterface::Msg m;
	m.message = UserInterface::CloneObject;
	UserInterface::EventEnque(m);
}

void EventHandlers::DeselectSelectedObject(Menu * )
{
	UserInterface::Msg m;
	m.message = UserInterface::Deselect;
	UserInterface::setSelectedObject( -1 );
	UserInterface::EventEnque(m);
}

//Menu Settings
void EventHandlers::ToggleInnerGrid(Menu * menu)
{
	UserInterface::getMenuBar()->ToggleCheckMark("Inner Grid");

	if ( menu->isChecked() )
	{
		UserInterface::Msg m;
		m.message = UserInterface::Operation::EnableInnerGrid;
		m.specialString = menu->getCaption();
		UserInterface::EventEnque(m);
	}
	else 
	{
		UserInterface::Msg m;
		m.message = UserInterface::Operation::DisableInnerGrid;
		m.specialString = menu->getCaption();
		UserInterface::EventEnque(m);
	}
}
void EventHandlers::ToggleHoloGrid(Menu * menu)
{
	UserInterface::getMenuBar()->ToggleCheckMark("HoloGrid");

	if ( menu->isChecked() )
	{
		UserInterface::Msg m;
		m.message = UserInterface::Operation::EnableHoloGrid;
		m.specialString = menu->getCaption();
		UserInterface::EventEnque(m);
	}
	else 
	{
		UserInterface::Msg m;
		m.message = UserInterface::Operation::DisableHoloGrid;
		m.specialString = menu->getCaption();
		UserInterface::EventEnque(m);
	}
}
void EventHandlers::SetWireframeMode(Menu * menu)
{
	menu->setCaption("Textured");
	menu->AssignActionEvent(EventHandlers::SetTexturedMode);

	UserInterface::Msg m;
	m.message = UserInterface::GotoWireframe;
	UserInterface::EventEnque(m);
}
void EventHandlers::SetTexturedMode(Menu * menu)
{
	menu->setCaption("Wireframe");
	menu->AssignActionEvent(EventHandlers::SetWireframeMode);
	UserInterface::Msg m;
	m.message = UserInterface::GotoTextured;
	UserInterface::EventEnque(m);
}
void EventHandlers::ColorSettings(Menu * )
{
	if ( UserInterface::command == UserInterface::None)
	{
		UserInterface::command = UserInterface::RequestGridSettings;
		Window3d * gridWindow = UserInterface::AddConfirmWindow("Grid", "");
		TextBox3d * title = gridWindow->addText(30, 30, 325, 20);
		title->setReadOnly(true);
		title->setText("Please select a color scheme:");
		Button3d * whiteBG = gridWindow->addButton(Button3d::OK, 60, 60);
		whiteBG->AssignActionEvent(EventHandlers::GridSettings);
		Button3d * blackBG = gridWindow->addButton(Button3d::OK, 60, 80);
		blackBG->AssignActionEvent(EventHandlers::GridSettings);
		whiteBG->setCaption("White Backround");
		blackBG->setCaption("Black Backround");
	}
}

//View settings
void EventHandlers::SetView3d(Menu * menu)
{
	MenuBar * menuBar = UserInterface::getMenuBar();
	menuBar->UnCheck("3d");
	menuBar->UnCheck("Front");
	menuBar->UnCheck("Back");
	menuBar->UnCheck("Top");
	menuBar->UnCheck("Bottom");
	menuBar->UnCheck("Left");
	menuBar->UnCheck("Right");
	menuBar->ToggleCheckMark("3d");

	UserInterface::Msg m;
	m.message = UserInterface::SetView;
	m.specialString = menu->getCaption();
	UserInterface::EventEnque(m);
}
void EventHandlers::SetViewFront(Menu * menu)
{
	MenuBar * menuBar = UserInterface::getMenuBar();
	menuBar->UnCheck("3d");
	menuBar->UnCheck("Front");
	menuBar->UnCheck("Back");
	menuBar->UnCheck("Top");
	menuBar->UnCheck("Bottom");
	menuBar->UnCheck("Left");
	menuBar->UnCheck("Right");
	menuBar->ToggleCheckMark("Front");

	UserInterface::Msg m;
	m.message = UserInterface::SetView;
	m.specialString = menu->getCaption();
	UserInterface::EventEnque(m);
}
void EventHandlers::SetViewBack(Menu * menu)
{
	MenuBar * menuBar = UserInterface::getMenuBar();
	menuBar->UnCheck("3d");
	menuBar->UnCheck("Front");
	menuBar->UnCheck("Back");
	menuBar->UnCheck("Top");
	menuBar->UnCheck("Bottom");
	menuBar->UnCheck("Left");
	menuBar->UnCheck("Right");
	menuBar->ToggleCheckMark("Back");

	UserInterface::Msg m;
	m.message = UserInterface::SetView;
	m.specialString = menu->getCaption();
	UserInterface::EventEnque(m);
}
void EventHandlers::SetViewLeft(Menu * menu)
{
	MenuBar * menuBar = UserInterface::getMenuBar();
	menuBar->UnCheck("3d");
	menuBar->UnCheck("Front");
	menuBar->UnCheck("Back");
	menuBar->UnCheck("Top");
	menuBar->UnCheck("Bottom");
	menuBar->UnCheck("Left");
	menuBar->UnCheck("Right");
	menuBar->ToggleCheckMark("Left");

	UserInterface::Msg m;
	m.message = UserInterface::SetView;
	m.specialString = menu->getCaption();
	UserInterface::EventEnque(m);
}
void EventHandlers::SetViewRight(Menu * menu)
{
	MenuBar * menuBar = UserInterface::getMenuBar();
	menuBar->UnCheck("3d");
	menuBar->UnCheck("Front");
	menuBar->UnCheck("Back");
	menuBar->UnCheck("Top");
	menuBar->UnCheck("Bottom");
	menuBar->UnCheck("Left");
	menuBar->UnCheck("Right");
	menuBar->ToggleCheckMark("Right");

	UserInterface::Msg m;
	m.message = UserInterface::SetView;
	m.specialString = menu->getCaption();
	UserInterface::EventEnque(m);
}
void EventHandlers::SetViewTop(Menu * menu)
{
	MenuBar * menuBar = UserInterface::getMenuBar();
	menuBar->UnCheck("3d");
	menuBar->UnCheck("Front");
	menuBar->UnCheck("Back");
	menuBar->UnCheck("Top");
	menuBar->UnCheck("Bottom");
	menuBar->UnCheck("Left");
	menuBar->UnCheck("Right");
	menuBar->ToggleCheckMark("Top");

	UserInterface::Msg m;
	m.message = UserInterface::SetView;
	m.specialString = menu->getCaption();
	UserInterface::EventEnque(m);
}
void EventHandlers::SetViewBottom(Menu * menu)
{
	MenuBar * menuBar = UserInterface::getMenuBar();
	menuBar->UnCheck("3d");
	menuBar->UnCheck("Front");
	menuBar->UnCheck("Back");
	menuBar->UnCheck("Top");
	menuBar->UnCheck("Bottom");
	menuBar->UnCheck("Left");
	menuBar->UnCheck("Right");
	menuBar->ToggleCheckMark("Bottom");

	UserInterface::Msg m;
	m.message = UserInterface::SetView;
	m.specialString = menu->getCaption();
	UserInterface::EventEnque(m);
}

void EventHandlers::AddWorldForcesMenu(Menu * )
{
	float line1 = 70;
	float line2 = 110;

	float line4 = 230;
	Window3d * window = UserInterface::AddWorldForceWindow();

	TextBox3d * top = window->addText(30, 30, 200, 20);
	top ->setReadOnly(true);
	top->setText("Force Direction");

	window->vX = window->addText(50, line1, 60, 20);
	window->vY = window->addText(150, line1, 60, 20);
	window->vZ = window->addText(250, line1, 60, 20);

	TextBox3d * lblX = window->addText(15, (int)line1, 20, 20);
	lblX->setReadOnly(true);
	lblX->setText("X:");

	TextBox3d * lblY = window->addText(115, (int)line1, 20, 20);
	lblY->setReadOnly(true);
	lblY->setText("Y:");

	TextBox3d * lblZ = window->addText(215, (int)line1, 20, 20);
	lblZ->setReadOnly(true);
	lblZ->setText("Z:");

	TextBox3d * lblDur = window->addText(30, (int)line2, 150, 20);
	lblDur->setReadOnly(true);
	lblDur->setText("Duration:");
	window->forceDur = window->addText(30, (int)line2 + 30, 50, 20);



	TextBox3d * lblName = window->addText(30, line4 , 150, 20);
	lblName->setReadOnly(true);
	lblName->setText("Name:");
	window->forceName = window->addText(30, (int)line4 + 30, 50, 20);

	window->vX->setDigit(true);
	window->vY->setDigit(true);
	window->vZ->setDigit(true);
	window->forceDur->setDigit(true);
	window->forceMass->setDigit(true);

	window->addButton(Button3d::OK, 240, 180) -> AssignActionEvent(EventHandlers::AddWorldForce);

}

void EventHandlers::DefaultMenuEvent(Menu * )
{

}

void EventHandlers::ControlCamera(Menu *)
{
	UserInterface::Msg m;
	m.message = UserInterface::ControlCamera;
	UserInterface::EventEnque(m);
}

void EventHandlers::LookFromCamera(Menu *)
{
	UserInterface::Msg m;
	m.message = UserInterface::ViewCamera;
	UserInterface::EventEnque(m);
}
