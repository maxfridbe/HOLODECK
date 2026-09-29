#ifndef MODELMANIP_H
#define MODELMANIP_H

#include "movableobject.h"

#include "wlist.h"
#include "userinterface.h"
#include "mvector3.h"
#include "cameramanager.h"


//	<summary>
//	This is the class that allows for interaction between the user
//	and the objects in the world. This is a static class because only
//	one instance of a manipulator can exist in a given time.
//	</summary>

//	<remarks>
//	The class needs to be rewriten to allow for other "UP" axis' right
//	now it only works with the positive y, but all it would take is an
//	if statement to allow for the rest.
//	</remarks>


class ModelManipulator
{
	public:
		//<summary>This is every state that the world can put the manipulator into.</summary>
		enum ManipulatorTypes
		{
			ROTATE,
			TRANSLATE,
			SCALE,
			INVISIBLE,
		};

		//<summary>This is the main draw function that turns on the manipulator and begins the mouse checks.</summary>
		static void DrawManipulator(bool enable);

		//<summary>Most important component: allows for the translation of user x/y movement to xyz movement based on the up axis.</summary>
		static void CheckMouse();

		//<summary>Tells the world if the manipulator is being used.</summary>
		static bool ManipActive();

		//<summary>Throws an object onto the classes list of objects.</summary>
		//<param name='object'>The address of the object to be manipulated.</param>
		//<param name='init'>Should the class wipe the list it currently has?</param>
		static void AddObject(Object & object, bool init);

		//<summary>In order for CheckMouse to work the players postion needs to be set.</summary>
		//<param name='pos'>The address of the position of the player.</param>
		static void SetPlayer(MVector3 & pos);

		//<summary>Sets the currentType variable to the ManipulatorType.</summary>
		//<param name='type'>What the user want to do with the object.</param>
		static void SetState(ManipulatorTypes type);

		//<summary>Returns the manipulators current State.</summary>
		static ManipulatorTypes GetState();

		//<summary>Wipes the list of objects.</summary>
		static void ClearObjects();
		static double getDistance();
		static void setDistance(Camera * camera);


	private:
		static double distance;

		//<summary>Nada, it's static.</summary>
		ModelManipulator();
	
		//<summary>That which calls the objects translation functions</summary>
		//<param name='offsetX'>Takes the offset of which to move the object based on X.</param>
		//<param name='offsetY'>Takes the offset of which to move the object based on Y.</param>
		//<param name='offsetZ'>Takes the offset of which to move the object based on Z.</param>
		//<remarks>It will do internal calculation to determine actual changes</remarks>
		static void UpdateObject(float offsetX, float offsetY, float offsetZ);

		//<summary>This is the states of each individual type of manipulators.</summary>
		enum ManipulatorStates
		{
			IDLE,
			OFF,
			ACTIVATING,
			DEACTIVATING,
			DRAGGINGX,
			DRAGGINGY,
			DRAGGINGZ,
		};

		//<summary>Array of glnumberd manipulator X objects in TRANSLATE, SCALE, ROTATE order.</summary>
		static int xObject[3];
		//<summary>Array of glnumberd manipulator Y objects in TRANSLATE, SCALE, ROTATE order.</summary>
		static int yObject[3];
		//<summary>Array of glnumberd manipulator Z objects in TRANSLATE, SCALE, ROTATE order.</summary>
		static int zObject[3];

		//<summary>The current manipulator selected.</summary>
		static ManipulatorTypes currentType;
		//<summary>The current state of the manipulator selected.</summary>
		static ManipulatorStates currentState;
		//<summary>The last stored manipulator type.</summary>
		static ManipulatorTypes oldForm;

		static void DoTranslate();
		static void DoRotate();

		//<summary>The list of objects affected by one manipulator.</summary>
		//<remarks>Currently only one object can be selected at a time, but at least there's a list to grow with.</remarks>*/
		static List<Object *> currentObject;

		//<summary>The X position of the Manipulator.</summary>
		static float x;
		//<summary>The Y position of the Manipulator.</summary>
		static float y;
		//<summary>The Z position of the Manipulator.</summary>
		static float z;

		//<summary>Not really used for anything other than mouseState.</summary>
		static bool show;

		//<summary>Can be, but not, used in generic multiplication of scaler sizes</summary>
		//<remarks>implement if needed</remarks>
		static float scalar;

		//<summary>The Pointer to the players position to be used internaly</summary>
		static MVector3 * PlayerPosition;

		//<summary>Distance from x,y,z to the x object manipulators x,y,z.</summary>
		static float offsetx; 
		//<summary>Distance from x,y,z to the Y object manipulators x,y,z.</summary>
		static float offsety;
		//<summary>Distance from x,y,z to the Z object manipulators x,y,z.</summary>
		static float offsetz;

		//<summary>Distance from a manipulators x,y,z to the end of the manipulator.</summary>
		static float base; // the length of the manipulator
		//<summary>The radius of the manipulators.</summary>
		static float rad;  // the radius of the manipulator

		/********************************************************
					Draw Manipulator Functions
		********************************************************/

		//<summary>Draws the pyramid/translate tools X manipulator.</summary>
		//<param name='loc'>Takes the position to which to point in 3Space.</param>
		static void DrawPyramidX(const MVector3 & loc);
		//<summary>Draws the pyramid/translate tools Y manipulator.</summary>
		//<param name='loc'>Takes the position to which to point in 3Space.</param>
		static void DrawPyramidY(const MVector3 & loc);
		//<summary>Draws the pyramid/translate tools Z manipulator.</summary>
		//<param name='loc'>Takes the position to which to point in 3Space.</param>
		static void DrawPyramidZ(const MVector3 & loc);

		//<summary>Draws the cube/scale tools X manipulator.</summary>
		//<param name='loc'>Takes the position to which to point in 3Space.</param>
		static void DrawCubeX(const MVector3 & loc);
		//<summary>Draws the cube/scale tools Y manipulator.</summary>
		//<param name='loc'>Takes the position to which to point in 3Space.</param>
		static void DrawCubeY(const MVector3 & loc);
		//<summary>Draws the cube/scale tools Z manipulator.</summary>
		//<param name='loc'>Takes the position to which to point in 3Space.</param>
		static void DrawCubeZ(const MVector3 & loc);

		//<summary>Draws the circle/rotate tools X manipulator.</summary>
		//<param name='loc'>Take the radius of the circle as a float.</param>
		static void DrawCircleX(float Rad);
		//<summary>Draws the circle/rotate tools Y manipulator.</summary>
		//<param name='loc'>Take the radius of the circle as a float.</param>
		static void DrawCircleY(float Rad);
		//<summary>Draws the circle/rotate tools Z manipulator.</summary>
		//<param name='loc'>Take the radius of the circle as a float.</param>
		static void DrawCircleZ(float Rad);
};

#endif
