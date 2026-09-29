#ifndef MATRIX_H
#define MATRIX_H

#include <memory.h>
#include <stdarg.h>
#include <cstdlib>

#include "mvector3.h"

class Matrix
{
	public:
		Matrix(float * values);
		Matrix();
		Matrix(float a00, float a01, float a02, float a03, float a10, float a11, float a12, float a13,
			   float a20, float a21, float a22, float a23, float a30, float a31, float a32, float a33);
		Matrix(const Matrix & source);
		Matrix & operator * (const Matrix & rhs);
		float & operator [] (int index);
		const float & Matrix::operator [] (int index) const;
		const Matrix & operator = (const Matrix & rhs);
		
		void HardMult(const Matrix & rhs, Matrix & output);
		void HardMult(const Matrix & rhs);
		void HardRotate(float angle, float * rotate);
		void Transpose(Matrix * output = NULL);
		void VectorMult(const MPoint3d & point, MPoint3d & output) const;
		void VectorMult(const MVector3 & vector, MVector3 & output) const;
		float get(int row, int col) const;
		void set(int row, int col, float value);
		void LoadIdentity();
		
		float * Values();
	private:
		float m_values[16];

};

inline Matrix::Matrix(float a00, float a01, float a02, float a03, float a10, float a11, float a12, float a13,
	   float a20, float a21, float a22, float a23, float a30, float a31, float a32, float a33)
{
	m_values[0] = a00;
	m_values[1] = a01;
	m_values[2] = a02;
	m_values[3] = a03;
	m_values[4] = a10;
	m_values[5] = a11;
	m_values[6] = a12;
	m_values[7] = a13;
	m_values[8] = a20;
	m_values[9] = a21;
	m_values[10] = a22;
	m_values[11] = a23;
	m_values[12] = a30;
	m_values[13] = a31;
	m_values[14] = a32;
	m_values[15] = a33;
}

inline Matrix::Matrix(float * values)
{
	memcpy(m_values, values, sizeof(m_values));
}

inline Matrix::Matrix()
{
	memset(m_values, 0, sizeof(m_values));
	m_values[0] = 1.0f;
	m_values[5] = 1.0f;
	m_values[10] = 1.0f;
	m_values[15] = 1.0f;

	/*
	va_list list = NULL;
	Matrix * ptr = this;
	va_start(list, ptr);
	
	int i(0);
	while (list != NULL && i < 16)
	{
		m_values[i] = va_arg(list, float);		
	}
	*/
}

inline Matrix::Matrix(const Matrix & source)
{
	memcpy(m_values, source.m_values, sizeof(m_values));
}

inline const Matrix & Matrix::operator = (const Matrix & rhs)
{
	if (this == &rhs)
	{
		return *this;
	}
	memcpy(m_values, rhs.m_values, sizeof(m_values));
	return *this;
}

inline float & Matrix::operator [] (int index) 
{
	return m_values[index];
}

inline const float & Matrix::operator [] (int index) const 
{
	return m_values[index];
}


inline float * Matrix::Values()
{
	return m_values;
}

#endif
