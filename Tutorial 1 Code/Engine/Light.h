#pragma once
#include "Xmath.h"

using namespace XMath;

enum Lighttype
{
	Csepc = 0,
	Cdiff = 1,
	Camb = 2

};

class Light
{
public:
	Light();
	Light(Lighttype lightype,float dmin,float dmax,float r,float g, float b,Vector4d v4d);
	float getdmin() const;
	float getdmax() const;
	float getr() const;
	float getg() const;
	float getb() const;
	Vector4d getpostion();
	Lighttype getlightype() const;
	~Light();
private:
	Lighttype lightype;
	float dmin;
	float dmax;
	float r;
	float g;
	float b;
	Vector4d postion;
};

