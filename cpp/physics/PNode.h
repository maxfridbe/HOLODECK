#ifndef PNODE_H
#define PNODE_H

#include "SpaceVector.h"
#include <vector>

//for test purposes
#include <iostream>
#include <list>
#include <string>

using namespace std;

//============================= p_link, p_node and force ================================
// supporting structs for p_node

struct Force
{
    string name;
	const string & getName() const
	{
		return name;
	}
	void setName(const string & name)
	{
		this -> name = name;
	}

	bool impulse; //true if force has limited duration
    double duration; //how long the force will last in seconds
    SpaceVector force_vect;
    int scalar_attrib; //scalar multiple for force fields ( f = mg, etc.)
};   

struct PLink
{ 
 //id for p_nodes at either end of the link   
 int aNode;
 int bNode;
 
 //vector representation of link from node a to node b
 SpaceVector linkVector;   
 
 //insert other physical properties as necessary (conductivity, "springiness", etc.)
};

//======= p_node =================================================================================

class PNode
{
	private:        
		//physical information
		double mass;
		SpaceVector velocity;

		vector<Force> forces;
		int forceIndex;

		SpaceVector netForce;//sum of all forces

		//list of links to other nodes
		PLink * links;  

		double x;
		double y;
		double z;     
        
    public:
        
        //constructors
		PNode( double );
        
        
        //accessors
        double GetX();
        double GetY();
        double GetZ();
        
        SpaceVector GetVelocity();
        double GetMass();  
        
        //mutators
        void SetPosition( double* ); //used to set initial position
              
        //physical manipulators
        void AddForce(const Force & force);
        void UpdateForces( double delta_t ); //delta_t is the delta_t in seconds since last update
        void UpdateVelocity( double delta_t );
        void UpdatePosition( double delta_t );
        void Collision( PNode& );
		void Collision(double mass, const SpaceVector & vector );
        void Reset();
        
        //object editing functions        
};
    
#endif
    