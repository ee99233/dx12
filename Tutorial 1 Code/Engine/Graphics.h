/******************************************************************************************
*	Chili DirectX Framework Version 16.07.20											  *
*	Graphics.h																			  *
*	Copyright 2016 PlanetChili <http://www.planetchili.net>								  *
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
#pragma once
#include <d3d11.h>
#include <wrl.h>
#include "ChiliException.h"
#include "Colors.h"
#include "Light.h"

class Graphics
{
public:
	class Exception : public ChiliException
	{
	public:
		Exception( HRESULT hr,const std::wstring& note,const wchar_t* file,unsigned int line );
		std::wstring GetErrorName() const;
		std::wstring GetErrorDescription() const;
		virtual std::wstring GetFullMessage() const override;
		virtual std::wstring GetExceptionType() const override;
	private:
		HRESULT hr;
	};
private:
	// vertex format for the framebuffer fullscreen textured quad
	struct FSQVertex
	{
		float x,y,z;		// position
		float u,v;			// texcoords
	};
public:
	Graphics( class HWNDKey& key );
	Graphics( const Graphics& ) = delete;
	Graphics& operator=( const Graphics& ) = delete;
	void EndFrame();
	void BeginFrame();
	void PutPixel( int x,int y,int r,int g,int b )
	{
		PutPixel( x,y,{ unsigned char( r ),unsigned char( g ),unsigned char( b ) } );
	}
	void Drawline(int x1, int y1, int x2, int y2);
	void PutPixel( int x,int y,Color c );
	void DrawTri(int x1, int y1, int x2, int y2, int x3, int y3);
	void Draw_Circle(int r,int x,int y);
	void Draw_tr(int x1, int y1, int x2, int y2, int x3, int y3);
	void Draw_BottomTri(int x1, int y1, int x2, int y2, int x3, int y3);
	void Draw_TopTri(int x1, int y1, int x2, int y2, int x3, int y3);

	void Draw_TopTri(int x1, int y1, int x2, int y2, int x3, int y3,int u0,int v0,int u1,int v1,int u2,int v2);
	void Draw_FillTri(int x1, int y1, int x2, int y2, int x3, int y3);
	void Draw_FillTri(int x1, int y1,float z1, int x2, int y2,float z2, int x3, int y3,float z3,float u1,float v1,float u2,float v2,float u3,float v3,UINT* img,Light *light,const class Cmaera  &camera,Vector4d norml);
	void Draw_UV(float u1, float v1, float u2, float v2, float u3, float v3, float& u, float& v);
	bool Draw_UV(int x1,int y1,int x2,int y2,int x3,int y3,int x,int y,float& u,float& v);
	void Load2D();
	BYTE* GetImageData(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture, /*OUT*/ int* nWidth, /*OUT*/ int*  nHeight);
	~Graphics();
	UINT img[256 * 256];
private:
	Microsoft::WRL::ComPtr<IDXGISwapChain>				pSwapChain;
	Microsoft::WRL::ComPtr<ID3D11Device>				pDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>			pImmediateContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		pRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				pSysBufferTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	pSysBufferTextureView;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>			pInputLayout;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			pSamplerState;
	ID3D11Texture2D				                        *pTexture2D ;
	D3D11_MAPPED_SUBRESOURCE							mappedSysBufferTexture;
	Color*                                              pSysBuffer = nullptr;
	UINT* Texture2D;
public:
	static constexpr int ScreenWidth = 1280;
	static constexpr int ScreenHeight = 720;
};