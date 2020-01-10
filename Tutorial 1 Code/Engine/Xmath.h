#pragma once
#define PI 3.1415927

namespace XMath 
{

	typedef struct Vector_Typ
	{
		union
		{
			float M[4];
			struct
			{
				float x, y, z, w;
			};
			
		};
		Vector_Typ(float x1 ,float y1,float z1, float w = 1.f)
		{
			x = x1;
			y = y1;
			z = z1;
			w = 1;

		}
		Vector_Typ() {}
	}Vector4d, *Vector4D_Ptr ,Point4d, *Point4_Ptr;



	typedef struct Vector2d_Typ
	{
		union
		{
			float M[2];
			struct
			{
				float u, v;
			};

		};
		Vector2d_Typ(float u1, float v1)
		{
			u = u1;
			v = v1;
		}
		Vector2d_Typ() {}
	}Vector2d, *Vector2D_Ptr, Point2d,*Point2_Ptr;

	typedef struct MATRIX4X4_TYP 
	{
		union
		{

			float M[4][4];
			struct
			{
				float M00, M01, M02, M03;
				float M10, M11, M12, M13;
				float M20, M21, M22, M23;
				float M30, M31, M32, M33;

			};

		};


	}MATRIX4X4,*MATRIX4X4_PTR;


	

	void CrossProduct(Vector4D_Ptr result, Vector4D_Ptr v1, Vector4D_Ptr v2);

	void CopyVector(Vector4D_Ptr v1, Vector4D_Ptr v2);

	void NormlVecotr(Vector4D_Ptr v);

	float DotProduct(Vector4D_Ptr v1, Vector4D_Ptr v2);

	float DotProduct(Vector2D_Ptr v1, Vector2D_Ptr v2);

	float Cos(float angle);

	float Sin(float angle);

	float Tan(float angle);


	void SubVector(Vector4d &v1, Vector4d &v2, Vector4d &v3);

	void Mat_Init_4X4(MATRIX4X4_PTR ma,
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33);

	void Mat_Mul_4X4(MATRIX4X4_PTR ma, MATRIX4X4_PTR mb, MATRIX4X4_PTR mc);

	void V4d_Mul_4X4(Vector4d v, MATRIX4X4& M,Vector4d &result);
	void V4d_Mul_4X4(MATRIX4X4& M, Vector4d v, Vector4d &result);

	 int Max(const int& a, const int&b);
	 int Min(const int&a, const int&b);

	float CrossProductF(int x1, int x2, int y1, int y2);

	bool insideangle(int vx1, int vy1, int vx2, int vy2, int vx3, int vy3, int vx0, int vy0);
}

