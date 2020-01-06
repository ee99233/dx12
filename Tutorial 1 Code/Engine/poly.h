#pragma once
#include "Xmath.h"
using namespace XMath;
class poly
{
public:
	poly();
	~poly();
	void Setpoly(Point4d* poly,int index);
private:
	int index[3];
	Point4d polypoint[3];
	bool isremove;
	Point2d texture[3];
	
};

