//Lib file by Mark Tulewicz for Math Vectors.
#include "mvector3.h"

/************************************************************
                          Constructors
************************************************************/

MPoint3d::MPoint3d()
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
}
MPoint3d::MPoint3d(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}


MVector3::MVector3()
{
	x = 0;
	y = 0;
	z = 0;
}

MVector3::MVector3(const MVector3 & rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
}

MVector3::MVector3(float x, float y, float z )
{
	this->x = x;
	this->y = y;
	this->z = z;
}

/************************************************************
                          Magnitudes
************************************************************/

float MVector3::GenerateMag() const
{
	this->myMag = (float) sqrt( (x * x ) + (y * y) + ( z * z ));
	return myMag;
}

float MVector3::GenerateMag2D() const
{
	this->myMag2D = (float) sqrt( (x * x ) + (z * z) );
	return myMag2D;
}

/************************************************************
                      Static Functions
************************************************************/

MVector3 MVector3::XProduct(const MVector3 & A, const MVector3 & B)
{
	return MVector3( ( A.y * B.z ) - (B.y * A.z ), 
					-( ( A.x * B.z ) - (B.x * A.z ) ),
					 ( A.x * B.y )  - ( B.x * A.y ) );	
}

MPoint3d MVector3::XProduct(const MPoint3d & A, const MPoint3d & B)
{
	return MPoint3d( ( A.y * B.z ) - (B.y * A.z ), 
					-( ( A.x * B.z ) - (B.x * A.z ) ),
					 ( A.x * B.y )  - ( B.x * A.y ) );	
}

float MVector3::dotProduct2D(const MVector3 & A, const MVector3 & B)
{
	return ( A.x * B.x + A.y * B.y);
}

float MVector3::dotProduct(const MVector3 & A, const MVector3 & B)
{
	return ( A.x * B.x + A.y * B.y + A.z * B.z);
}

/************************************************************
                  Operator Comparason
************************************************************/

bool MVector3::operator == ( const MVector3 & rhs ) const
{
	if ( x == rhs.x && y == rhs.y && z == rhs.z )
		return true;

	return false;
}

bool MVector3::operator != ( const MVector3 & rhs ) const
{
	if ( x == rhs.x && y == rhs.y && z == rhs.z )
		return false;

	return true;
}

bool MVector3::operator >= ( MVector3 & rhs )
{
	float rhsMag = rhs.GenerateMag();
	float lhsMag = this->GenerateMag();

	if ( lhsMag >= rhsMag)
		return true;

	return false;
}

bool MVector3::operator > ( MVector3 & rhs )
{
	float rhsMag = rhs.GenerateMag();
	float lhsMag = this->GenerateMag();

	if ( lhsMag > rhsMag)
		return true;

	return false;
}

bool MVector3::operator <= ( MVector3 & rhs )
{
	float rhsMag = rhs.GenerateMag();
	float lhsMag = this->GenerateMag();

	if ( lhsMag <= rhsMag)
		return true;

	return false;
}

bool MVector3::operator < ( MVector3 & rhs ) 
{
	float rhsMag = rhs.GenerateMag();
	float lhsMag = this->GenerateMag();

	if ( lhsMag < rhsMag)
		return true;

	return false;
}

/************************************************************
                      Subscript Accessor
************************************************************/

float & MVector3::operator [] ( int place )
{
	switch (place)
	{
		case 0:
			return x;
		case 1:
			return y;
		case 2: 
			return z;
		default:
			return *reinterpret_cast<float *>(this);
	}
}

/************************************************************
                      Multiplication
************************************************************/


MVector3 MVector3::operator * (float rhs) const
{
	return MVector3(x * rhs, y * rhs, z * rhs);
}

MVector3 MVector3::operator * (const MVector3 & rhs) const
{
	return MVector3(x * rhs.x, y * rhs.y, z * rhs.z);
}
const MVector3&  MVector3::operator *= (float rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
	return * this;
}
const MVector3& MVector3::operator *= (const MVector3 & rhs)
{
	x *= rhs.x;
	y *= rhs.y;
	z *= rhs.z;
	return * this;
}

/************************************************************
                          Addition
************************************************************/

MVector3 MVector3::operator + (float rhs) const
{
	return MVector3(x + rhs, y + rhs, z + rhs);
}

MVector3 MVector3::operator + (const MVector3 & rhs) const
{
	return MVector3(x + rhs.x, y + rhs.y, z + rhs.z);
}
const MVector3&  MVector3::operator += (float rhs)
{
	x += rhs;
	y += rhs;
	z += rhs;
	return * this;
}

