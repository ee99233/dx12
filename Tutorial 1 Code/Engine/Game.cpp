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
	Vector4d(-1.0f, -1.0f, -1.0f),
	Vector4d(-1.0f, +1.0f, -1.0f),
	Vector4d(+1.0f, +1.0f, -1.0f),
	Vector4d(+1.0f, -1.0f, -1.0f),
	Vector4d(-1.0f, -1.0f, +1.0f),
	Vector4d(-1.0f, +1.0f, +1.0f),
	Vector4d(+1.0f, +1.0f, +1.0f),
	Vector4d(+1.0f, -1.0f, +1.0f),
	};
	MATRIX4X4 mrz, mrx, mry;
	
	

	dx.setindex(index, 36);
	dx.setvertx(v, 8);
	
	Vector4d v1(0, 0,0);
	Vector4d dir(0, 0, 0);
	dx.setCameraTarget(v1, dir);
	float aspect = 800.f / 600.f;
	dx.SetScreen(aspect, 90.f, 1);
	dx.SetReslotion(800, 600);
	gfx.Load2D();

	

}



void Game::UpdateModel()
{
}

void Game::ComposeFrame()
{

	/*MATRIX4X4 mrz, mrx, mry;
	static float x = 0.f;
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

	gfx.Drawline(50, 50, 100, 100);
	gfx.DrawTri(50, 200,100 ,100,200,200 );
	gfx.Drawline(50, 200, 100, 100);
	
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
	if (wnd.kbd.KeyIsPressed('Z'))
	{

		z += 1.f;
		wnd.kbd.FlushKey();
	}
	

	dx.copytemp();
	dx.Rotate(mrx);
	dx.Rotate(mrz);
	dx.Rotate(mry);
	dx.setWorldLocation(Vector4d(0.f, 0.f, 5.f));
	dx.BackRemove();
	dx.RelativetoWorld();
	dx.DrawTri(gfx);
	gfx.PutPixel(400, 300, Color(111111000));*/

	gfx.Draw_FillTri(500, 100, 756, 100,500, 356,0.f,0.f,1.f,0.f,0.0f ,1.f,gfx.img);
	gfx.Draw_FillTri(756, 356, 500, 356, 756, 100, 1.f, 1.f, 0.f, 1.f, 1.0f, 0.f, gfx.img);

	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 256; j++)
		{
			UINT color = gfx.img[j+i*256];
			gfx.PutPixel(i, j, Color(color));
		}
	}

	/*gfx.Draw_tr(100.f,100.f,150.f,200.f,50.f,200.f);

	gfx.Draw_tr(100.f, 100.f, 150.f, 100.f, 50.f, 200.f);
*/
	/*gfx.Draw_tr(439.325702f,205.390976f,500.16449f,340.353180f,369.730957f,403.943359f);
	gfx.Draw_tr(439.325702f, 205.390976f, 300.760193f, 272.970093f, 369.730957f, 403.943359f);*/


}





