/* inputhandler.cpp - Most important file of the program.
	This file houses the main loop that the user needs to edit for display.
	Handle Keys and Mouse.

	Written by Mark Tulewicz, Wiktor Kopec, Maksim Fridberg.
*/


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdio.h>

#include "inputhandler.h"
#include "display.h"
#include "mvector3.h"
#include "movableobject.h"
#include "console.h"
#include "settings.h"
#include "wtime.h"
#include "model.h"
#include "wfunctions.h"
#include "modelmanip.h"
#include "controller.h"
#include "thread.h"
#include "client.h"
#include "utility.h"
#include "PNode.h"
#include "SpaceVector.h"
#include "forcemanager.h"

using namespace std;

ViewPort * InputHandler::view = NULL;
Model * InputHandler::model = NULL;
Client * InputHandler::client = NULL;
CameraManager InputHandler::cameras;

Client::ClientState g_clientState;
Client::ClientState g_connectHeader;

static PNode pnode(50);
static Force gravityForce;
static Force thrustForce;

InputHandler::InputHandler() {}

void InputHandler::Init(HWND handle)
{		
	RECT rect;
	GetClientRect(handle, &rect);
	Settings::WindowWidth = rect.right  - rect.left;
	Settings::WindowHeight = rect.bottom - rect.top;

	UserInterface::Init(handle);
	Time::Start();

	g_connectHeader.buffer = new char[1024];
	g_connectHeader.currentHeader.request = Header::Connect;
	g_connectHeader.total = sizeof(Header);
	g_connectHeader.processed = 0;
	Utility::Serialize(g_connectHeader.currentHeader, g_connectHeader.buffer);
	
	g_clientState.buffer = new char[1024];
	g_clientState.currentHeader.request = Header::Position;
	g_clientState.currentHeader.size = 24;
	g_clientState.total = sizeof(Header) + 24;
	g_clientState.processed = 0;	
	Utility::Serialize(g_clientState.currentHeader, g_clientState.buffer);

	client -> EnqueueSendData(&g_connectHeader);

	gravityForce.force_vect = SpaceVector(0, -( 9.8 * 50 ), 0);
	gravityForce.impulse = false;

	thrustForce.force_vect = SpaceVector(0, 1000, 0);
	thrustForce.impulse = true;
	thrustForce.duration = 3;

//	Camera * cam1 = cameras.AddCamera(model);
	Camera * cam2 = cameras.AddCamera(model);

	cam2->Pos() = MVector3(10,10,10);
	cam2->SyncModel();

	pnode.AddForce(thrustForce);
	pnode.AddForce(gravityForce);
}

void InputHandler::HandleChar(unsigned char key)
{
	if ( Settings::system == States::World::CameraControl)
	{
		return;
	}


	if ( !UserInterface::UserInterfaceActive() )
	{
		switch (key)
		{
			case 'Q':
			case 'q':
				if ( UserInterface::command == UserInterface::None)
				{
					UserInterface::command = UserInterface::RequestExit;
					UserInterface::AddConfirmWindow("Exit", "Do you wish to exit?");
				}
				break;
			case 'R':
				break;

			case 'E':
				UserInterface::AddMessageBox("Oh NO!!", "You pressed \'E\'!!");
				break;

			case 'g':
			case 'G':	
				//Toggle the grid state machine.
				if ( Settings::Grid::gridState == States::Grid::ON )
				{
					Settings::Grid::gridState = States::Grid::Disable;
				}
				else if ( Settings::Grid::gridState == States::Grid::OFF )
				{
					Settings::Grid::gridState = States::Grid::Enable;
				}
				break;

			case 'P':
				break;
			default:
				break;
		}
	}
	else
	{
		UserInterface::sendKeyPress(key, Controller<InputHandler>::isDown(VK_SHIFT));
	}
}

