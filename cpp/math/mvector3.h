/*		Math Vector class - written by Mark Tulewicz.
		
		* This class assumes all functions to be 3d, unless 
		the function name specifies 2D in it.
		
		* It accounts for all comparative operations and has [] 
		access for individual component access. Comparasons are
		based off of the magnitude of the vector, which makes it
		a little more expensive then a simple function. Use sparingly.

		* It is also zero divide protected.  diving by now 
		becomes the same as dividing by 1.0.
*/

#ifndef MVECTOR3_H
#define MVECTOR3_H
#include <cmath>
#include <stdio.h>

using namespace std;

#define PI 3.14159265359
#define TWOPI 6.28318530718
#define HALFPI 1.57079632679

class MPoint3d
{
	public:
		MPoint3d();
		MPoint3d(double x, double y, double z);
		double x;
		double y;
		double z;
};

class MVector3
{
	public:

		float x;
		float y;
		float z;

		MVector3();
		MVector3(float x, float y, float z);
		MVector3(const MVector3 & );

		mutable float myMag;
		mutable float myMag2D;
		float GenerateMag() const;
		float GenerateMag2D() const;
		
		const float * asArray() const;

		//Compare Operators:
		bool operator == ( const MVector3& ) const;
		bool operator != ( const MVector3& ) const;
		//Cannot be const since we will generate mags for them.	
		bool operator >= ( MVector3& );
		bool operator > ( MVector3& );
		bool operator <= ( MVector3& );
		bool operator < ( MVector3& );

		//crossing 2 vectors:
		float & operator [] ( int );

		//Multiply:
		MVector3 operator * ( float scalar ) const;
		MVector3 operator * ( const MVector3 & dotVector ) const;	//calls dot product;
		const MVector3& operator *= ( float scalar ) ;
		const MVector3& operator *= ( const MVector3 & dotVector );	//calls dot product;

		//Add:
		MVector3 operator + ( float scalar ) const;
		MVector3 operator + ( const MVector3 & addativeVector ) const;
		const MVector3& operator += ( float scalar );
		const MVector3& operator += ( const MVector3 & addativeVector );
		
		//Substract:
		MVector3 operator - ( float scalar ) const;
		MVector3 operator - ( const MVector3 & subtractVector ) const;
		const MVector3& operator -= ( float scalar );
		const MVector3& operator -= ( const MVector3 & subtractiveVector );

		//Divide:
		MVector3 operator / ( float scalar ) const;
		MVector3 operator / ( const MVector3 & divVector ) const;
		const MVector3& operator /= ( float scalar );
		const MVector3& operator /= ( const MVector3 & dividiveVector ) ;

		//Statics:
		static MVector3 XProduct( const MVector3 & A, const MVector3 & B );
		static MPoint3d XProduct( const MPoint3d & A, const MPoint3d & B );
		static float dotProduct2D( const MVector3 & A, const MVector3 & B );
		static float dotProduct( const MVector3 & A, const MVector3 & B );
		
		//Utilities:
		static float AngleBetweenVectors(const MVector3 & A, const MVector3 & B);
			
		//rotations:
		static MVector3 rotateX(const MVector3 &, double angle);
		static MVector3 rotateY(const MVector3 &, double angle);
		static MVector3 rotateZ(const MVector3 &, double angle);
		static MVector3 rotate(const MVector3 &, const MVector3 &, double angle);
		static MVector3 rotate(const MVector3 &, double theta, double phi);
};

#endif
