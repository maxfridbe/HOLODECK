#ifndef SPACEVECTOR_H
#define SPACEVECTOR_H

#include <math.h>

class SpaceVector
{
   
    private:
        enum unitVectors { i, j , k };
        double magnitudes[3];
    
    public:
        
        //constructors
        SpaceVector();
		SpaceVector(const SpaceVector & source);
		const SpaceVector & operator = (const SpaceVector & rhs);
        SpaceVector( double iIn, double jIn, double kIn);
        SpaceVector( double *input );
        
        
        //operations (overload operators later)
        SpaceVector CrossProduct( const SpaceVector* source ); 
        double DotProduct( const SpaceVector* );
        SpaceVector Add( const SpaceVector* );
        SpaceVector Scale( double );
        SpaceVector Complement();
   
        
        // i/o stuff
        void GetMagnitudes( double* );
        double GetIMag();
        double GetJMag();
        double GetKMag();
        double GetAbsMag();        
        
};

#endif
