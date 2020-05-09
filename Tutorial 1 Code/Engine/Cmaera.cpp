#include "Cmaera.h"



Cmaera::Cmaera()
{
}


Cmaera::~Cmaera()
{
}

void Cmaera::initmcam()
{
	MATRIX4X4 mpos;
	Mat_Init_4X4(&mpos, 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-pos.x, -pos.y, -pos.z, 1
	);

	ViewMATRIX.trans = mpos;
	InversePos(mpos, ViewMATRIX.inversetrans);
	float phi = dir.x;
	float theta = dir.z;

	target.z = Cos(theta);
	target.x = Sin(theta)*Cos(phi);
	target.y = Sin(theta)*Sin(phi);

	/*target.z = 1;
	target.y = 0;
	target.x = 0;*/

	CopyVector(&n, &target);
	v.x = 0;
	v.y = 1;
	v.z = 0;
	CrossProduct(&u, &v, &n);
	CrossProduct(&v, &n, &u);

	NormlVecotr(&u);
	NormlVecotr(&v);
	NormlVecotr(&n);

	MATRIX4X4 muvn;
	Mat_Init_4X4(&muvn, u.x, v.x, n.x, 0,
		u.y, v.y, n.y, 0,
		u.z, v.z, n.z, 0,
		0, 0, 0, 1
	);
	ViewMATRIX.uvn = muvn;
	Transposed(muvn, ViewMATRIX.inverseuvn);
	Mat_Mul_4X4(&mpos,&muvn, &ViewMATRIX.mcam);

	//Mat_Init_4X4(&mcam, u.x, v.x, n.x, 0,
	//	u.y, v.y, n.y, 0,
	//	u.z, v.z, n.z, 0,
	//	-pos.x, -pos.y, -pos.z, 1
	//);
	int i = 0;
}

void Cmaera::initmper()
{
	float d = viewplane_width / Tan((fov / 2.f));
	
	Mat_Init_4X4(&ViewMATRIX.mper, d/aspect_radio, 0, 0, 0,
		0, d, 0, 0,
		0, 0, farview/(farview-nearview), 1,
		0, 0, -(nearview*farview)/(farview - nearview), 0);

	inversepro(ViewMATRIX.mper, ViewMATRIX.inversemper);

	int i = 0;
}

void Cmaera::initmscr()
{
	float a = viewport_width / 2.f - 0.5f;
	float b = viewport_heght / 2.f - 0.5f;

	Mat_Init_4X4(&ViewMATRIX.mscr, a, 0, 0, 0,
		0, -b, 0, 0,
		0, 0, 1, 0,
		a, b, 0, 1
	);

	inversemcar(ViewMATRIX.mscr, ViewMATRIX.inversemscr);

}

void Cmaera::SetPos(const Vector4d & pos)
{

	this->pos.x = pos.x;
	this->pos.y = pos.y;
	this->pos.z = pos.z;
	this->pos.w = pos.w;
}

void Cmaera::Setfar(float near1, float far1)
{
	this->farview=far1;
	this->nearview = near1;
}

void Cmaera::SetDir(const float Xangle, const float & Zangle)
{
	dir.x = Xangle;
	dir.z = Zangle;
}

void Cmaera::SetScreen(float aspect_radio, float fov, int iewport_width)
{

	this->aspect_radio = aspect_radio;
	this->fov = fov;
	this->viewplane_width = this->viewplane_height = iewport_width;
	initmper();
}

void Cmaera::SetReslotion(float screenwidth, float screenheight)
{

	viewport_width = screenwidth;
	viewport_heght = screenheight;
	initmscr();
}

MATRIX4X4 Cmaera::getcam() const
{
	return ViewMATRIX.mcam;
}

MATRIX4X4 Cmaera::getmscr() const
{
	return ViewMATRIX.mscr;
}

MATRIX4X4 Cmaera::getmper() const
{
	return ViewMATRIX.mper;
}


