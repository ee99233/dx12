
#include <math.h>
#include "Xmath.h"
#include "Colors.h"

void XMath::CrossProduct(Vector4D_Ptr result, Vector4D_Ptr v1, Vector4D_Ptr v2)
{
	result->x = v1->y*v2->z - v1->z*v2->y;
	result->y = v1->z*v2->x - v1->x*v2->z;
	result->z = v1->x*v2->y - v1->y*v2->x;
	result->w = 1;

}

void XMath::CopyVector(Vector4D_Ptr v1, Vector4D_Ptr v2)
{
	v1->x = v2->x;
	v1->y = v2->y;
	v1->z = v2->z;
	v1->w = 1;

}

void XMath::NormlVecotr(Vector4D_Ptr v)
{
	float length = sqrt(v->x*v->x + v->y*v->y + v->z*v->z);
	if(length > 0.0f)
	{
		v->x = v->x / length;
		v->y = v->y / length;
		v->z = v->z / length;
		v->w = 1;
	}
	else
	{
		v->x = 0;
		v->y = 0;
		v->z = 0;
		v->w = 1;
	}

}

float XMath::DotProduct(Vector4D_Ptr v1, Vector4D_Ptr v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
}

float XMath::DotProduct(Vector2D_Ptr v1, Vector2D_Ptr v2)
{
	return v1->u*v2->u + v1->v*v2->v;
}

float XMath::Cos(float angle)
{
	return cos((angle / 180.0f)*PI);
}

float XMath::Sin(float angle)
{
	return sin((angle / 180.0f)*PI);
}

float XMath::Tan(float angle)
{
	return tan((angle / 180.0f)*PI);
}

void XMath::SubVector(Vector4d & v1, Vector4d & v2, Vector4d & v3)
{

	v3.x = v2.x - v1.x;
	v3.y = v2.y - v1.y;
	v3.z = v2.z - v1.z;
	v3.w = 1;

}

void XMath::addVector(Vector4d & v1, Vector4d & v2, Vector4d & v3)
{
	v3.x = v1.x + v2.x;
	v3.y = v1.y + v2.y;
	v3.z = v1.z + v2.z;
	v3.w = 1;
}

void XMath::Mat_Init_4X4(MATRIX4X4_PTR ma, float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	ma->M00 = m00; ma->M01 = m01; ma->M02 = m02; ma->M03 = m03;
	ma->M10 = m10; ma->M11 = m11; ma->M12 = m12; ma->M13 = m13;
	ma->M20 = m20; ma->M21 = m21; ma->M22 = m22; ma->M23 = m23;
	ma->M30 = m30; ma->M31 = m31; ma->M32 = m32; ma->M33 = m33;
}

void XMath::Mat_Mul_4X4(MATRIX4X4_PTR ma, MATRIX4X4_PTR mb, MATRIX4X4_PTR mc)
{
	for (int row = 0; row < 4; row++)
	{
		
		for (int column = 0; column < 4; column++)
		{
			float temp = 0.f;
			for (int i = 0; i < 4; i++)
			{
				temp += ma->M[row][i] * mb->M[i][column];
			}
			mc->M[row][column] = temp;
		}
		
	}


}

void XMath::V4d_Mul_4X4(Vector4d  v, MATRIX4X4 & M, Vector4d &result)
{
	result.x = v.x*M.M[0][0] + v.y*M.M[1][0] + v.z*M.M[2][0] + v.w*M.M[3][0];
	result.y = v.x*M.M[0][1] + v.y*M.M[1][1] + v.z*M.M[2][1] + v.w*M.M[3][1];
	result.z = v.x*M.M[0][2] + v.y*M.M[1][2] + v.z*M.M[2][2] + v.w*M.M[3][2];
	result.w = v.x*M.M[0][3] + v.y*M.M[1][3] + v.z*M.M[2][3] + v.w*M.M[3][3];
	int i = 0;

}

void XMath::V4d_Mul_4X4(MATRIX4X4& M, Vector4d  v,Vector4d &result)
{
	result.x = v.x*M.M[0][0] + v.y*M.M[0][1] + v.z*M.M[0][2] + v.w*M.M[0][3];
	result.y = v.x*M.M[1][0] + v.y*M.M[1][1] + v.z*M.M[1][2] + v.w*M.M[1][3];
	result.z = v.x*M.M[2][0] + v.y*M.M[2][1] + v.z*M.M[2][2] + v.w*M.M[2][3];
	result.w = 1;
}

int XMath::Max(const int & a, const int & b)
{
	return a > b ? a : b;
}

int XMath::Min(const int & a, const int & b)
{
	return a<b?a:b;
}

float XMath::CrossProductF(int x1, int x2, int y1, int y2)
{
	return x1 * y2 - x2 * y1;
}

float XMath::CrossProductF(float x1, float x2, float y1, float y2)
{
	return x1 * y2 - x2 * y1;
}

bool XMath::insideangle(int vx1, int vy1, int vx2, int vy2, int vx3, int vy3, int vx0, int vy0)
{
	float s1 = CrossProductF(vx1, vy1, vx2, vy2);
	float s2 = CrossProductF(vx0, vy0, vx1, vy1);
	float s3 = CrossProductF(vx0, vy0, vx2, vy2);
	float s4 = CrossProductF(vx0, vy0, vx3, vy3);

	float u = s2 / s1;
	float v = s3 / s1;
	float c = 1.f - u - v;
	return u >= 0 && v >= 0 && c >= 0;
}

