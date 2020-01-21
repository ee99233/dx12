#include "Light.h"


Light::Light()
{
}

Light:: Light(Lighttype lightype, float dmin, float dmax, float r, float g, float b, Vector4d v4d)
{
	this->lightype = lightype;
	this->dmin = dmin;
	this->dmax = dmax;
	this->r = r;
	this->g = g;
	this->b = b;
	this->postion = v4d;

}

float Light::getdmin() const
{
	return dmin;
}

float Light::getdmax() const
{
	return dmax;
}

float Light::getr() const
{
	return r;
}

float Light::getg() const
{
	return g;
}

float Light::getb() const
{
	return b;
}

Vector4d Light::getpostion()
{
	return postion;
}

Lighttype Light::getlightype() const
{
	return lightype;
}



Light::~Light()
{


}