void InputHandler::HandleKey(unsigned char keycode)
{
	//Camera control state is exclusive.
	if ( Settings::system == States::World::CameraControl)
	{
		if ( keycode == VK_ESCAPE )
		{
			Settings::system = Settings::oldSystemState;
			cameras.GetActive()->SyncModel();
			cameras.GetActive()->setControl(false);
			cameras.setCamera( CameraManager::GetWorldCamera()->GetModel() );
			view->setCamera( *CameraManager::GetWorldCamera() );
			
			return;
		}
		return;
	}

	switch(keycode)
	{
		case ' ':
			UserInterface::ToggleMainMenu();
			break;
		//case 'P':
		//	if (Settings::mouseOnObject > -1)
		//	{
		//		RenderableObject * dobject = dynamic_cast<RenderableObject *>(&(*model)[Settings::mouseOnObject]);
		//		if (dobject)
		//		{
		//			dobject -> AddPhysicsNode(pnode);
		//		}
		//	}
		//	break;
		case 'C':
			{
//				RenderableObject * dobject = dynamic_cast<RenderableObject *>(&(*model)[Settings::mouseOnObject]);
//				if (dobject)
//				{
//					if (dobject -> getPhysicsNode())
//					{
//						dobject -> getPhysicsNode() -> Collision(100, SpaceVector(0, 1, 0));
//					}
//				}
			}
			break;
		case VK_UP:
			UserInterface::SelectPrevBox();
			break;
		case VK_DOWN:
			UserInterface::SelectNextBox();
			break;
		case VK_LEFT:
			UserInterface::MoveTextCursor(-1);
			break;
		case VK_RIGHT:
			UserInterface::MoveTextCursor(1);
			break;
		case VK_DELETE:
			UserInterface::sendKeyPress(127, Controller<InputHandler>::isDown(VK_SHIFT));
			break;
		default:
			break;
	}
}

/*
void InputHandler::HandleGlove(const GloveDelegate<InputHandler>::GloveState & gloveState)
{
	
}
*/

