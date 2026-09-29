#include "matrix.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>

void Matrix::VectorMult(const MPoint3d & point, MPoint3d & output) const
{
	MPoint3d result = MPoint3d(0,0,0);

	result.x = (*this)[0] * point.x + (*this)[1] * point.y + (*this)[2] * point.z + (*this)[3] * 1;
	result.y = (*this)[4] * point.x + (*this)[5] * point.y + (*this)[6] * point.z + (*this)[7] * 1;
	result.z = (*this)[8] * point.x + (*this)[9] * point.y + (*this)[10] * point.z + (*this)[11] * 1;

	output = result;
}

void Matrix::VectorMult(const MVector3 & point, MVector3 & output) const
{
	MVector3 result = MVector3(0,0,0);
	result.x = (*this)[0] * point.x + (*this)[1] * point.y + (*this)[2] * point.z + (*this)[3] * 1;
	result.y = (*this)[4] * point.x + (*this)[5] * point.y + (*this)[6] * point.z + (*this)[7] * 1;
	result.z = (*this)[8] * point.x + (*this)[9] * point.y + (*this)[10] * point.z + (*this)[11] * 1;
	output = result;
}

void Matrix::LoadIdentity()
{
	memset(m_values, 0, sizeof(m_values));
	m_values[0] = 1.0f;
	m_values[5] = 1.0f;
	m_values[10] = 1.0f;
	m_values[15] = 1.0f;
}

void Matrix::HardMult(const Matrix & rhs, Matrix & output)
{
	glPushAttrib(GL_TRANSFORM_BIT);
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		{
			glLoadIdentity();
			glMultMatrixf(m_values);
			glMultMatrixf(rhs.m_values);
			glGetFloatv(GL_MODELVIEW_MATRIX, output.m_values);
		}
		glPopMatrix();
	}
	glPopAttrib();
}

void Matrix::HardMult(const Matrix & rhs)
{
	glPushAttrib(GL_TRANSFORM_BIT);
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		{
			glLoadIdentity();
			glMultMatrixf(m_values);
			glMultMatrixf(rhs.m_values);
			glGetFloatv(GL_MODELVIEW_MATRIX, m_values);
		}
		glPopMatrix();
	}
	glPopAttrib();
}

void Matrix::HardRotate(float angle, float * axis)
{
	glPushAttrib(GL_TRANSFORM_BIT);
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		{
			glLoadIdentity();
			glRotatef(angle, axis[0], axis[1], axis[2]);
			glGetFloatv(GL_MODELVIEW_MATRIX, m_values);
		}
		glPopMatrix();
	}
	glPopAttrib();
}

void Matrix::Transpose(Matrix * output)
{
	Matrix & result = ( (output) ? *output : *this);
	
	Matrix source(*this);
	
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			(&m_values[j * sizeof(float)])[i] = (&source.m_values[i * sizeof(float)])[j];
		}
	}
}

Matrix & Matrix::operator *(const Matrix & b)
{
	Matrix * m = new Matrix();

	for (int row = 0; row < 4; row++) 
	{ 
		for (int col = 0; col < 4; col++) 
		{ 
			float val = 0;
			val += get(row, 0) * (float)(b.get(0, col));
			val += get(row, 1) * b.get(1, col);
			val += get(row, 2) * b.get(2, col);
			val += get(row, 3) * b.get(3, col);
			m->set(row, col, val ); 
		}
	}

	return *m;
}

float Matrix::get(int row, int col ) const
{
	if ( row > 3 || col > 3 )
	{
		throw "error";
	}
	return m_values[row * 4 + col];
}

void Matrix::set(int row, int col, float value )
{
	if ( row > 3 || col > 3 )
	{
		return;
	}

	m_values[row * 4 + col] = value;
	return;
}
