#pragma once
#define PI 3.1415927
class Color;
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
		Vector_Typ(float x1 ,float y1,float z1, float w1 = 1.f)
		{
			x = x1;
			y = y1;
			z = z1;
			w =w1;

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

	typedef struct MXFlaot3
	{
		float x;
		float y;
		float z;

		MXFlaot3& operator-=(const MXFlaot3& mf1 )
		{
		
			this->x= this->x - mf1.x;
			this->y = this->y - mf1.y;
			this->z = this->z - mf1.z;
			return *this;
			
		}

		MXFlaot3& operator+=(const MXFlaot3& mf1)
		{

			this->x = this->x + mf1.x;
			this->y = this->y + mf1.y;
			this->z = this->z + mf1.z;
			return *this;
		}
		
		MXFlaot3 operator+(const MXFlaot3& mf1) const
		{
			MXFlaot3 m3;
			m3.x = this->x + mf1.x;
			m3.y = this->y + mf1.y;
			m3.z = this->z + mf1.z;
			return m3;

		}

		MXFlaot3 operator-(const MXFlaot3& mf1) const
		{
			MXFlaot3 m3;
			m3.x = this->x - mf1.x;
			m3.y = this->y - mf1.y;
			m3.z = this->z - mf1.z;
			return m3;

		}
		MXFlaot3& operator=(const MXFlaot3& mf1)
		{
			
			 this->x = mf1.x;
			 this->y = mf1.y;
			 this->z = mf1.z;
			return *this;

		}
		MXFlaot3() 
		{
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
		}
		
	};

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

		MATRIX4X4_TYP&	operator =(const MATRIX4X4_TYP& Ma)
		{
			this->M[0][0] =Ma.M[0][0];
			this->M[0][1] = Ma.M[0][1];
			this->M[0][2] = Ma.M[0][2];
			this->M[0][3] = Ma.M[0][3];
			this->M[1][0] = Ma.M[1][0];
			this->M[1][1] = Ma.M[1][1];
			this->M[1][2] = Ma.M[1][2];
			this->M[1][3] = Ma.M[1][3];
			this->M[2][0] = Ma.M[2][0];
			this->M[2][1] = Ma.M[2][1];
			this->M[2][2] = Ma.M[2][2];
			this->M[2][3] = Ma.M[2][3];
			this->M[3][0] = Ma.M[3][0];
			this->M[3][1] = Ma.M[3][1];
			this->M[3][2] = Ma.M[3][2];
			this->M[3][3] = Ma.M[3][3];
			return *this;
		}
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

	void addVector(Vector4d &v1, Vector4d &v2, Vector4d &v3);

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
	float CrossProductF(float x1, float x2, float y1, float y2);

	bool insideangle(int vx1, int vy1, int vx2, int vy2, int vx3, int vy3, int vx0, int vy0);


	float dlength(float dmax, float dmin, float d);

	float getlength(Vector4d &v1, Vector4d& v2);


	void InversePos(const MATRIX4X4& M1, MATRIX4X4& M2);
	void Transposed(const MATRIX4X4& M1, MATRIX4X4& M2);
	void inversepro(const MATRIX4X4& M1, MATRIX4X4& M2);
	void inversemcar(const MATRIX4X4& M1, MATRIX4X4& M2);
	void CopyMaTR(const MATRIX4X4& M1, MATRIX4X4& M2);
	void RGBTOYCOCG(MXFlaot3 &color);
	void YCOCGTORGB(MXFlaot3& color);
}

