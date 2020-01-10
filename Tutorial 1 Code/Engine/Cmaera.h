#pragma once
#include "Xmath.h"

using namespace XMath;

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
	 Point4d getpos() const
	 {
		 return pos;
	 }
	 Point4d gettarget() const
	 {
		 return target;
	 }
	 MATRIX4X4 getcam() const ;
	 MATRIX4X4 getmscr() const;
	 MATRIX4X4 getmper() const;
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

	float viewplane_width;//��ƽ����
	float viewplane_height;//��ƽ��߶�

	float viewport_width;//�ӿڿ��
	float viewport_heght;//�ӿڸ߶�
	float viewport_center_x;//�ӿ�����x
	float viewport_center_y;//�ӿ�����y
	float aspect_radio;
	MATRIX4X4 mcam; //uvn
	MATRIX4X4 mper; //͸�ӱ任����
	MATRIX4X4 mscr; //��Ļ�任����
};

