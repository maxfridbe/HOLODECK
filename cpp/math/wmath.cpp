/*
	Wiktor Kopec
	Last Modified 04/27/04
*/

#include "wmath.h"


MVector3 WMath::ComputeMidpoint(const MVector3 & low, const MVector3 & high)
{
	return low + ((high - low) / 2.0f);
}

void WMath::ComputeMidpoint(const float * low, const float * high, float * out)
{
	out[0] = low[0] + ((high[0] - low[0]) / 2.0f);
	out[1] = low[0] + ((high[1] - low[1]) / 2.0f);
	out[2] = low[0] + ((high[2] - low[2]) / 2.0f);
}

void WMath::RotateXYZ(const float * point, float angleX, float angleY, float angleZ, float * out)
{	    	
	float c[3] = {0.0f, 0.0f, 0.0f};
	float s[3] = {0.0f, 0.0f, 0.0f};

	c[0] = cos(angleX);
	c[1] = cos(angleY);
	c[2] = cos(angleZ);

	s[0] = sin(angleX);
	s[1] = sin(angleY);
	s[2] = sin(angleZ);
	
	RotatePrecomputedXYZ(point, c, s, out);
}

void WMath::RotateXYZ(float x, float y, float z, float angleX, float angleY, float angleZ, float & xout, float & yout, float & zout)
{
	float c[3] = {0.0f, 0.0f, 0.0f};
	float s[3] = {0.0f, 0.0f, 0.0f};

	c[0] = cos(angleX);
	c[1] = cos(angleY);
	c[2] = cos(angleZ);

	s[0] = sin(angleX);
	s[1] = sin(angleY);
	s[2] = sin(angleZ);
	
	RotatePrecomputedXYZ(x, y, z, c, s, xout, yout, zout);
}

void WMath::RotatePrecomputedXYZ(float x, float y, float z, const float * c, const float * s, float & xout, float & yout, float & zout)
{
	float temp[3] = {0.0f, 0.0f, 0.0f};
	
	temp[0] = x;
	temp[1] = y;
	temp[2] = z;

	xout = temp[0];
	yout = temp[1] * c[0] - temp[2] * s[0];
	zout = temp[2] * c[0] + temp[1] * s[0];
	
	temp[0] = xout;
	temp[1] = yout;
	temp[2] = zout;

	xout = temp[0] * c[1] - temp[2] * s[1];
	yout = temp[1];
	zout = temp[2] * c[1] + temp[0] * s[1];

	temp[0] = xout;
	temp[1] = yout;
	temp[2] = zout;

	xout = temp[0] * c[2] - temp[1] * s[2];
	yout = temp[1] * c[2] + temp[0] * s[2];
	zout = temp[2];	
}

void WMath::RotatePrecomputedXYZ(const float * point, const float * c, const float * s, float * out)
{
	float temp[3] = {0.0f, 0.0f, 0.0f};
	
	memcpy(temp, point, sizeof(temp));

	out[0] = temp[0];
	out[1] = temp[1] * c[0] - temp[2] * s[0];
	out[2] = temp[2] * c[0] + temp[1] * s[0];

	memcpy(temp, out, sizeof(temp));

	out[0] = temp[0] * c[1] - temp[2] * s[1];
	out[1] = temp[1];
	out[2] = temp[2] * c[1] + temp[0] * s[1];

	memcpy(temp, out, sizeof(temp));

	out[0] = temp[0] * c[2] - temp[1] * s[2];
	out[1] = temp[1] * c[2] + temp[0] * s[2];
	out[2] = temp[2];	
}

void WMath::RotateZYX(const float * point, float angleX, float angleY, float angleZ, float * out)
{	    	
	float c[3] = {0.0f, 0.0f, 0.0f};
	float s[3] = {0.0f, 0.0f, 0.0f};

	c[0] = cos(angleX);
	c[1] = cos(angleY);
	c[2] = cos(angleZ);

	s[0] = sin(angleX);
	s[1] = sin(angleY);
	s[2] = sin(angleZ);
	
	RotatePrecomputedZYX(point, c, s, out);
}

void WMath::RotateZYX(float x, float y, float z, float angleX, float angleY, float angleZ, float & xout, float & yout, float & zout)
{
	float c[3] = {0.0f, 0.0f, 0.0f};
	float s[3] = {0.0f, 0.0f, 0.0f};

	c[0] = cos(angleX);
	c[1] = cos(angleY);
	c[2] = cos(angleZ);

	s[0] = sin(angleX);
	s[1] = sin(angleY);
	s[2] = sin(angleZ);
	
	RotatePrecomputedZYX(x, y, z, c, s, xout, yout, zout);
}

void WMath::RotatePrecomputedZYX(float x, float y, float z, const float * c, const float * s, float & xout, float & yout, float & zout)
{
	float temp[3] = {0.0f, 0.0f, 0.0f};
	
	temp[0] = x;
	temp[1] = y;
	temp[2] = z;

	xout = temp[0] * c[2] - temp[1] * s[2];
	yout = temp[1] * c[2] + temp[0] * s[2];
	zout = temp[2];	
	
	temp[0] = xout;
	temp[1] = yout;
	temp[2] = zout;

	xout = temp[0] * c[1] - temp[2] * s[1];
	yout = temp[1];
	zout = temp[2] * c[1] + temp[0] * s[1];

	temp[0] = xout;
	temp[1] = yout;
	temp[2] = zout;

	xout = temp[0];
	yout = temp[1] * c[0] - temp[2] * s[0];
	zout = temp[2] * c[0] + temp[1] * s[0];
}

void WMath::RotatePrecomputedZYX(const float * point, const float * c, const float * s, float * out)
{
	float temp[3] = {0.0f, 0.0f, 0.0f};
	
	memcpy(temp, point, sizeof(temp));

	out[0] = temp[0] * c[2] - temp[1] * s[2];
	out[1] = temp[1] * c[2] + temp[0] * s[2];
	out[2] = temp[2];	

	memcpy(temp, out, sizeof(temp));

	out[0] = temp[0] * c[1] - temp[2] * s[1];
	out[1] = temp[1];
	out[2] = temp[2] * c[1] + temp[0] * s[1];

	memcpy(temp, out, sizeof(temp));

	out[0] = temp[0];
	out[1] = temp[1] * c[0] - temp[2] * s[0];
	out[2] = temp[2] * c[0] + temp[1] * s[0];
}


float WMath::DegreeToRadian(float degree)
{
	return degree / static_cast<float>(DEGRAD);
}

float WMath::RadianToDegree(float radian)
{
	return radian * static_cast<float>(DEGRAD);
}

double WMath::DegreeToRadian(double degree)
{
	return degree / DEGRAD;
}

double WMath::RadianToDegree(double radian)
{
	return radian * DEGRAD;
}
