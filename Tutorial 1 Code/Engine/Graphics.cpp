/******************************************************************************************
*	Chili DirectX Framework Version 16.07.20											  *
*	Graphics.cpp																		  *
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
#include "Graphics.h"
#include "DXErr.h"
#include "ChiliException.h"
#include <assert.h>
#include <array>
#include "Xmath.h"
#include <D3D11.h>
#include <D3DX11tex.h>


using namespace std;
using namespace XMath;

// Ignore the intellisense error "cannot open source file" for .shh files.
// They will be created during the build sequence before the preprocessor runs.
namespace FramebufferShaders
{
#include "FramebufferPS.shh"
#include "FramebufferVS.shh"
}

#pragma comment( lib,"d3d11.lib" )

#define CHILI_GFX_EXCEPTION( hr,note ) Graphics::Exception( hr,note,_CRT_WIDE(__FILE__),__LINE__ )

using Microsoft::WRL::ComPtr;

Graphics::Graphics( HWNDKey& key )
{
	assert( key.hWnd != nullptr );

	//////////////////////////////////////////////////////
	// create device and swap chain/get render target view
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferCount = 1;
	sd.BufferDesc.Width = Graphics::ScreenWidth;
	sd.BufferDesc.Height = Graphics::ScreenHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 1;
	sd.BufferDesc.RefreshRate.Denominator = 60;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = key.hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	HRESULT				hr;
	UINT				createFlags = 0u;
#ifdef CHILI_USE_D3D_DEBUG_LAYER
#ifdef _DEBUG
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
#endif
	
	// create device and front/back buffers
	if( FAILED( hr = D3D11CreateDeviceAndSwapChain( 
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwapChain,
		&pDevice,
		nullptr,
		&pImmediateContext ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Creating device and swap chain" );
	}

	// get handle to backbuffer
	ComPtr<ID3D11Resource> pBackBuffer;
	if( FAILED( hr = pSwapChain->GetBuffer(
		0,
		__uuidof( ID3D11Texture2D ),
		(LPVOID*)&pBackBuffer ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Getting back buffer" );
	}

	// create a view on backbuffer that we can render to
	if( FAILED( hr = pDevice->CreateRenderTargetView( 
		pBackBuffer.Get(),
		nullptr,
		&pRenderTargetView ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Creating render target view on backbuffer" );
	}


	// set backbuffer as the render target using created view
	pImmediateContext->OMSetRenderTargets( 1,pRenderTargetView.GetAddressOf(),nullptr );


	// set viewport dimensions
	D3D11_VIEWPORT vp;
	vp.Width = float( Graphics::ScreenWidth );
	vp.Height = float( Graphics::ScreenHeight );
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	pImmediateContext->RSSetViewports( 1,&vp );


	///////////////////////////////////////
	// create texture for cpu render target
	D3D11_TEXTURE2D_DESC sysTexDesc;
	sysTexDesc.Width = Graphics::ScreenWidth;
	sysTexDesc.Height = Graphics::ScreenHeight;
	sysTexDesc.MipLevels = 1;
	sysTexDesc.ArraySize = 1;
	sysTexDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sysTexDesc.SampleDesc.Count = 1;
	sysTexDesc.SampleDesc.Quality = 0;
	sysTexDesc.Usage = D3D11_USAGE_DYNAMIC;
	sysTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	sysTexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	sysTexDesc.MiscFlags = 0;
	// create the texture
	if( FAILED( hr = pDevice->CreateTexture2D( &sysTexDesc,nullptr,&pSysBufferTexture ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Creating sysbuffer texture" );
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = sysTexDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	// create the resource view on the texture
	if( FAILED( hr = pDevice->CreateShaderResourceView( pSysBufferTexture.Get(),
		&srvDesc,&pSysBufferTextureView ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Creating view on sysBuffer texture" );
	}


	////////////////////////////////////////////////
	// create pixel shader for framebuffer
	// Ignore the intellisense error "namespace has no member"
	if( FAILED( hr = pDevice->CreatePixelShader(
		FramebufferShaders::FramebufferPSBytecode,
		sizeof( FramebufferShaders::FramebufferPSBytecode ),
		nullptr,
		&pPixelShader ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Creating pixel shader" );
	}
	

	/////////////////////////////////////////////////
	// create vertex shader for framebuffer
	// Ignore the intellisense error "namespace has no member"
	if( FAILED( hr = pDevice->CreateVertexShader(
		FramebufferShaders::FramebufferVSBytecode,
		sizeof( FramebufferShaders::FramebufferVSBytecode ),
		nullptr,
		&pVertexShader ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Creating vertex shader" );
	}
	

	//////////////////////////////////////////////////////////////
	// create and fill vertex buffer with quad for rendering frame
	const FSQVertex vertices[] =
	{
		{ -1.0f,1.0f,0.5f,0.0f,0.0f },
		{ 1.0f,1.0f,0.5f,1.0f,0.0f },
		{ 1.0f,-1.0f,0.5f,1.0f,1.0f },
		{ -1.0f,1.0f,0.5f,0.0f,0.0f },
		{ 1.0f,-1.0f,0.5f,1.0f,1.0f },
		{ -1.0f,-1.0f,0.5f,0.0f,1.0f },
	};
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( FSQVertex ) * 6;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0u;
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;
	if( FAILED( hr = pDevice->CreateBuffer( &bd,&initData,&pVertexBuffer ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Creating vertex buffer" );
	}

	
	//////////////////////////////////////////
	// create input layout for fullscreen quad
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};

	// Ignore the intellisense error "namespace has no member"
	if( FAILED( hr = pDevice->CreateInputLayout( ied,2,
		FramebufferShaders::FramebufferVSBytecode,
		sizeof( FramebufferShaders::FramebufferVSBytecode ),
		&pInputLayout ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Creating input layout" );
	}


	////////////////////////////////////////////////////
	// Create sampler state for fullscreen textured quad
	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	if( FAILED( hr = pDevice->CreateSamplerState( &sampDesc,&pSamplerState ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Creating sampler state" );
	}

	// allocate memory for sysbuffer (16-byte aligned for faster access)
	pSysBuffer = reinterpret_cast<Color*>( 
		_aligned_malloc( sizeof( Color ) * Graphics::ScreenWidth * Graphics::ScreenHeight,16u ) );
}

void Graphics::Load2D()
{
	int width = 256;
	int height = 256;

	D3DX11_IMAGE_LOAD_INFO    ImgLoadInfo;
	ImgLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	ImgLoadInfo.CpuAccessFlags = 0;
	ImgLoadInfo.Depth = 1;
	ImgLoadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ImgLoadInfo.Height = 256;	// 图像高度
	ImgLoadInfo.MipLevels = 1;
	ImgLoadInfo.Width = 256;	// 图像宽度
	ImgLoadInfo.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D	 *pTexture2D = NULL;


	//D3DX11CreateShaderResourceViewFromFileW(pDevice.Get(), L"E://timg.jpg", NULL, NULL, &mTexture, NULL)
	 //从图像读取纹理
	D3DX11CreateTextureFromFile(
		pDevice.Get(),		// D3D设备
		L"E://timg.jpg",		// 图像文件名
		&ImgLoadInfo,		// 图像信息
		NULL,
		(ID3D11Resource **)&pTexture2D,	// 输出：Texture2D图像纹理
		NULL);
	BYTE* img = GetImageData(pDevice.Get(), pImmediateContext.Get(), pTexture2D, &width, &height);
	
	for (int h = 0; h < 256; h++)
	{
		for (int w = 0; w < 256; w++)
		{
			UINT r = img[w*4 + 256 * h * 4];
			UINT  g = img[w*4 + 256 * h * 4+1];
			UINT b = img[w*4 + 256 * h * 4+2];
			UINT  a = img[w*4 + 256 * h * 4+3];
			/*UINT Color= UINT((a << 24) | (r << 16) | (g << 8) | b);*/
			UINT Color = UINT((a << 24) | (r << 16) | (g << 8) | b);
			this->img[w + 256 * h] = Color;
		}
	}
	int i = 0;
}

