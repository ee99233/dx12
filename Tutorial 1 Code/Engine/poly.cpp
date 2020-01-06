#include "poly.h"



poly::poly()
{
}


poly::~poly()
{
}

void poly::Setpoly(Point4d * poly, int index)
{

	if (index < 3)
	{
		polypoint[index].x = poly->x;
		polypoint[index].y = poly->y;
		polypoint[index].z = poly->z;
		polypoint[index].w = poly->w;
	}

}