const MVector3& MVector3::operator += (const MVector3 & rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return * this;
}

/************************************************************
                          Substraction
************************************************************/

MVector3 MVector3::operator - (float rhs) const
{
	return MVector3(x - rhs, y - rhs, z - rhs);
}

MVector3 MVector3::operator - (const MVector3 & rhs) const
{
	return MVector3(x - rhs.x, y - rhs.y, z - rhs.z);
}
const MVector3&  MVector3::operator -= (float rhs)
{
	x -= rhs;
	y -= rhs;
	z -= rhs;
	return * this;
}

const MVector3& MVector3::operator -= (const MVector3 & rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return * this;
}

/************************************************************
                          Division
************************************************************/

MVector3 MVector3::operator / (float rhs) const
{
	if ( rhs == 0.0f )
		return MVector3 (x, y, z);
	return MVector3(x / rhs, y / rhs, z / rhs);
}

MVector3 MVector3::operator / (const MVector3 & rhs) const
{
	float rhsx = rhs.x;
	float rhsy = rhs.y;
	float rhsz = rhs.z;
	
	if ( rhs.x == 0.0f )
		rhsx = 1.0f;
	if ( rhs.y == 0.0f )
		rhsy = 1.0f;
	if ( rhs.z == 0.0f )
		rhsz = 1.0f;
	return MVector3(x / rhsx, y / rhsy, z / rhsz);
}
const MVector3&  MVector3::operator /= (float rhs)
{
	if ( rhs == 0.0f )
		return * this;

	x /= rhs;
	y /= rhs;
	z /= rhs;
	return * this;
}

const MVector3 & MVector3::operator /= (const MVector3 & rhs)
{
	float rhsx = rhs.x;
	float rhsy = rhs.y;
	float rhsz = rhs.z;
	
	if ( rhs.x == 0.0f )
		rhsx = 1.0f;
	if ( rhs.y == 0.0f )
		rhsy = 1.0f;
	if ( rhs.z == 0.0f )
		rhsz = 1.0f;
	
	x /= rhsx;
	y /= rhsy;
	z /= rhsz;
	return * this;
}

/************************************************************
						  Rotations
************************************************************/

MVector3 MVector3::rotateY(const MVector3 & v, double angle)
{
	MVector3 value(0.0f, 0.0f, 0.0f);
	value.x = static_cast<float>(cos(angle) * v.x - sin(angle) * v.z);
	value.y = v.y;
	value.z = static_cast<float>(cos(angle) * v.z + sin(angle) * v.x);
	return value;
}

MVector3 MVector3::rotateZ(const MVector3 & v, double angle)
{
	MVector3 value(0.0f, 0.0f, 0.0f);
	value.x = static_cast<float>(cos(angle) * v.x - sin(angle) * v.y);
	value.y = static_cast<float>(cos(angle) * v.y + sin(angle) * v.x);
	value.z = v.z;
	return value;
}

MVector3 MVector3::rotateX(const MVector3 & v, double angle)
{
	MVector3 value(0.0f, 0.0f, 0.0f);
	value.x = v.x;
	value.y = static_cast<float>(cos(angle) * v.y - sin(angle) * v.z);
	value.z = static_cast<float>(cos(angle) * v.z + sin(angle) * v.y);
	return value;
}

MVector3 MVector3::rotate(const MVector3 & v, const MVector3 & axis, double angle)
{
	/*
	MVector3 value(0.0f, 0.0f, 0.0f);
	float c = static_cast<float>(cos(angle));
	float s = static_cast<float>(sin(angle));
	float t = 1 - c;
	*/
	angle;
	return MVector3(v.x, axis.y, 0.0f);
}

MVector3 MVector3::rotate(const MVector3 & v, double theta, double phi)
{	
	MVector3 value(0.0f, 0.0f, 0.0f);
	
	double sinphi = sin(phi);
	double cosphi = cos(phi);
	double sintheta = sin(theta);
	double costheta = cos(theta);

	double mag = v.GenerateMag();

	value.x = static_cast<float>(mag * sinphi * sintheta);
	value.z = static_cast<float>(mag * sinphi * costheta);
	value.y = static_cast<float>(mag * cosphi);
	
	return value;
}

const float * MVector3::asArray() const
{
	return &x;
}

/**********************************************************************
					UTILITIES
			
**********************************************************************/

float MVector3::AngleBetweenVectors(const MVector3 & A, const MVector3 & B)
{
	return acos(((MVector3::dotProduct(A,B)) / (A.GenerateMag() * B.GenerateMag())));
}



