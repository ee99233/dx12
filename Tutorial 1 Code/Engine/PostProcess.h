#pragma once
#include <memory>
using namespace std;
class PostProcess
{
public:
	PostProcess();
	~PostProcess();
	static PostProcess* GetApplcation();
	float GetDepth(int width, int height) const 
	{
		
		return (*depth)[1280 * height + width];
	}
	void SetDepth(int width,int height, float depthnum)
	{

		(*depth)[1280 * height + width] = depthnum;
	}
	std::shared_ptr<float*> depth ;
private:
	static PostProcess* postprocess;

};