void InputHandler::HandleMouse(int relX, int relY, int relZ, int x, int y, unsigned char buttons[])
{	
	
	if ( Settings::system != States::World::CameraControl)
	{
		//Windowing commands
		UserInterface::PerformMouseEvents(relX, relY, relZ, x, y, buttons);

		//Selection buffer:
		InputHandler::SelectObjectsWithMouse(x, y);

		//Context Menus
		if ( UserInterface::OnMouseClick(1) && !UserInterface::UserInterfaceActive() )
		{
			MenuBar * context = UserInterface::AddContextMenuObject();
			//You can have any type of context menu at any time.
			if ( Settings::mouseOnObject >= 128 )
			{
				RenderableObject * dobject = dynamic_cast<RenderableObject *>(&(*model)[Settings::mouseOnObject]);
				if (dobject)
				{
					if ( cameras.IsCamera(dobject) )
					{
						//Object is actually a camera
						context->addMenuAcross("Camera");
						context->addMenuDown("Translate", 0) ->AssignActionEvent((MenuEvent)EventHandlers::TranslateObject);
						context->addMenuDown("View", 0) -> AssignActionEvent((MenuEvent)EventHandlers::LookFromCamera);
						context->addMenuDown("Control", 0) -> AssignActionEvent((MenuEvent)EventHandlers::ControlCamera);
					}
					else
					{
						//Regular object
						context->addMenuAcross("Object");
						
						context->addMenuDown("Translate", 0) -> AssignActionEvent((MenuEvent)EventHandlers::TranslateObject);
						context->addMenuDown("Scale", 0) -> AssignActionEvent((MenuEvent)EventHandlers::ScaleObject);
						context->addMenuDown("Rotate", 0) -> AssignActionEvent((MenuEvent)EventHandlers::RotateObject);
						context->addMenuDown("Properties", 0) -> AssignActionEvent((MenuEvent)EventHandlers::ObjectProperties);
						context->addMenuDown("Modify Forces", 0) -> AssignActionEvent((MenuEvent)EventHandlers::ModifyForces);
						context->addMenuDown(" ", 0);
						context->addMenuDown("Duplicate", 0) -> AssignActionEvent((MenuEvent)EventHandlers::DuplicateSelectedObject);
						context->addMenuDown("Deselect", 0) -> AssignActionEvent((MenuEvent)EventHandlers::DeselectSelectedObject);
						context->addMenuDown(" ", 0);		
						context->addMenuDown(String("Unload"), 0)->AssignActionEvent((MenuEvent) EventHandlers::UnloadModel );
					}
				}

			}
			else if ( Settings::mouseOnObject == -1 )
			{
				context->addMenuAcross("Scene");
				context->addMenuDown("Open", 0) -> AssignActionEvent((MenuEvent)EventHandlers::LoadScene);
				context->addMenuDown("Exit", 0) -> AssignActionEvent((MenuEvent)EventHandlers::Quit);
				context->addMenuAcross("Model");
				context->addMenuDown("Load", 1) -> AssignActionEvent((MenuEvent)EventHandlers::LoadModel);			
			}
			else if ( Settings::mouseOnObject == 0 )
			{
				context->addMenuAcross("X Selected");
				context->addMenuDown("Exit", 0) -> AssignActionEvent((MenuEvent)EventHandlers::Quit);
			}
			else if ( Settings::mouseOnObject > 0 && Settings::mouseOnObject < 10 ) 
			{
				context->addMenuAcross("Manipulator");
				context->addMenuDown("Done", 0) -> AssignActionEvent((MenuEvent)EventHandlers::DeselectSelectedObject);
				

			}
			else
			{
				context->addMenuAcross("Error");
				context->addMenuDown("We Should not be here", 0) -> AssignActionEvent(EventHandlers::Quit);
			}
		}

	}
	// END OF WINDOW CONTROL SETUPS -------------

	if ( Settings::system == States::World::CameraControl || (!UserInterface::UserInterfaceActive() && !ModelManipulator::ManipActive()))
	{
		//Mouse look controls the objectIn focus::
		//Object & object = (view -> getCamera());
		//MVector3 rotationVector = object.view - object.pos;

		static bool Once = false;

		//Filtered X
		double multiplier = 1.0f;
		if ( ((relX != 0) || (relY != 0)))
		{
			if ( abs(relX) <= 1 )
				view->getCamera().Theta() -= (double)(relX / 1000.0) * multiplier;
			else if ( abs(relX) <= 2 )
				view->getCamera().Theta() -= (double)(relX / 800.0) * multiplier;
			else if ( abs(relX) <= 6)
				view->getCamera().Theta() -= (double)(relX / 500.0) * multiplier;
			else if ( abs(relX) <= 8 )
				view->getCamera().Theta() -= (double)(relX / 400.0) * multiplier;

			else if ( abs(relX) <= 16 )
				view->getCamera().Theta() -= (double)(relX / 300.0) * multiplier;

			else if ( abs(relX) <= 32  )
				view->getCamera().Theta() -= (double)(relX / 200.0) * multiplier;
			else
				view->getCamera().Theta() -= (double)(relX / 100.0) * multiplier;

			//Filtered Y
			if ( abs(relY) <= 1 )
				view->getCamera().Phi() += (double)(relY / 1000.0) * multiplier;
			else if ( abs(relY) <= 2 )
				view->getCamera().Phi() += (double)(relY / 800.0) * multiplier;
			else if ( abs(relY) <= 6)
				view->getCamera().Phi() += (double)(relY / 500.0) * multiplier;
			else if ( abs(relY) <= 8 )
				view->getCamera().Phi() += (double)(relY / 400.0) * multiplier;

			else if ( abs(relY) <= 16 )
				view->getCamera().Phi() += (double)(relY / 300.0) * multiplier;

			else if ( abs(relY) <= 32 )
				view->getCamera().Phi() += (double)(relY / 200.0) * multiplier;
			else
				view->getCamera().Phi() += (double)(relY / 100.0) * multiplier;
			
			
			if ( view->getCamera().Phi() > HALFPI - 0.08 )
			{
				view->getCamera().Phi() = HALFPI - 0.08;
			}
			else if ( view->getCamera().Phi() < -HALFPI + 0.08 )
			{
				view->getCamera().Phi() = -HALFPI + 0.08;
			}		
		//	camera->RotateYBy(view-);
			//object.view = object.pos + MVector3::rotate(rotationVector, view->theta, view->phi + HALFPI);
		}
		
		if ( ! Once )
		{
			//Any one time runs can go here.
			Once = true;
		}
	}
}

