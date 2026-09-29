/*
	Wiktor Kopec
	Last Modified 04/26/04
*/

#ifndef MODEL_H
#define MODEL_H

#include "wstring.h"
#include "renderableobject.h"
#include "wlist.h"
#include "whashtable.h"

#define DEFAULTOBJECTCOUNT 64

//<summary>The model class provides functionality for opening, loading, and saving all of the
//imported objects.</summary>
//<remarks>The model keeps track of all the Object instances in the world.  It also does the parsing
//and interpreting of the 3dbin file format.  Currently all scene loading is done relative to the scene
//file path.  Models that fail to load are ignored.</remarks>
class Model
{
	public:
		
		//<summary>Default constructor</summary>
		Model();
		
		//<summary>Close the current scene</summary>
		//<remarks>This function effectively wipes all models from memory</remarks>
		void CloseScene();
		
		//<summary>Opens a 3dbin object, and draws it onto the screen</summary>
		//<remarks>Specifying an empty string for the path indicates a relative path.</remarks>
		//<param name='object'>The object name to load.  This should have no extension</param>
		//<param name='ext'>The extension of the object.  Defaults to 3dbin, the only format
		//currently supported</param>
		//<param name='path'>The path of the object.  Defaults to "", which indicates relative path</param>
		Object * Open(const String & object, const String & ext = "3dbin", const String & path = "");
		
		//<summary>Returns the total amount of objects loaded</summary>
		int getCount() const;		

		bool CollisionTest();

		RenderableObject * Clone(RenderableObject & source);
		
		//<summary>Returns an object reference that matches the id paramter</summary>
		//<remarks>The id represents the OpenGL glLoadName id.  It actually maps to an index
		//in the Vector of objects in the model.  Because these are assigned sequentially, but can
		//be unloaded, it is important that only valid id's are passed in.</remarks>
		//<param name='id'>The id assigned to an object using the glLoadName command</param>
		//<returns>A reference to the object which has this id</returns>
		Object & operator [](int id) const;
		
		//<summary>Class destructor</summary>
		~Model();

		//<summary>Closes a model</summary>
		//<param name='id'>The id of the model obtained from glLoadName</param>
		void Close(int id);

		//<summary>Saves a scene to an ascii scene file, which can later be loaded</summary>
		//<remarks>Scene files contain only relative model information which means the scene files
		//and the model files should reside in the same location.  The scene file name and extension
		//are irrelevent.</remarks>
		//<param name='sceneFile>The scene file to save to</param>
		void SaveScene(const String & sceneFile);

		//<summary>Loads a scene from an ascii scene file</summary>
		//<remarks>Scene files contain only relative model information which means the scene files
		//and the model files should reside in the same location.  The scene file name and extension
		//are irrelevent.</remarks>
		//<param name='sceneFile>The scene file to load</param>
		void LoadScene(const String & sceneFile);

		void UpdateDisjointSetForObject(int id);
		//<summary>This function draws all the RenderableObjects the model may currently have</summary>
		void Draw();

		void ApplyForces();
	private:	
		/*<summary>To update the disjoint sets, When an object dies / is murdered, the first child
		must be promoted to parent, and then all remaining syblings become children of the new 
		promoted child </summary>*/
		int promoteChildwithParentID(int dieingParentIdOfChild);
		void updateAllChildrenWithID(int dieingParentIdOfChild, int newParentID);
		
		void ParseObject(FILE * in, const String &);

		int nextId;
		Vector<Object *> objects;
		
		FILE * in;		
};

#endif
