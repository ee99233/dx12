#pragma once
#include "Xmath.h"

using namespace XMath;



typedef struct FMATRIX {
	MATRIX4X4 mcam; //uvn
	MATRIX4X4 mper; //透视变换矩阵
	MATRIX4X4 mscr; //屏幕变换矩阵
	MATRIX4X4 trans;
	MATRIX4X4 uvn;

	MATRIX4X4 inversemcam; //uvn
	MATRIX4X4 inversemper; //透视变换矩阵
	MATRIX4X4 inversemscr; //屏幕变换矩阵
	MATRIX4X4 inversetrans;
	MATRIX4X4 inverseuvn;

}FMATRIX;
class Cmaera
{
public:
	Cmaera();
	~Cmaera();
	 void initmcam();
	 void initmper();
	 void initmscr();
	 void SetPos(const Vector4d& pos);
	 void Setfar(float near1,float far1);
	 void SetDir(const float Xangle, const float& Zangle);
	 void SetScreen(float aspect_radio, float fov,int iewport_width);
	 void SetReslotion(float screenwidth, float screenheight);
	 void TemporalAASample();
	 void CopytoPrevMat();
	 Point4d getpos() const
	 {
		 return pos;
	 }
	 Point4d gettarget() const
	 {
		 return target;
	 }

	 float getjx() const
	 {
		 return JitterX;
	 }
	 float getjy() const
	 {
		 return JitterY;
	 }
	 MATRIX4X4 getcam() const ;
	 MATRIX4X4 getmscr() const;
	 MATRIX4X4 getmper() const;
	 FMATRIX ViewMATRIX;
	 FMATRIX PrevViewMATRIX;
private:
	Vector4d pos;
	Vector4d u;
	Vector4d v;
	Vector4d n;
	Vector4d dir;
	float fov;
	Point4d target;
	float nearview;
	float farview;

	float viewplane_width;//视平面宽度
	float viewplane_height;//视平面高度

	float viewport_width;//视口宽度
	float viewport_heght;//视口高度
	float viewport_center_x;//视口中心x
	float viewport_center_y;//视口中心y
	float aspect_radio;
	int hutn = 0;
	float JitterX;
	float JitterY;

	
	
};

