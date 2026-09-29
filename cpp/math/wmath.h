/*
	Wiktor Kopec
	Last Modified 04/27/04
*/

#ifndef WMATH_H
#define WMATH_H

#include "mvector3.h"
#include "string.h"

#define DEGRAD 57.295779513082320876798154814105

//<summary>This is a special purpose math class designed to quickly compute
//certain mathematical formulas</summary>
//<remarks>This class cannot be instantiated.</remarks>
class WMath
{
	public:
		//<summary>Computes a midpoint between two points in three space</summary>
		//<param name='lo'>An array of 3 floats</param>
		//<param name='hi'>An array of 3 floats</param>
		//<param name='out'>An array of 3 floats, allocated by the caller</param>
		static void ComputeMidpoint(const float * lo, const float * hi, float * out);
		
		//<summary>Computes a midpoint between two points in three space</summary>		
		//<param name='low'>An MVector3 class representing a point in 3 space</param>
		//<param name='hi'>An MVector3 class representing a point in 3 space</param>
		//<returns>An MVector3 class representing the middle point</returns>
		static MVector3 ComputeMidpoint(const MVector3 & low, const MVector3 & hi);	
		
		//<summary>Converts degrees to radians</summary>
		//<param name='degree'>Degrees to convert</param>
		//<returns>The amount of radians equal to the degree parameter</returns>
		static float DegreeToRadian(float degree);

		//<summary>Converts radians to degrees</summary>
		//<param name='radian'>Radians to convert</param>
		//<returns>The amount of degrees equal to the radian parameter</returns>
		static float RadianToDegree(float radian);

		//<summary>Converts degrees to radians</summary>
		//<param name='degree'>Degrees to convert</param>
		//<returns>The amount of radians equal to the degree parameter</returns>
		static double DegreeToRadian(double degree);

		//<summary>Converts radians to degrees</summary>
		//<param name='radian'>Radians to convert</param>
		//<returns>The amount of degrees equal to the radian parameter</returns>
		static double RadianToDegree(double radian);

        //<summary>Rotates a point in three space along all three axes, starting with the X axis then the
		//Y axis and then the Z axis</summary>
		//<param name='point'>A point in three space</param>
		//<param name='angleX'>The angle by which to rotate on the X axis, in degrees</param>
		//<param name='angleY'>The angle by which to rotate on the Y axis, in degrees</param>
		//<param name='angleZ'>The angle by which to rotate on the Z axis, in degrees</param>
		//<param name='out'>The resulting point in 3 space, allocated by the caller</param>
		static void RotateXYZ(const float * point, float angleX, float angleY, float angleZ, float * out);
		
		//<summary>Rotates a point in three space along all three axes, starting with the X axis then the
		//Y axis and then the Z axis</summary>
		//<param name='x'>The x component of a point in 3 space</param>
		//<param name='y'>The y component of a point in 3 space</param>
		//<param name='z'>The z component of a point in 3 space</param>
		//<param name='angleX'>The angle by which to rotate on the X axis, in degrees</param>
		//<param name='angleY'>The angle by which to rotate on the Y axis, in degrees</param>
		//<param name='angleZ'>The angle by which to rotate on the Z axis, in degrees</param>		
		//<param name='xout'>The resulting x component</param>
		//<param name='yout'>The resulting x component</param>
		//<param name='zout'>The resulting x component</param>
		static void RotateXYZ(float x, float y, float z, float angleX, float angleY, float angleZ, float & xout, float & yout, float & zout);
        
        //<summary>Rotates a point in three space along all three axes, starting with the Z axis then the
		//Y axis and then the Z axis</summary>
		//<remarks>This function is specifically designed to undo the effects of the RotateXYZ function.</remarks>
		//<param name='point'>A point in three space</param>
		//<param name='angleX'>The angle by which to rotate on the X axis, in degrees</param>
		//<param name='angleY'>The angle by which to rotate on the Y axis, in degrees</param>
		//<param name='angleZ'>The angle by which to rotate on the Z axis, in degrees</param>
		//<param name='out'>The resulting point in 3 space, allocated by the caller</param>		
		static void RotateZYX(const float * point, float angleX, float angleY, float angleZ, float * out);

