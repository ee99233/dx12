#include "DrawX.h"
#include "Cmaera.h"
#include "MainWindow.h"

DrawX::DrawX() 
{

	index = nullptr;
	vertx = nullptr;
}



DrawX::~DrawX()
{

	if (index != nullptr)
	{
		delete[] index;
		index = nullptr;
	}
	if (vertx!=nullptr)
	{
		delete[] vertx;
		vertx = nullptr;
	}
}

void DrawX::setWorldLocation(const Vector4d & world)
{

	for (int i = 0; i <vsize ; i++)
	{
		tempx[i].x += world.x;
		tempx[i].y += world.y;
		tempx[i].z += world.z;
	}
}





void DrawX::setindex(uint * in,int size)
{
	if (index != nullptr)
	{
		delete[] index;
		index = nullptr;
	}
	index = new uint[size];

	for (int i = 0; i < size; i++)
	{
		index[i] = in[i];

	}
	isize = size;
	isbackremove = new bool[size / 3];

}

void DrawX::setvertx(Point4_Ptr ver,int size)
{
	if (vertx != nullptr)
	{
		delete[] vertx;
	}
	vertx = new Point4d[size];
	tempx = new Point4d[size];
	for (int i = 0; i < size; i++)
	{
		vertx[i].x = ver[i].x;
		vertx[i].y = ver[i].y;
		vertx[i].z = ver[i].z;
		vertx[i].w = 1;

	}
	vsize = size;

}

void DrawX::DrawTri( Graphics& gfx)
{
	int b = 0;
	for (int i = 0; i < isize; i += 3)
	{

		if (isbackremove[b])
		{
			Vector4d v1 = tempx[index[i]];
			Vector4d v2 = tempx[index[i + 1]];
			Vector4d v3 = tempx[index[i + 2]];
			gfx.Draw_FillTri(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
		}
		++b;
	}

}

void DrawX::SetScreen(float aspect_radio, float fov, int iewport_width)
{
	camera.SetScreen(aspect_radio, fov, iewport_width);

}

void DrawX::SetReslotion(float screenwidth, float screenheight)
{
	camera.SetReslotion(screenwidth,screenheight);
}





void DrawX::setCameraTarget(const Vector4d & target, const Vector4d & dir)
{
	camera.SetPos(target);
	camera.SetDir(dir.x,dir.z);
	camera.initmcam();
	
}

void DrawX::RelativetoWorld()
{
	for (int i = 0; i < vsize; i++)
	{
		
		V4d_Mul_4X4(tempx[i],camera.getcam(),tempx[i]);
	
		V4d_Mul_4X4(tempx[i], camera.getmper(), tempx[i]);
		tempx[i].x = tempx[i].x / tempx[i].z;
		tempx[i].y = tempx[i].y / tempx[i].z;
		tempx[i].z = tempx[i].z / tempx[i].z;
		tempx[i].w = 1;
		V4d_Mul_4X4(tempx[i], camera.getmscr(), tempx[i]);
		int j = 0;

	}


}

void DrawX::BackRemove()
{

	int b = 0;
	for (int i = 0; i < isize; i+=3)
	{

		Vector4d sub1, sub2,sub3;
		Vector4d cross;
		SubVector(tempx[index[i]], tempx[index[i + 1]], sub1);
		SubVector(tempx[index[i+1]], tempx[index[i + 2]], sub2);
		CrossProduct(&cross,&sub1, &sub2);
		NormlVecotr(&cross);
		SubVector(camera.gettarget(), tempx[index[i]],sub3);
		NormlVecotr(&sub3);

		float dot = DotProduct(&cross, &sub3);
		float d = sqrt(2) / 2.f;
		if (dot <0.0f)
		{
			isbackremove[b] = true;

		}
		else
		{
			isbackremove[b] = false;
		}

		++b;
	}

}

void DrawX::Rotate(MATRIX4X4 & r)
{

	for (int i = 0; i < vsize; i++)
	{
		//V4d_Mul_4X4(tempx[i],r);
		
		V4d_Mul_4X4(r, tempx[i],tempx[i]);
	}
}

void DrawX::copytemp()
{
	for (int i = 0; i < 8; i++)
	{
		CopyVector(&tempx[i], &vertx[i]);
	}
}


