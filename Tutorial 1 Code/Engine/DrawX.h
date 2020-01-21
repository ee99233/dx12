#pragma once
#include "XMath.h"
#include <vector>
#include "Cmaera.h"
#include "Light.h"
using namespace XMath;
typedef unsigned int uint;

class DrawX
{
public:
	DrawX();
	~DrawX();
	void setWorldLocation(const Vector4d &world);
	void setNorml();
	void setindex(uint* in,int size);
	void settindex(uint* tindex, int size);
	void setvertx(Point4_Ptr ver,int size);
	void settexcord(Point2_Ptr ver, int size);
	void DrawTri( class Graphics& gfx);
	void SetScreen(float aspect_radio, float fov, int iewport_width);
	void SetReslotion(float screenwidth, float screenheight);
	void setCameraTarget(const Vector4d& target, const Vector4d& dir);
	void RelativetoWorld();
	void BackRemove();
	void Setfar(float near1, float far1);
	void Rotate(MATRIX4X4 &r);
	void copytemp();
private:
	uint* index;
	uint* tindex;
	int isize;
	Point4_Ptr vertx;
	Point4_Ptr tempx;
	Point2d* utex;
	int vsize;
	int bsize;
	Cmaera camera;
	bool* isbackremove;
	class poly* pol;
	Light light[3];
	int lightsize;
	Point4_Ptr norml;
	int normlsize;
	
};

