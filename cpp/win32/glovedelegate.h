/*
	Wiktor Kopec
	Last Modified 04/26/04
*/

#ifndef GLOVEDELEGATE_H
#define GLOVEDELEGATE_H

#include "delegate.h"
/*#include <P5dll.h>*/

/*#pragma comment(lib, "P5DLL.lib")*/

//<summary>This delegate is used to handle events from the P5 glove</summary>
//<remarks>This class is not fully implemented yet, and cannot be correctly registered with
//the controller</remarks>
template<class T>
class GloveDelegate : public Delegate<T>
{
	public:		
		
		//<summary>Structure representing the current glove state.  This is updated through DMA or 
		//interrupt system</summary>
		struct GloveState
		{
			//<summary>The x position of the glove</summary>
			float x;
			//<summary>The y position of the glove</summary>
			float y;
			//<summary>The z position of the glove</summary>
			float z;
			//<summary>The yaw of the glove</summary>
			float yaw;
			//<summary>The pitch of the glove</summary>
			float pitch;
			//<summary>The roll of the glove</summary>
			float roll;
			//<summary>The magnitude of the five bendable fingers of the glove</summary>
			unsigned char bendingStates[5];
			
			//<summary>The states of the buttons of the glove</summary>
			unsigned char buttonStates[4];
			/*float m_fRotMat[3][3];  //matrix for inverse kinematics?*/
		};

		typedef void (T::*VoidMethodS)(const GloveState &);

		//<summary>Default constructor</summary>
		GloveDelegate() : Delegate<T>(Delegate<T>::Invalid, NULL, NULL, 0x0) {}

		//<summary>Creates a pointer to the method that handles the glove input</summary>
		//<remarks>This function is not yet supported by the Controller</remarks>
		//<param name='classInstance'>The instance of the class being pointed to</param>
		//<param name='classMethod'>A pointer to the method to invoke.  This method must
		//take a const GloveState & as its parameter</param>
		GloveDelegate(T * classInstance, VoidMethodS classMethod) : Delegate<T>(Delegate<T>::Glove, classInstance, reinterpret_cast<VoidMethodV>(classMethod), 0x0)
		{
			referenceCount++;

			if (!glove)
			{
				/*glove = new CP5DLL();
				glove -> P5_Init();
				glove -> P5_GetMouseState(1);
				*/
			}
		}
		
		//<summary>Copy constructor</summary>
		//<param name='source'>The source DirectDelegate</param>
		GloveDelegate<T>(const GloveDelegate<T> & source) : Delegate<T>(source.type, source.instance, source.method, source.filter)
		{
			referenceCount++;
		}
		
		//<summary>Class destructor</summary>
		~GloveDelegate<T>()
		{
			referenceCount--;

			if (referenceCount == 0)
			{				
				/*glove -> P5_Close();
				delete glove;
				glove = NULL;
				*/				
			}
		}

		//<summary>Invoke the method pointed to by this delegate</summary>
		//<remarks>This function is not yet supported</remarks>
		void Invoke()
		{						
			/*
			if (glove -> m_P5Devices)
			{
				GloveState gloveState;			
				gloveState.x = glove -> m_P5Devices -> m_fx;
				gloveState.y = glove -> m_P5Devices -> m_fy;
				gloveState.z = glove -> m_P5Devices -> m_fz;

				gloveState.yaw = glove -> m_P5Devices -> m_fyaw;
				gloveState.pitch = glove -> m_P5Devices -> m_fpitch;
				gloveState.roll = glove -> m_P5Devices -> m_froll;
				
				memcpy(gloveState.bendingStates, glove -> m_P5Devices -> m_byBendSensor_Data, sizeof(gloveState.bendingStates));
				memcpy(gloveState.buttonStates, glove -> m_P5Devices -> m_byButtons, sizeof(gloveState.buttonStates));
	           
				(instance ->* reinterpret_cast<VoidMethodS>(method))(gloveState);
			}
			*/
		}

	private:
		static int referenceCount;
		static CP5DLL * glove;
};

template<class T>
CP5DLL * GloveDelegate<T>::glove = NULL;

template<class T>
int GloveDelegate<T>::referenceCount = 0;

#endif