BYTE * Graphics::GetImageData(ID3D11Device * device, ID3D11DeviceContext * context, ID3D11Texture2D * texture, int * nWidth, int * nHeight)
{
	if (texture) {
		D3D11_TEXTURE2D_DESC description;
		texture->GetDesc(&description);
		description.BindFlags = 0;
		description.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		description.Usage = D3D11_USAGE_STAGING;

		ID3D11Texture2D* texTemp = NULL;

		HRESULT hr = device->CreateTexture2D(&description, NULL, &texTemp);
		if (FAILED(hr))
		{
			if (hr == E_OUTOFMEMORY) {
				printf("GetImageData - CreateTexture2D - OUT OF MEMORY \n");
			}
			if (texTemp)
			{
				texTemp->Release();
				texTemp = NULL;
			}
			return NULL;
		}
		context->CopyResource(texTemp, texture);

		D3D11_MAPPED_SUBRESOURCE  mapped;
		unsigned int subresource = 0;
		hr = context->Map(texTemp, 0, D3D11_MAP_READ, 0, &mapped);
		if (FAILED(hr))
		{
			printf("GetImageData - Map - FAILED \n");
			texTemp->Release();
			texTemp = NULL;
			return NULL;
		}

		*nWidth = description.Width;
		*nHeight = description.Height;
		const int pitch = mapped.RowPitch;
		BYTE* source = (BYTE*)(mapped.pData);
		BYTE* dest = new BYTE[(*nWidth)*(*nHeight) * 4];
		BYTE* destTemp = dest;
		for (int i = 0; i < *nHeight; ++i)
		{
			memcpy(destTemp, source, *nWidth * 4);
			source += pitch;
			destTemp += *nWidth * 4;
		}
		context->Unmap(texTemp, 0);
		//delete texTemp;
		return dest;
	}
	else {
		printf("GetImageData - texture null - FAILED \n");
		return NULL;
	}
}

