#include "PostProcess.h"



 PostProcess* PostProcess::postprocess = nullptr;
PostProcess::PostProcess()
{
	depth = make_shared<float*>(new float[1280 * 720]);
	
}


PostProcess::~PostProcess()
{
}

PostProcess * PostProcess::GetApplcation()
{
	if (postprocess == nullptr)
	{
		postprocess = new PostProcess();
	}
	
	return postprocess;
}
