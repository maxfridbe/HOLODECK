#include "PNode.h"

PNode::PNode( double objectMass)
{
    x = 0;
    y = 0;
    z = 0;
     
    //monoparticle has only a center of mass
    
    forceIndex = -1;
    
    mass = objectMass;
    
    //bounding box does not exist for monoparticles
    
}//PNode

double PNode::GetX()
{
        return x;
}

double PNode::GetY()
{
        return y;
}

double PNode::GetZ()
{
        return z;
}

void PNode::SetPosition( double* xyz )
{
    x = xyz[0];
    y = xyz[1];
    z = xyz[2];
    
}//set_position
 
SpaceVector PNode::GetVelocity()
{
    SpaceVector temp = velocity;
    
    return temp;
    
}//get velocity


double PNode::GetMass()
{
    return mass;
    
}//get_mass


void PNode::AddForce(const Force & appliedForce )
{
	forces.push_back( appliedForce );
}//add force       
      
void PNode::UpdateForces( double delta_t )
{         
    //reset force
    netForce = netForce.Scale(0);
    
    for ( int i = 0; i < forces.size() ; i++ )
    {
        if ( forces[i].impulse ) 
        {
           if ( forces[i].duration <= 0 )
              forces.erase( forces.begin() + i );
           else
           {                                
              forces[i].duration -= delta_t;
              netForce = netForce.Add( &forces[i].force_vect );
           }
            
        }    
        else 
           netForce = netForce.Add( &forces[i].force_vect );
         
     }    

}//update_forces

void PNode::UpdateVelocity( double delta_t )
{
    // v = v0 + at; f = ma; a = f/m; v =v0 + t*f/m
    SpaceVector temp_v = netForce.Scale( delta_t/mass   );
    velocity = velocity.Add( &temp_v ) ;
    
}//update velocity

void PNode::UpdatePosition( double delta_t ) 
{
    //x = x0 + vt
    x += velocity.GetIMag() * delta_t;
    y += velocity.GetJMag() * delta_t;
    z += velocity.GetKMag() * delta_t;
        

}//update position
    
    
void PNode::Collision( PNode &source )
{
  //this method simulates perfectly ellastic collisions
  //when links between nodes are implemented, they will
  //provide the "springiness" reqiured for partially 
  //elastic (realisitic) collisions
  //
  //things to fix: change velocity model to force model
  //for consistency
  
  
  //v1' = (v1(m1-m2)/(m1+m2) 
  //v2' = (v2(2m1)/(m1+m2)
  if ( source.mass == 0 )
  {
       velocity = velocity.Complement();
       return;
  }
  
  
  SpaceVector tempV1( 0,0,0 );
  SpaceVector tempV2( 0,0,0 );
  
  tempV1 = velocity.Scale( mass - source.mass );
  velocity = tempV1.Scale( 1/(mass + source.mass));
  
  
  tempV2 = source.velocity.Scale( 2 * mass );
  source.velocity = tempV2.Scale( 1/(mass + source.mass));
     
  cout << velocity.GetKMag() << " : " << source.velocity.GetKMag() << endl;  
     
     
}//collision   

void PNode::Collision(double mass, const SpaceVector & vector )
{
	//this method simulates perfectly ellastic collisions
	//when links between nodes are implemented, they will
	//provide the "springiness" reqiured for partially 
	//elastic (realisitic) collisions
	//
	//things to fix: change velocity model to force model
	//for consistency


	//v1' = (v1(m1-m2)/(m1+m2) 
	//v2' = (v2(2m1)/(m1+m2)
	if ( mass == 0 )
	{
		velocity = velocity.Complement();
		return;
	}


	SpaceVector tempV1( 0,0,0 );
//	SpaceVector tempV2( 0,0,0 );

	tempV1 = velocity.Scale( this -> mass - mass );
	velocity = tempV1.Scale( 1/(this -> mass + mass));


//	tempV2 = vector.Scale( 2 * this -> mass );
//	source.velocity = tempV2.Scale( 1/(this -> mass + mass));
	    
//	cout << velocity.GetKMag() << " : " << source.velocity.GetKMag() << endl;            
}//collision   
    
    
void PNode::Reset()
{
     velocity.Scale(0);
     
}//reset   