Graphics::~Graphics()
{
	// free sysbuffer memory (aligned free)
	if( pSysBuffer )
	{
		_aligned_free( pSysBuffer );
		pSysBuffer = nullptr;
	}
	// clear the state of the device context before destruction
	if( pImmediateContext ) pImmediateContext->ClearState();
}

void Graphics::EndFrame()
{
	HRESULT hr;

	// lock and map the adapter memory for copying over the sysbuffer
	if( FAILED( hr = pImmediateContext->Map( pSysBufferTexture.Get(),0u,
		D3D11_MAP_WRITE_DISCARD,0u,&mappedSysBufferTexture ) ) )
	{
		throw CHILI_GFX_EXCEPTION( hr,L"Mapping sysbuffer" );
	}
	// setup parameters for copy operation
	Color* pDst = reinterpret_cast<Color*>(mappedSysBufferTexture.pData );
	const size_t dstPitch = mappedSysBufferTexture.RowPitch / sizeof( Color );
	const size_t srcPitch = Graphics::ScreenWidth;
	const size_t rowBytes = srcPitch * sizeof( Color );
	// perform the copy line-by-line
	for( size_t y = 0u; y < Graphics::ScreenHeight; y++ )
	{
		memcpy( &pDst[ y * dstPitch ],&pSysBuffer[y * srcPitch],rowBytes );
	}
	// release the adapter memory
	pImmediateContext->Unmap( pSysBufferTexture.Get(),0u );

	// render offscreen scene texture to back buffer
	pImmediateContext->IASetInputLayout( pInputLayout.Get() );
	pImmediateContext->VSSetShader( pVertexShader.Get(),nullptr,0u );
	pImmediateContext->PSSetShader( pPixelShader.Get(),nullptr,0u );
	pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	const UINT stride = sizeof( FSQVertex );
	const UINT offset = 0u;
	pImmediateContext->IASetVertexBuffers( 0u,1u,pVertexBuffer.GetAddressOf(),&stride,&offset );
	pImmediateContext->PSSetShaderResources( 0u,1u,pSysBufferTextureView.GetAddressOf() );
	pImmediateContext->PSSetSamplers( 0u,1u,pSamplerState.GetAddressOf() );
	pImmediateContext->Draw( 6u,0u );

	// flip back/front buffers
	if( FAILED( hr = pSwapChain->Present( 1u,0u ) ) )
	{
		if( hr == DXGI_ERROR_DEVICE_REMOVED )
		{
			throw CHILI_GFX_EXCEPTION( pDevice->GetDeviceRemovedReason(),L"Presenting back buffer [device removed]" );
		}
		else
		{
			throw CHILI_GFX_EXCEPTION( hr,L"Presenting back buffer" );
		}
	}
}

void Graphics::BeginFrame()
{
	// clear the sysbuffer
	memset( pSysBuffer,0u,sizeof( Color ) * Graphics::ScreenHeight * Graphics::ScreenWidth );
}

inline void swap_int(int *a, int *b) {
	*a ^= *b;
	*b ^= *a;
	*a ^= *b;
}