		//<summary>Rotates a point in three space along all three axes, starting with the Z axis then the
		//Y axis and then the X axis</summary>
		//<remarks>This function is specifically designed to undo the effects of the RotateXYZ function.</remarks>
		//<param name='x'>The x component of a point in 3 space</param>
		//<param name='y'>The y component of a point in 3 space</param>
		//<param name='z'>The z component of a point in 3 space</param>
		//<param name='angleX'>The angle by which to rotate on the X axis, in degrees</param>
		//<param name='angleY'>The angle by which to rotate on the Y axis, in degrees</param>
		//<param name='angleZ'>The angle by which to rotate on the Z axis, in degrees</param>		
		//<param name='xout'>The resulting x component</param>
		//<param name='yout'>The resulting x component</param>
		//<param name='zout'>The resulting x component</param>
		static void RotateZYX(float x, float y, float z, float angleX, float angleY, float angleZ, float & xout, float & yout, float & zout);
		
        //<summary>Rotates a point in three space along all three axes, starting with the X axis then the
		//Y axis and then the Z axis</summary>
		//<remarks>Use this function when calling repeated rotation operations that rotate along the
		//same angle</remarks>		
		//<param name='point'>A point in three space</param>
		//<param name='c'>An array equivalent to cos(angleX), cos(angleY), cos(angleZ)
		//<param name='s'>An array equivalent to sin(angleX), sin(angleY), sin(angleZ)
		//<param name='out'>The resulting point in 3 space, allocated by the caller</param>
		static void RotatePrecomputedXYZ(const float * point, const float * c, const float * s, float * out);

        //<summary>Rotates a point in three space along all three axes, starting with the X axis then the
		//Y axis and then the Z axis</summary>
		//<remarks>Use this function when calling repeated rotation operations that rotate along the
		//same angle</remarks>		
		//<param name='x'>The x component of a point in 3 space</param>
		//<param name='y'>The y component of a point in 3 space</param>
		//<param name='z'>The z component of a point in 3 space</param>
		//<param name='c'>An array equivalent to cos(angleX), cos(angleY), cos(angleZ)
		//<param name='s'>An array equivalent to sin(angleX), sin(angleY), sin(angleZ)
		//<param name='xout'>The resulting x component</param>
		//<param name='yout'>The resulting x component</param>
		//<param name='zout'>The resulting x component</param>		
		static void RotatePrecomputedXYZ(float x, float y, float z, const float * c, const float * s, float & xout, float & yout, float & zout);

        //<summary>Rotates a point in three space along all three axes, starting with the Z axis then the
		//Y axis and then the X axis</summary>
		//<remarks>Use this function when calling repeated rotation operations that rotate along the
		//same angle.  This function is specifically designed to undo the effects of the RotatePrecomputedXYZ function.</remarks>
		//<param name='point'>A point in three space</param>
		//<param name='c'>An array equivalent to cos(angleX), cos(angleY), cos(angleZ)
		//<param name='s'>An array equivalent to sin(angleX), sin(angleY), sin(angleZ)
		//<param name='out'>The resulting point in 3 space, allocated by the caller</param>
		static void RotatePrecomputedZYX(const float * point, const float * c, const float * s, float * out);

        //<summary>Rotates a point in three space along all three axes, starting with the Z axis then the
		//Y axis and then the X axis</summary>
		//<remarks>Use this function when calling repeated rotation operations that rotate along the
		//same angle.  This function is specifically designed to undo the effects of the RotatePrecomputedXYZ function.</remarks>
		//<param name='x'>The x component of a point in 3 space</param>
		//<param name='y'>The y component of a point in 3 space</param>
		//<param name='z'>The z component of a point in 3 space</param>
		//<param name='c'>An array equivalent to cos(angleX), cos(angleY), cos(angleZ)
		//<param name='s'>An array equivalent to sin(angleX), sin(angleY), sin(angleZ)
		//<param name='xout'>The resulting x component</param>
		//<param name='yout'>The resulting x component</param>
		//<param name='zout'>The resulting x component</param>		
		static void RotatePrecomputedZYX(float x, float y, float z, const float * c, const float * s, float & xout, float & yout, float & zout);
	private:
		WMath();
};

#endif
