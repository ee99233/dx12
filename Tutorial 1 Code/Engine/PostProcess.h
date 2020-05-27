#pragma once
#include <memory>
using namespace std;
struct MotionVector {
	float MotingX;
	float MotingY;
	MotionVector() {
		MotingX = 0.0f;
		MotingY = 0.0f;
	}

};
class PostProcess
{
public:
	PostProcess();
	~PostProcess();
	void RestDepth();
	void ResetMotinVec();
	static PostProcess* GetApplcation();
	float GetDepth(int width, int height) const 
	{
		
		return depth[1280 * height + width];
	}
	void SetDepth(int width,int height, float depthnum)
	{

		depth[1280 * height + width] = depthnum;
	}

	MotionVector Getmotionvector(int width, int height) const
	{

		return motionvector[1280 * height + width];
	}
	void Setmotionvector(int width, int height, MotionVector motionvector)
	{

		this->motionvector[1280 * height + width] = motionvector;
	}
	float* depth ;
	MotionVector* motionvector;
private:
	static PostProcess* postprocess;

};