void Graphics::Draw_Circle(int r,int x1,int y1)
{

	int x = 0;
	int y = r;
	float d = 2 - 3 * r;

	for (; x <= y; x++)
	{
		PutPixel(x1+x,y1+y, Color(1111111));
		PutPixel(x1 +y, y1 + x, Color(1111111));
		PutPixel(x1-x,y1+y, Color(1111111));
		PutPixel(x1 - y, y1 + x, Color(1111111));
		PutPixel(x1 + x, y1 - y, Color(1111111));
		PutPixel(x1 +y, y1 - x, Color(1111111));
		PutPixel(x1 - y, y1 - x, Color(1111111));
		PutPixel(x1 - x, y1 - y, Color(1111111));
		if (d > 0)
		{
			y--;
			d += 4 * (x - y) + 10;
		}
		else
		{
			d += 4 * x + 6;
			
		}


	}

}


inline void swap(float& x1, float& y1, float& x2, float& y2)
{
	float tempx = x1;
	x1 = x2;
	x2 = tempx;

	float tempy = y1;
	y1 = y2;
	y2 = tempy;
	
}

inline void swap(int& x1, int& y1, int& x2, int& y2)
{
	float tempx = x1;
	x1 = x2;
	x2 = tempx;

	float tempy = y1;
	y1 = y2;
	y2 = tempy;

}

void Graphics::Draw_tr(int x1, int y1, int x2, int y2, int x3, int y3)
{
	if (y1 > y2)
	{
		swap(x1, y1, x2, y2);
	}
	if (y1 > y3)
	{
		swap(x1, y1, x3, y3);
	}
	if (y2 > y3)
	{
		swap(x2, y2, x3, y3);
	}

	if (y1 == y2)
	{
		Draw_TopTri(x1, y1, x2, y2, x3, y3);
	}
	else if (y2 == y3)
	{
		Draw_BottomTri(x1, y1, x2, y2, x3, y3);
	}
	else
	{
		
			float k = (float)(x3 - x1) / (float)(y3 - y1);
			int x4 = x1 + (y2 - y1)*k+0.5f;
			Draw_BottomTri(x1, y1, x2, y2, x4, y2);
			Draw_TopTri(x4, y2, x2, y2, x3, y3);

		

		
	}


	

}

void Graphics::Draw_BottomTri(int x1, int y1, int x2, int y2, int x3, int y3)
{
	float lk = 0.f, rk = 0.f;
	if (x3 < x2)
	{
		swap(x2, y2, x3, y3);
	}

	lk = float(x2 - x1) / float(y2 - y1);
	
	
	
	
	rk = float(x3 - x1) / float(y3 - y1);

	

	float lx = x1;
	float rx = x1;
	float ly = y1;
	float ry = y1;



	for (int i = y1; i <= y3; i++)
	{
		lx = (i - y1)*lk + x1 + 0.5;
		rx = (i - y1)*rk + x1 + 0.5;
		Drawline(lx, i, rx, i);

		
	}


}

void Graphics::Draw_TopTri(int x1, int y1, int x2, int y2, int x3, int y3)
{
	

	if (x2 < x1)
	{
		swap(x1, y1, x2, y2);
	}

	float lk = float(x3 - x1) / float(y3 - y1);
	float rk = float(x3 - x2) / float(y3 - y2);

	



	float lx = x1;
	float rx = x2;
	float ly = y1;
	float ry = y2;



	for (int i = y1; i <y3; i++)
	{
		lx = (i - y1)*lk + x1 + 0.5;
		rx = (i - y1)*rk + x2 + 0.5;
		Drawline(lx, i, rx, i);
		
	}
}

void Graphics::Draw_TopTri(int x1, int y1, int x2, int y2, int x3, int y3, int u1, int v1, int u2, int v2, int u3, int v3)
{
	if (x2 < x1)
	{
		swap(x1, y1, x2, y2);
	}

	float lk = float(x3 - x1) / float(y3 - y1);
	float rk = float(x3 - x2) / float(y3 - y2);

	float lu = float(u3 - u1) / float(v3 - v1);
	float ru = float(u3 - u2) / float(v3 - v2);

	float lv= float(v3 - v1) / float(y3 - y1);
	float rv = float(v3 - v2) / float(y3 - y2);

	float lx = x1;
	float rx = x2;
	float ly = y1;
	float ry = y2;

	float v1desc = v1;
	float v2desc = v2;

	float ludesc = u1;
	float rudesc = u2;

	for (int i = y1 + 1; i < y3; i++)
	{

		Drawline(lx, i, rx, i);
		lx = x1 + lk * (i - y1);
		rx = x2 + rk * (i - y2);
		v1desc += lv;
		v2desc += rv;
		ludesc += lu;
		rudesc += ru;

	}



}