float XMath::dlength(float dmax, float dmin, float d)
{
	if (d <= 0.f)
	{
		return 0.f;
	}
	else if (d >= 1.0f)
	{
		return 1.0f;
	}
	else
	{
		return dmax - d / dmax - dmin;
	}
}

float XMath::getlength(Vector4d & v1, Vector4d & v2)
{

	float x = v1.x - v2.x;
	float y = v1.y - v2.y;
	float z = v1.z - v2.z;

	return sqrt(x*x + y * y + z * z);
}

void XMath::InversePos(const MATRIX4X4 & M1, MATRIX4X4 & M2)
{
	
		float a = - M1.M[3][0];
	
	
		float b =- M1.M[3][1];
	
		float c = - M1.M[3][2];

	M2.M[0][0] = 1; M2.M[0][1] = 0; M2.M[0][2] = 0; M2.M[0][3] = 0;
	M2.M[1][0] = 0; M2.M[1][1] = 1; M2.M[1][2] = 0; M2.M[1][3] = 0;
	M2.M[2][0] = 0; M2.M[2][1] = 0; M2.M[2][2] = 1; M2.M[2][3] = 0;
	M2.M[3][0] = a; M2.M[3][1] = b; M2.M[3][2] = c; M2.M[3][3] = 1;


}

void XMath::Transposed(const MATRIX4X4 & M1, MATRIX4X4 & M2)
{

	M2.M[0][0] = M1.M[0][0]; M2.M[0][1] = M1.M[1][0]; M2.M[0][2] = M1.M[2][0]; M2.M[0][3] = M1.M[3][0];
	M2.M[1][0] = M1.M[0][1]; M2.M[1][1] = M1.M[1][1]; M2.M[1][2] = M1.M[2][1]; M2.M[1][3] = M1.M[3][1];
	M2.M[2][0] = M1.M[0][2]; M2.M[2][1] = M1.M[1][2]; M2.M[2][2] = M1.M[2][2]; M2.M[2][3] = M1.M[3][2];
	M2.M[3][0] = M1.M[3][0]; M2.M[3][1] = M1.M[1][3]; M2.M[3][2] = M1.M[2][3]; M2.M[3][3] = M1.M[3][3];
}

void XMath::inversepro(const MATRIX4X4 & M1, MATRIX4X4 & M2)
{
	float a  = 1.0f / M1.M[0][0];;
	float b = 1.0f / M1.M[1][1];
	float c= 1.0f / M1.M[3][2];
	float d =-M1.M[2][0] / M1.M[0][0];
	float e = -M1.M[2][1] / M1.M[1][1];
	float f = -M1.M[2][2] / M1.M[3][2];
	
	
	M2.M[0][0] = a; M2.M[0][1] = 0.0f; M2.M[0][2] = 0.0f; M2.M[0][3] = 0.0f;
	M2.M[1][0] = 0.0f; M2.M[1][1] = b; M2.M[1][2] = 0.0f; M2.M[1][3] = 0.0f;
	M2.M[2][0] = 0.0f; M2.M[2][1] = 0.0f; M2.M[2][2] = 0.0f; M2.M[2][3] = c;
	M2.M[3][0] = d; M2.M[3][1] = e; M2.M[3][2] = 1.0f; M2.M[3][3] = f;
	

}

void XMath::inversemcar(const MATRIX4X4 & M1, MATRIX4X4 & M2)
{
	float a = 1.0f / M1.M[0][0];
	float b = 1.0f / M1.M[1][1];
	M2.M[0][0] = a; M2.M[0][1] = 0.0f; M2.M[0][2] = 0.0f; M2.M[0][3] = 0.0f;
	M2.M[1][0] = 0.0f; M2.M[1][1] = b; M2.M[1][2] = 0.0f; M2.M[1][3] = 0.0f;
	M2.M[2][0] = 0.0f; M2.M[2][1] = 0.0f; M2.M[2][2] = 1.0f; M2.M[2][3] = 0.0f;
	M2.M[3][0] = -1.0f; M2.M[3][1] = 1.0f; M2.M[3][2] = 0.0f; M2.M[3][3] = 1.0f;

}

void XMath::CopyMaTR(const MATRIX4X4& M1, MATRIX4X4& M2)
{

	M2.M[0][0] = M1.M[0][0]; M2.M[0][1] = M1.M[0][1]; M2.M[0][2] = M1.M[0][2]; M2.M[0][3] = M1.M[0][3];
	M2.M[1][0] = M1.M[1][0]; M2.M[1][1] = M1.M[1][1]; M2.M[1][2] = M1.M[1][2]; M2.M[1][3] = M1.M[1][3];
	M2.M[2][0] = M1.M[2][0]; M2.M[2][1] = M1.M[2][1]; M2.M[2][2] = M1.M[2][2]; M2.M[2][3] = M1.M[2][3];
	M2.M[3][0] = M1.M[3][0]; M2.M[3][1] = M1.M[3][1]; M2.M[3][2] = M1.M[3][2]; M2.M[3][3] = M1.M[3][3];

}

void XMath::RGBTOYCOCG(MXFlaot3 & color)
{
	float red = color.x * (1 / 4) + 1 / 2 * color.y + 1 / 4 * color.z;
	float green = color.x * (1 / 2)  - 1 / 2 * color.z;
	float blue = color.x *(-1 / 4) + 1 / 2 * color.y + (-1 / 4) * color.z;
	color.x=red;
	color.y=green;
	color.z=blue;

}

void XMath::YCOCGTORGB(MXFlaot3 & color)
{
	float red = color.x  +  color.y - color.z;
	float green = color.x +  color.z;
	float blue = color.x  - color.y - color.z;

	color.x=red;
	color.y=green;
	color.z= blue;

}