void InputHandler::SelectObjectsWithMouse(int x, int y)
{
	//in the event of a click, select an object.
	if ( (UserInterface::OnMouseClick(0) && !UserInterface::OnMouseClick(1) && !UserInterface::OnMouseClick(2)) && !UserInterface::UserInterfaceActive() )	
	{
		GLuint buffer[512];
		GLint viewport[4];

		GLint hits = 0;
		glGetIntegerv(GL_VIEWPORT,viewport);
		glSelectBuffer(512,buffer);

		(void)glRenderMode(GL_SELECT);

		glInitNames();
		glPushName(0);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		{
			glLoadIdentity();	
			gluPickMatrix(x, (viewport[3] - y), 1.0f, 1.0f, viewport);
			
			view->RenderView();
//			gluPerspective(view->getFov(), 1024.0f / 768.0f, 1, 4000);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			{		
				
				glLoadIdentity();	
			
				view->update();
				
				model->Draw();
				ModelManipulator::DrawManipulator(false);
				hits = glRenderMode(GL_RENDER);

				//Hits is the # of objects that were under the mouse.
				if (hits > 0 )
				{
					int	choose = buffer[3];
					int depth = buffer[1];									// Store How Far Away It Is 

					for (int i = 1; i < hits; i++)					// Loop Through All The Detected Hits
					{
						
						//If it is a modifier (trans scale rot) select that instead.
						if ( buffer[i*4+3] < 10 && buffer[i*4+3] > 0 )
						{
							choose = buffer[i*4+3];
							break;
						}

						// If This Object Is Closer To Us Than The One We Have Selected
						if (buffer[i*4+1] < GLuint(depth))
						{
							choose = buffer[i*4+3];						// Select The Closer Object
							depth = buffer[i*4+1];						// Store How Far Away It Is
						}       
					}

					Settings::mouseOnObject = choose;
					UserInterface::setSelectedObject(choose);
				
					cout << "You have selected object " << choose << endl;

					if ( choose == 1 || choose == 4 || choose == 7 )
					{
						cout << "X Object moused" << endl;	
					}
					else if ( choose == 2 || choose == 5 || choose == 8 )
					{
						cout << "y Object moused" << endl;
					}
					else if ( choose == 3 || choose == 6 || choose == 9 )
					{
						cout << "z Object moused" << endl;
					}
					else if ( choose == 0 )
					{
						Settings::mouseOnObject = -1;
						UserInterface::setSelectedObject(-1);
						ModelManipulator::SetState(ModelManipulator::INVISIBLE);
					}
					else
					{
						ModelManipulator::AddObject((*model)[choose], true);
					}
				}
				else
				{
					Settings::mouseOnObject = -1;
					UserInterface::setSelectedObject(-1);
					ModelManipulator::SetState(ModelManipulator::INVISIBLE);
				//	view->unFocus();
				}
				
			}
			glPopMatrix();
		}
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}
}