void Graphics::Draw_FillTri(int x1, int y1, int x2, int y2, int x3, int y3)
{
	if (y1 > y2)
	{
		swap(x1, y1, x2, y2);
	}
	if (y1 > y3)
	{
		swap(x1, y1, x3, y3);
	}
	if (y2 > y3)
	{
		swap(x2, y2, x3, y3);
	}

	

	int vx1 = x2 - x1;
	int vy1 = y2 - y1;
	int vx2 = x3 - x1;
	int vy2 = y3 - y1;
	int vx3 = x2 - x3;
	int vy3 = y2 - y3;

	float k1 = (y2 - y1)*x3 + (x1 - x2)*y3 + x2 * y1 - x1 * y2;
	float k2 = (y3 - y1)*x2 + (x1 - x3)*y2 + x3 * y1 - x1 * y3;
	float k3 = (y3 - y2)*x1 + (x3 - x2)*y1 + x3 * y2 - x2 * y2;

	k1 = 1 / k1;
	k2 = 1 / k2;
	k3 = 1 / k3;
	int minx =Min(x1, Min(x2, x3));
	int maxx = Max(x1, Max(x2, x3));
	int miny = Min(y1, Min(y2, y3));
	int maxy = Max(y1, Max(y2, y3));

	


	for (int h = miny; h < maxy; h++)
	{
		for (int w = minx; w < maxx; w++)
		{
			int vx0 = w - x1;
			int vy0 = h - y1;
			/*if (insideangle(vx1, vy1, vx2, vy2, vx3, vy3, vx0, vy0))
			{
				PutPixel(w, h, Color(1111111100));
			}
*/			
			float u = ((y2 - y1)*w + (x1 - x2)*h + x2 * y1 - x1 * y2)*k1;
			float v= ((y3 - y1)*w + (x1 - x3)*h + x3 * y1 - x1 * y3)*k2;
			float c = 1 - u - v;
			if (u >= 0 && v >= 0 && c >= 0)
			{
				PutPixel(w, h, Color(1111111100));
			}


		}
	}






}

void Graphics::Draw_FillTri(int x1, int y1, int x2, int y2, int x3, int y3, float u1, float v1, float u2, float v2, float u3, float v3, UINT* img)
{



	int vx1 = x2 - x1;
	int vy1 = y2 - y1;
	int vx2 = x3 - x1;
	int vy2 = y3 - y1;
	int vx3 = x2 - x3;
	int vy3 = y2 - y3;

	float k1 = (y2 - y1)*x3 + (x1 - x2)*y3 + x2 * y1 - x1 * y2;
	float k2 = (y3 - y1)*x2 + (x1 - x3)*y2 + x3 * y1 - x1 * y3;
	float k3 = (y3 - y2)*x1 + (x3 - x2)*y1 + x3 * y2 - x2 * y2;

	k1 = 1 / k1;
	k2 = 1 / k2;
	k3 = 1 / k3;
	int minx = Min(x1, Min(x2, x3));
	int maxx = Max(x1, Max(x2, x3));
	int miny = Min(y1, Min(y2, y3));
	int maxy = Max(y1, Max(y2, y3));




	for (int h = miny; h < maxy; h++)
	{
		for (int w = minx; w < maxx; w++)
		{
			int vx0 = w - x1;
			int vy0 = h - y1;
			/*if (insideangle(vx1, vy1, vx2, vy2, vx3, vy3, vx0, vy0))
			{
				PutPixel(w, h, Color(1111111100));
			}
*/
			float u = ((y2 - y1)*w + (x1 - x2)*h + x2 * y1 - x1 * y2)*k1;
			float v = ((y3 - y1)*w + (x1 - x3)*h + x3 * y1 - x1 * y3)*k2;
			float c = 1 - u - v;
			if (u >= 0 && v >= 0 && c >= 0)
			{
				Draw_UV(x1, y1, x2, y2, x3, y3, w, h, u, v);
					float c = 1 - u - v;
					float ux1 = u2 - u1;
					float uy1 = v2 - v1;
					float ux2 = u3 - u1;
					float uy2 = v3 - v1;
					float ux3 =u3 - u2;
					float uy3 = v3 - v3;
					float ux0 =u1+u*ux1+v*ux2;
					float uy0 = v1+u*uy1+v*uy2;
				/*	float ux0 = c * u1 + u * u2 + v * u3;
					float uy0 = c * v1 + u * v2 + v3 * v;*/
					float width = 256.f * ux0;
					float height = 256.f* uy0;
					int de = width +256 *height;

					UINT color = img[de];
					PutPixel(w, h, Color(color));
				

			}


		}
	}
}

