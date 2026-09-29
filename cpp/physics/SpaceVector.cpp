#include "SpaceVector.h"
#include <memory.h>

SpaceVector::SpaceVector()
{
    magnitudes[i] = 0;
    magnitudes[j] = 0;
    magnitudes[k] = 0;
    
}

SpaceVector::SpaceVector( double iIn, double jIn, double kIn )
{
    magnitudes[i] = iIn;
    magnitudes[j] = jIn;
    magnitudes[k] = kIn;
    
}

SpaceVector::SpaceVector( double *input )
{
    magnitudes[i] = input[i];
    magnitudes[j] = input[j];
    magnitudes[k] = input[k];
    
}

void SpaceVector::GetMagnitudes(double *temp )
{
    temp[i] = magnitudes[i];
    temp[j] = magnitudes[j];
    temp[k] = magnitudes[k];    
    
}

SpaceVector::SpaceVector(const SpaceVector & source)
{
	memcpy(magnitudes, source.magnitudes, sizeof(magnitudes));
}

const SpaceVector & SpaceVector::operator = (const SpaceVector & rhs)
{
	if (this == &rhs)
	{
		return *this;
	}
	memcpy(magnitudes, rhs.magnitudes, sizeof(magnitudes));
	return *this;
}

double SpaceVector::GetIMag()
{
    return magnitudes[i];
    
}

double SpaceVector::GetJMag()
{
    return magnitudes[j];
    
}
    
double SpaceVector::GetKMag()
{
    return magnitudes[k];
    
}  

double SpaceVector::GetAbsMag()
{
    return sqrt( (magnitudes[i] * magnitudes[i]) + (magnitudes[j] * magnitudes[j]) + (magnitudes[k] * magnitudes[k]) );
    
} 
    
SpaceVector SpaceVector::CrossProduct( const SpaceVector* source )
{
    double crs_i = magnitudes[j] * source->magnitudes[k] - magnitudes[k] * source->magnitudes[j];
    double crs_j = -( magnitudes[i] * source->magnitudes[k] - magnitudes[k] * source->magnitudes[i] );
    double crs_k = magnitudes[i] * source->magnitudes[j] - magnitudes[j] * source->magnitudes[i];
    
    SpaceVector ret( crs_i, crs_j, crs_k );
    
    return ret;
    
}  

double SpaceVector::DotProduct( const SpaceVector* source )
{
    
    return ( magnitudes[i] * source->magnitudes[i] ) + ( magnitudes[j] * source->magnitudes[j] ) + ( magnitudes[k] * source->magnitudes[k] );
    
}

SpaceVector SpaceVector::Add( const SpaceVector* source )
{
    SpaceVector ret( ( magnitudes[i] + source->magnitudes[i] ), ( magnitudes[j] + source->magnitudes[j] ), ( magnitudes[k] + source->magnitudes[k] ) );
    
    return ret;
    
}



SpaceVector SpaceVector::Scale( double sFactor )
{
    SpaceVector ret( (magnitudes[i] * sFactor), (magnitudes[j] * sFactor), (magnitudes[k] * sFactor) );
    
    return ret;
    
}



SpaceVector SpaceVector::Complement()
{
    SpaceVector ret( -(magnitudes[i]), -(magnitudes[j]), -(magnitudes[k]) );
    
    return ret;
    
}