void InputHandler::HandleKeys(unsigned char keys [])
{
	//TODO: Put all these as functions into view.cpp
	if ( !UserInterface::UserInterfaceActive()  || Settings::system == States::World::CameraControl)
	{
		Camera & object = (view -> getCamera());	
		MVector3 mov = object.View() - object.Pos();

		MVector3 forceAccum;

		mov /= mov.GenerateMag() * 1.0f;

		if ( view->viewType == ViewPort::Perspective)
		{
			if (!(keys[0x11] & 0x80) && !(keys[0x1F] & 0x80) && !(keys[0x1E] & 0x80) && !(keys[0x20] & 0x80) )
			{
				Settings::currentSpeed = 0;
			}

			if (keys[0x11] & 0x80)
			{	
				Settings::currentSpeed +=  Settings::accelSpeed * (float)Time::getInterval();

				if ( Settings::currentSpeed >= Settings::maxPlayerSpeed )
				{
					Settings::currentSpeed = Settings::maxPlayerSpeed;
				}

				forceAccum += mov;
			}
			
			if (keys[0x1F] & 0x80)
			{
				Settings::currentSpeed += Settings::accelSpeed * (float)Time::getInterval();

				if ( Settings::currentSpeed >= Settings::maxPlayerSpeed )
				{
					Settings::currentSpeed = Settings::maxPlayerSpeed;
				}

				forceAccum -= mov;
			}
			if (keys[0x1E] & 0x80)
			{		
				Settings::currentSpeed += Settings::accelSpeed * (float)Time::getInterval();

				if ( Settings::currentSpeed >= Settings::maxPlayerSpeed )
				{
					Settings::currentSpeed = Settings::maxPlayerSpeed;
				}

				forceAccum.x += mov.z;
				forceAccum.z += -mov.x;
			}
			
			if (keys[0x20] & 0x80)
			{
				Settings::currentSpeed += Settings::accelSpeed * (float)Time::getInterval();

				if ( Settings::currentSpeed >= Settings::maxPlayerSpeed )
				{
					Settings::currentSpeed = Settings::maxPlayerSpeed;
				}

				forceAccum.x += -mov.z;
				forceAccum.z += mov.x;
			}
			forceAccum /= forceAccum.GenerateMag();
			forceAccum *= Settings::currentSpeed;
		}
		//If front, right, back or left.
		else if ( view->viewType != ViewPort::Top && view->viewType != ViewPort::Bottom)
		{	
			if (keys[0x11] & 0x80)
			{	
				forceAccum += object.Up() * view->ZoomPercent(10.0);

			}
			
			if (keys[0x1F] & 0x80)
			{
				forceAccum -= object.Up() * view->ZoomPercent(10.0);
			}

			if (keys[0x1E] & 0x80)
			{		
				forceAccum.x += mov.z * view->ZoomPercent(10.0);;
				forceAccum.z += -mov.x * view->ZoomPercent(10.0);;
			}
			if (keys[0x20] & 0x80)
			{
				forceAccum.x += -mov.z * view->ZoomPercent(10.0);;
				forceAccum.z += mov.x * view->ZoomPercent(10.0);;
			}
		}
		//if top or bottom
		else if (view->viewType == ViewPort::Top || view->viewType == ViewPort::Bottom)
		{
			if (keys[0x11] & 0x80)
			{	
				forceAccum += object.Up() * view->ZoomPercent(10.0);
			}
			
			if (keys[0x1F] & 0x80)
			{
				forceAccum -= object.Up() * view->ZoomPercent(10.0);
			}
			
			if (keys[0x1E] & 0x80)
			{		
				forceAccum.z += -1 * view->ZoomPercent(10.0);;
			}
			
			if (keys[0x20] & 0x80)
			{
				forceAccum.z += 1 * view->ZoomPercent(10.0);;
			}
		}

		if (keys[ DIK_LCONTROL] & 0x80)
		{

			SetCursorPos(Settings::WindowWidth / 2, Settings::WindowHeight / 2 );
		}

		object.Pos() += forceAccum * (float)Time::getInterval() * 8.0f;
		object.View() += forceAccum * (float)Time::getInterval() * 8.0f;

	}
}

void InputHandler::HandleDisplay()
{			
	glClearDepth(1.0f);
	glClearColor(Settings::Grid::bred, Settings::Grid::bgreen, Settings::Grid::bblue, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//Update the view's position.
//	Physics::ApplyForces(view->getCamera());
	view -> update();
	model->Draw();

	if ( Settings::system != States::World::CameraControl)
	{
		ModelManipulator::DrawManipulator(true);
	}

	//Used if you want to see how long it takes to render 1 frame of info.
	//Time::Start();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	{
		view->DrawGrid();
	}
	glPopMatrix();


	view->DrawAxis();

	if ( Settings::system != States::World::CameraControl)
	{

		//Interface controls:
		UserInterface::DisplayWindows();

		//MovableObject miniCam(MVector3(10,10,0), MVector3(0,0,0));

		cameras.DisplayQuickCams(view, model);	
		glColor3ub(255,255,255);
	}

	PostDisplayCleanUp();		

//	Utility::Serialize(view ->getCamera().Pos().asArray(), 3, &g_clientState.buffer[8]);
//	Utility::Serialize(view ->getCamera().View().asArray(), 3, &g_clientState.buffer[8 + sizeof(float) * 3]);
	
//	client -> EnqueueSendData(&g_clientState);
}