void Graphics::Draw_UV(float u1, float v1, float u2, float v2, float u3, float v3, float & u, float & v)
{

}



bool Graphics::Draw_UV(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y, float& u, float& v)
{
	Vector2d v1(x2 - x1, y2 - y1);
	Vector2d v2(x3 - x1, y3 - y1);
	Vector2d v3(x3 - x2, y3 - y2);
	Vector2d v0(x - x1, y - y1);
	float s = CrossProductF(v1.u,v1.v,v2.u,v2.v);
	float s1= CrossProductF(v1.u, v1.v,v0.u, v0.v);
	float s2 = CrossProductF(v0.u, v0.v,v2.u, v2.v);
	float s3 = CrossProductF(v1.u, v1.v, v2.u, v2.v);
	
	u= s1 / s;
	v =s2 /s;

	return u >= 0 && v >= 0 && u + v <= 1;
}

void Graphics::Drawline(int x1, int y1, int x2, int y2)
{

	int addx = 1;
	int addy = -1;

	if (x1 > x2)
	{
		addx = -1;
	}

	if (y2>y1 )
	{
		addy = 1;
	}
	
	
	int dy =abs( y2 - y1); 
	int dx =abs(x2 - x1);
	
	int intechabge = 0;
	if (dy > dx)
	{
		int temp = dx;
		dx = dy;
		dy = temp;
		intechabge = 1;
	}
	else {
		intechabge = 0;
	}
	int x = x1, y = y1;
	float d = 2 * dy - dx;

	for (int i = 0; i <dx; i++)
	{
		PutPixel(x, y,Color(1111111));
		if (d > 0)
		{
			if (intechabge == 0)
			{
				y+=addy;
			}
			else
			{
				x+=addx;
			}
			d = d + 2*(dy - dx);
		}
		else
		{
		
			d = d + 2*dy;
		}
		if (intechabge == 0)
		{
			x+=addx;
		}
		else
		{
			y+=addy;
		}
	}
}

void Graphics::PutPixel( int x,int y,Color c )
{
	assert( x >= 0 );
	assert( x < int( Graphics::ScreenWidth ) );
	assert( y >= 0 );
	assert( y < int( Graphics::ScreenHeight ) );
	pSysBuffer[Graphics::ScreenWidth * y + x] = c;
}

void Graphics::DrawTri(int x1, int y1, int x2, int y2, int x3, int y3)
{
	Drawline(x1, y1, x2, y2);
	Drawline(x1, y1, x3, y3);
	Drawline(x2, y2, x3, y3);


}


//////////////////////////////////////////////////
//           Graphics Exception
Graphics::Exception::Exception( HRESULT hr,const std::wstring& note,const wchar_t* file,unsigned int line )
	:
	ChiliException( file,line,note ),
	hr( hr )
{}

std::wstring Graphics::Exception::GetFullMessage() const
{
	const std::wstring empty = L"";
	const std::wstring errorName = GetErrorName();
	const std::wstring errorDesc = GetErrorDescription();
	const std::wstring& note = GetNote();
	const std::wstring location = GetLocation();
	return    (!errorName.empty() ? std::wstring( L"Error: " ) + errorName + L"\n"
		: empty)
		+ (!errorDesc.empty() ? std::wstring( L"Description: " ) + errorDesc + L"\n"
			: empty)
		+ (!note.empty() ? std::wstring( L"Note: " ) + note + L"\n"
			: empty)
		+ (!location.empty() ? std::wstring( L"Location: " ) + location
			: empty);
}

std::wstring Graphics::Exception::GetErrorName() const
{
	return DXGetErrorString( hr );
}

std::wstring Graphics::Exception::GetErrorDescription() const
{
	std::array<wchar_t,512> wideDescription;
	DXGetErrorDescription( hr,wideDescription.data(),wideDescription.size() );
	return wideDescription.data();
}

std::wstring Graphics::Exception::GetExceptionType() const
{
	return L"Chili Graphics Exception";
}