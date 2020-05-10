/****************************************************************************************** 
 *	Chili DirectX Framework Version 16.07.20											  *	
 *	Game.cpp																			  *
 *	Copyright 2016 PlanetChili.net <http://www.planetchili.net>							  *
 *																						  *
 *	This file is part of The Chili DirectX Framework.									  *
 *																						  *
 *	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
 *	it under the terms of the GNU General Public License as published by				  *
 *	the Free Software Foundation, either version 3 of the License, or					  *
 *	(at your option) any later version.													  *
 *																						  *
 *	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
 *	GNU General Public License for more details.										  *
 *																						  *
 *	You should have received a copy of the GNU General Public License					  *
 *	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
 ******************************************************************************************/
#include "MainWindow.h"
#include "Game.h"
#include "PostProcess.h"
Game::Game( MainWindow& wnd )
	:
	wnd( wnd ),
	gfx( wnd )
{
	initDrawX();

}

void Game::Go()
{
	gfx.BeginFrame();	
	UpdateModel();
	ComposeFrame();
	gfx.EndFrame();

}

void Game::initDrawX()
{

	uint index[36] = {
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};
	Vector4d v[8] = {
	Vector4d(-4.0f, -4.0f, -4.0f),
	Vector4d(-4.0f, +4.0f, -4.0f),
	Vector4d(+4.0f, +4.0f, -4.0f),
	Vector4d(+4.0f, -4.0f, -4.0f),
	Vector4d(-4.0f, -4.0f, +4.0f),
	Vector4d(-4.0f, +4.0f, +4.0f),
	Vector4d(+4.0f, +4.0f, +4.0f),
	Vector4d(+4.0f, -4.0f, +4.0f),
	};

	uint tindex[36] = {
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		
		
		// top face
		0,1 ,2,
		0,2, 3,

		// bottom face
		0, 1, 2,
		0, 2,3,
	
	};

	Vector2d t[12] = {
		Vector2d(0.f,1.f),
		Vector2d(0.f,0.f),
		Vector2d(1.f,0.f),
		Vector2d(1.f,1.f),
		Vector2d(1.f,1.f),
		Vector2d(1.f,0.f),
		Vector2d(0.f,0.f),
		Vector2d(0.f,1.f),
		Vector2d(0.f,1.f),
		Vector2d(0.f,0.f),
		Vector2d(1.f,0.f),
		Vector2d(1.f,1.f),
	};

	MATRIX4X4 mrz, mrx, mry;
	
	

	dx.setindex(index, 36);
	dx.setvertx(v, 8);
	

	Vector4d v1(0.f, 0.f,1.0f);
	Vector4d dir(0, 0, 0);
	dx.Setfar(10.f, 100.f);
	dx.setCameraTarget(v1, dir);
	float aspect = 1280.f / 720.f;
	dx.SetScreen(aspect, 90.f, 1);
	dx.SetReslotion(1280, 720);
	dx.settexcord(t, 12);
	dx.settindex(tindex, 36);
	//dx.TemporAASapmle();
	gfx.Load2D();
	//PostProcess::GetApplcation()->SetDepth(0, 0,5.0f);
	//float Post = PostProcess::GetApplcation()->GetDepth(0, 0);
	
	



	int i = 0;


}



void Game::UpdateModel()
{
	
	
}

void Game::ComposeFrame()
{

	static int process = 0;
	MATRIX4X4 mrz, mrx, mry;

	static float x = 45.f;
	static float y =0.f;
	static float z = 0.f;

	Mat_Init_4X4(&mrz, Cos(z), -Sin(z), 0, 0,
		Sin(z), Cos(z), 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);

	Mat_Init_4X4(&mrx, 1.f, 0, 0, 0,
		0, Cos(x), -Sin(x), 0,
		0, Sin(x), Cos(x), 0,
		0, 0, 0, 1.f
	);

	Mat_Init_4X4(&mry, Cos(y), 0, -Sin(y), 0,
		0, 1, 0, 0,
		Sin(y), 0, Cos(y), 0,
		0, 0, 0, 1
	);

	
	if (wnd.kbd.KeyIsPressed('X'))
	{
		
		x += 1.f;
		wnd.kbd.FlushKey();
	}

	if (wnd.kbd.KeyIsPressed('Y'))
	{

		y += 1.f;
		wnd.kbd.FlushKey();
	}


	dx.copytemp();
	dx.Rotate(mrx);
	dx.Rotate(mrz);
	dx.Rotate(mry);
	
	dx.setWorldLocation(Vector4d(0.f, 0.f, 20.f));
	dx.TemporAASapmle();
	dx.BackRemove();
	dx.RelativetoWorld();
	dx.DrawTri(gfx);
	

	static float s = 1.0f;
	if (wnd.kbd.KeyIsPressed('S'))
	{

		s += 0.f;
		wnd.kbd.FlushKey();
	}




	if (!process)
	{
		gfx.CopyColor();
		++process;
	}
	else
	{
		gfx.postprocessTemporaa(dx.getjx(), dx.getjy(),dx);
		gfx.CopyColor();
	}
	//gfx.Draw_FillTri(150.f, 100.f, 0.5f, 100.f, 100.f, 0.4f, 200.f, 200.f, 0.3f);
	//gfx.Draw_FillTri(100.f, 100.f, 0.5f, 120.f, 200.f, 0.4f, 200.f, 200.f, 0.3f);
	
	
	dx.camera.CopytoPrevMat();
	PostProcess::GetApplcation()->RestDepth();


}





