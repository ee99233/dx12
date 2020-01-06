#pragma once
#include "XMath.h"
#include <vector>
#include "Cmaera.h"
using namespace XMath;
typedef unsigned int uint;

class DrawX
{
public:
	DrawX();
	~DrawX();
	void setWorldLocation(const Vector4d &world);
	void setindex(uint* in,int size);
	void setvertx(Point4_Ptr ver,int size);
	void DrawTri( class Graphics& gfx);
	void SetScreen(float aspect_radio, float fov, int iewport_width);
	void SetReslotion(float screenwidth, float screenheight);
	void setCameraTarget(const Vector4d& target, const Vector4d& dir);
	void RelativetoWorld();
	void BackRemove();
	void Rotate(MATRIX4X4 &r);
	void copytemp();
private:
	uint* index;
	int isize;
	Point4_Ptr vertx;
	Point4_Ptr tempx;
	int vsize;
	int bsize;
	Cmaera camera;
	bool* isbackremove;
	class poly* pol;
	
};

