#include "PostProcess.h"



 PostProcess* PostProcess::postprocess = nullptr;
PostProcess::PostProcess()
{
	depth = new float[1280 * 720];
	
}


PostProcess::~PostProcess()

{
	if (depth != nullptr)
	{
		delete[] depth;
		depth = nullptr;
	}
}

void PostProcess::RestDepth()
{
	if (depth!=nullptr)
	{
		delete[] depth;
		depth = nullptr;
	}

	depth = new float[1280 * 720];
}

PostProcess * PostProcess::GetApplcation()
{
	if (postprocess == nullptr)
	{
		postprocess = new PostProcess();
	}
	
	return postprocess;
}