void InputHandler::PostDisplayCleanUp()
{
	model->CollisionTest();

	ModelManipulator::setDistance(&view->getCamera());

	String fps = String("FPS: ") + String::ToString(Time::getFPS());
	//UserInterface::UpdateMessageBox((char *) (String("FPS = ") + fps).getCString());

	bool reenable = false;
	if ( ( reenable = Settings::isDepthTestOn()) )
	{
		glDisable(GL_DEPTH_TEST);
	}

	glColor3ub(255,0,0);
	UserInterface::PrintGL(Settings::WindowWidth - 128, 24, fps);
	
	if ( Settings::system == States::World::CameraControl)
	{
		UserInterface::PrintGL(30, 44, "Camera Control Mode, Press ESC when camera is positioned.");
		UserInterface::PrintGL(30, 66, String("Phi: ") + String::ToString(cameras.GetActive()->Phi() * (180 / PI)));
		UserInterface::PrintGL(30, 86, String("Theta: ") + String::ToString(cameras.GetActive()->Theta() * (180 / PI)));
	}

	glColor3ub(255,255,255);

	if ( reenable )
	{
		glEnable(GL_DEPTH_TEST);
	}

	//FINALIZE and draw to Monitor.
	Display::SwapBuffers();

	Time::updateTime();
	if ( Settings::system != States::World::CameraControl)
	{
		model->ApplyForces();
	}

	//Message Handler between UI and system. EVENT HANDLER
	UserInterface::Msg event;
	if (UserInterface::CheckForMessages(event))
	{	
		switch(event.message)
		{
			case UserInterface::Operation::AddWorldForce:
				{					
					EventHandlers::ForceInfo * forceInfo = (EventHandlers::ForceInfo *)event.dataInfo;
					//TODO replace with overloaded constructors once they become available
					//TODO resolve mass issues
					Force * force = new Force();
					
					force -> duration = forceInfo ->duration;
					force -> force_vect = SpaceVector(forceInfo ->x, forceInfo ->y, forceInfo ->z);					
					force -> setName(event.specialString.getCString());					
					force -> impulse = ( (force ->duration) ? true : false);
					force -> scalar_attrib = 1;

					ForceManager::AddForce(force);

					delete forceInfo;
				}
				break;	
			case UserInterface::Operation::SendWorldForces:
				{
					//TODO conversion between string and String, unify data types
					vector<String> forces;
					for (int i = 0; i < (int)ForceManager::getForces().size(); i++)
					{
						forces.push_back(((ForceManager::getForces())[i] -> getName()).c_str());
					}
					UserInterface::SendForceList(forces);
				}
				break;

			case UserInterface::Operation::AddForce:
				{
					//TODO: Special String has the name of the Force being added
					//Settings::mouseOnObject has the number of the selected object;
									
					RenderableObject * dobject = dynamic_cast<RenderableObject *>(&(*model)[Settings::mouseOnObject]);
					
					if (dobject)
					{
						PNode * pnode = dobject -> getPhysicsNode();
						if (pnode)
						{
							//pnode ->getfor
							//pnode -> AddForce(ForceManager::getForceByName(event.specialString));
						}
						else
						{
					//		dobject -> AddPhysicsNode(new PNode(
						}
					}
				}
				break;
			case UserInterface::Operation::RemoveForce:
				{
					//TODO: Special String has the name of the Force being removed
					// Settings::mouseOnObject has the number of the selected object;
					
					event.specialString;

				}
				break;

			case UserInterface::ModelLoadReady:
				{
					String fileToOpen;
					fileToOpen = event.specialString;
					/*split this up into path, object, extension*/
					Vector <String> paths;
					Split(fileToOpen,paths,'\\');
					
					String path;
					for ( int i = 0 ; i < paths.getSize() - 1; i++)
					{
						path += paths[i] + "\\"; 
					}
					path.TrimEnd(1);					
					
					String objectAndExt = paths[paths.getSize() - 1];

					Vector<String> objectExt;
					Split(objectAndExt, objectExt, '.');
					
					if ( objectExt.getSize() < 2 )
					{
						break;
					}

					model->Open(objectExt[0], objectExt[1], path);
				}
				break;
			case UserInterface::ModelModReady:
				{
					if ( Settings::mouseOnObject > Object::getIDOffset() )
					{
						float * data = static_cast<float *>(event.dataInfo);

						RenderableObject * dobject = dynamic_cast<RenderableObject *>(&(*model)[Settings::mouseOnObject]);

						if (dobject)
						{
							dobject->RotateXBy(data[0]);
							dobject->RotateYBy(data[1]);
							dobject->RotateZBy(data[2]);

							dobject->ScaleTo(data[3], data[4], data[5]);
							dobject->TranslateTo(data[6],data[7],data[8]);
						}
					}
				}
				break;
			case UserInterface::RequestModelInfo:
				{			
					if ( Settings::mouseOnObject > Object::getIDOffset() )
					{
						RenderableObject * dobject = dynamic_cast<RenderableObject *>(&(*model)[Settings::mouseOnObject]);
						
						if (dobject)
						{													
							float rotate[3];
							rotate[0] = 0.0f;
							rotate[1] = 0.0f;
							rotate[2] = 0.0f;

							float scale[3];
							dobject ->getScale(scale[0], scale[1], scale[2]);
							
							float trans[3];
							dobject -> getTranslate(trans[0], trans[1], trans[2]);

							UserInterface::SendModelInfo(rotate, scale, trans, dobject->getData().name, dobject->getData().comments);
						}
					}
				}
				break;
			case UserInterface::SceneLoadReady:
				{
					ModelManipulator::ClearObjects();
					model -> LoadScene(event.specialString);
				}
				break;

			case UserInterface::SceneUnloadReady:
				{
					ModelManipulator::ClearObjects();
					model -> CloseScene();
				}
				break;

			case UserInterface::Operation::EnableInnerGrid:
				{
					Settings::InnerGrid = true;
				}
				break;

			case UserInterface::Operation::DisableInnerGrid:
				{
					Settings::InnerGrid = false;
				}
				break;
			case UserInterface::Operation::DisableHoloGrid:
				{
					if ( Settings::Grid::gridState == States::Grid::ON )
					{
						Settings::Grid::gridState = States::Grid::Disable;
					}
				}
				break;
			case UserInterface::Operation::EnableHoloGrid:
				{	
					if ( Settings::Grid::gridState == States::Grid::OFF )
					{
						Settings::Grid::gridState = States::Grid::Enable;
					}
				}
				break;
			case UserInterface::ModelUnloadReady:
				ModelManipulator::ClearObjects();
				model ->Close(Settings::mouseOnObject);
				break;
			case UserInterface::SceneSaveReady:
				{
					model ->SaveScene(event.specialString);
				}
				break;
			case UserInterface::SetManipState:
				{
					if ( event.specialString == String("Rotate") )
					{
						ModelManipulator::SetState(ModelManipulator::ROTATE);	
					}
					else if ( event.specialString == String("Scale"))
					{
						ModelManipulator::SetState(ModelManipulator::SCALE);	
					}
					else if (event.specialString == String("Translate") )
					{
						ModelManipulator::SetState(ModelManipulator::TRANSLATE);	
					}
				}
				break;
			case UserInterface::SetFOV:
				{
					float * subtract = static_cast<float *> (event.dataInfo);
					if ( view->viewType == ViewPort::Perspective )
					{
						view->init(Settings::WindowWidth, Settings::WindowHeight, view->getFov() - subtract[0]);
					}
					else
					{
						view->ZoomIn( subtract[0] * view->ZoomPercent(25.0f) );
						view->UpdateProjectionMatrix();
					}

				}
				break;
			case UserInterface::CloneObject:
				{
					if ( Settings::mouseOnObject > Object::getIDOffset() )
					{
						RenderableObject * dobject = dynamic_cast<RenderableObject *>(&(*model)[Settings::mouseOnObject]);
						
						if (dobject)
						{
							model -> Clone(*dobject);
						}
					}
				}
				break;
			case UserInterface::SetView:
				{
					if ( event.specialString == String("3d") )
					{
						view->MakePerspective();
					}
					else if ( event.specialString == String("Front") )
					{
						view->MakeOrthoFront();
					}
					else if ( event.specialString == String("Back") )
					{
						view->MakeOrthoBack();
					}
					else if ( event.specialString == String("Left") )
					{
						view->MakeOrthoLeft();
					}
					else if ( event.specialString == String("Right") )
					{
						view->MakeOrthoRight();
					}
					else if ( event.specialString == String("Top") )
					{
						view->MakeOrthoTop();
					}
					else if ( event.specialString == String("Bottom") )
					{
						view->MakeOrthoBottom();
					}
				}
				break;

			case UserInterface::GotoWireframe:
				{
					//Mew menu Colors
					//UserInterface::setMenuBorderColorUpperOn(0, 255, 0);
					//UserInterface::setMenuBorderColorLowerOn(0, 128, 0);
					//UserInterface::setMenuBorderColorUpperOff(255, 0,0);
					//UserInterface::setMenuBorderColorLowerOff(128,0,0);
					//UserInterface::setMenuTextColor(0, 200, 0);
					//UserInterface::setMenuSelectedTextColor(0, 255, 0);
	
					//UserInterface::setFolderColor(0,0,255);
					//UserInterface::setTextColor(0,255,0);
					Settings::WireframeMode = true;
				}
				break;
			case UserInterface::GotoTextured:
				{
					//UserInterface::setMenuBorderColorUpperOn(255, 255, 255);
					//UserInterface::setMenuBorderColorLowerOn(255 / 2, 255 / 2, 255 /2);
					//UserInterface::setMenuBorderColorUpperOff(128, 128, 128);
					//UserInterface::setMenuBorderColorLowerOff(64, 64, 64);
					//UserInterface::setMenuTextColor(30, 30, 30);
					//UserInterface::setMenuSelectedTextColor(255, 255, 255);
	
					//UserInterface::setFolderColor(0,0,230);
					//UserInterface::setTextColor(30,30,30);
					Settings::WireframeMode = false;
				}
				break;
			case UserInterface::Deselect:
				{
					Settings::mouseOnObject = -1;
					UserInterface::setSelectedObject(-1);
					ModelManipulator::SetState(ModelManipulator::INVISIBLE);
				}
				break;
			case UserInterface::SelectGridColorScheme :
				{
					if ( event.specialString == "White")
					{
						Settings::Grid::SetGridBackColor(255,255,255);
						Settings::Grid::SetGridColor(128,128,128);
					}
					else if ( event.specialString == "Black")
					{
						Settings::Grid::SetGridBackColor(0,0,0);
						Settings::Grid::SetGridColor(255,255,0);
					}
				}
				break;
			case UserInterface::ConnectionRequest:
				{
					int result = client -> Connect(event.specialString.getCString(), Client::Port);
					
					if (result == -1)
					{													
						UserInterface::AddConfirmWindow("Error", "Could not connect to server", "Retry", "Cancel");
					}
					else
					{						
						Thread<Client> * recvThread = new Thread<Client>(Client::ReceiveHandler, client, NULL);
						recvThread -> Run();
						Thread<Client> * sendThread = new Thread<Client>(Client::SendHandler, client, NULL);
						sendThread -> Run();
						client -> Ready() = true;
					}
				}
				break;
			case UserInterface::ControlCamera:
				{
					if ( Settings::mouseOnObject >= 128 )
					{
						RenderableObject * dobject = dynamic_cast<RenderableObject *>(&(*model)[Settings::mouseOnObject]);
						if (dobject)
						{
							if ( cameras.IsCamera(dobject) )
							{
								//dobject is the selected object
								Settings::oldSystemState = Settings::system;
								Settings::system = States::World::CameraControl;
								cameras.setCamera(dobject);
								view->setCamera(*cameras.GetActive());
								cameras.GetActive()->setControl(true);
							}
						}
					}
				}
				break;
			case UserInterface::Operation::ViewCamera:
				{
					if ( Settings::mouseOnObject >= 128 )
					{
						RenderableObject * dobject = dynamic_cast<RenderableObject *>(&(*model)[Settings::mouseOnObject]);
						if (dobject)
						{
							if ( cameras.IsCamera(dobject) )
							{
								//dobject is the selected object
								//Settings::oldSystemState = Settings::system;
								//Settings::system = States::World::CameraControl;
								cameras.setCamera(dobject);
								cameras.GetActive()->ViewportOn(true);
								//view->setCamera(*cameras.GetActive());
							}
						}
					}
				}
				break;
			case UserInterface::Operation::WindowClosed:
				{
					cameras.DisconnectWindows(event.dataInfo);
				}
				break;
			case UserInterface::Exit:
				Controller<InputHandler>::PostQuitMessage(reinterpret_cast<int>(event.dataInfo));				
				break;
			default:
				return;
		}
	}

	//END OF EVENT HANDLER

}

void InputHandler::setView(ViewPort & newView)
{
	view = &newView;
}

void InputHandler::setModel(Model & model)
{
	InputHandler::model = &model;
}

void InputHandler::setClient(Client & client)
{
	InputHandler::client = &client;
}

CameraManager * InputHandler::GetCameras()
{
	return &InputHandler::cameras;
}

Model & InputHandler::getModel()
{
	return *InputHandler::model;
}

ViewPort * InputHandler::GetViewport()
{ return view; }