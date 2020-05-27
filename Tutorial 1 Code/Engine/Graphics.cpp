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
#include "Cmaera.h"
#include "D3D11.h"
#include "D3DX11tex.h"
#include "PostProcess.h"
#include "DrawX.h"

using namespace std;
using namespace XMath;

// Ignore the intellisense error "cannot open source file" for .shh files.
// They will be created during the build sequence before the preprocessor runs.
namespace FramebufferShaders
{
#include "FramebufferPS.shh"
#include "FramebufferVS.shh"
#include "PostProcess.h"
#include "PostProcess.h"
#include "DrawX.h"
#include "DrawX.h"
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

	prevpSysBuffer = reinterpret_cast<Color*>(
		_aligned_malloc(sizeof(Color) * Graphics::ScreenWidth * Graphics::ScreenHeight, 16u));

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
	if (prevpSysBuffer)
	{
		_aligned_free(pSysBuffer);
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


inline void swap(float& x1, float& y1, float& x2, float& y2, float& z1,float &z2)
{
	float tempx = x1;
	x1 = x2;
	x2 = tempx;

	float tempy = y1;
	y1 = y2;
	y2 = tempy;

	float tempz = z1;
	z1 = z2;
	z2 = tempz;

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

void Graphics::Bresenhamline(int x1, int y1, int x2, int y2)
{
	int x, y, dx, dy, s1, s2, p, temp, interchange, i;
	x = x1;
	y = y1;
	dx = abs(x2 - x1);
	dy = abs(y2 - y1);

	if (x2 > x1)
		s1 = 1;
	else
		s1 = -1;

	if (y2 > y1)
		s2 = 1;
	else
		s2 = -1;

	if (dy > dx)
	{
		temp = dx;
		dx = dy;
		dy = temp;
		interchange = 1;
	}
	else
		interchange = 0;

	p = 2 * dy - dx;
	for (i = 1; i <= dx; i++)
	{
		PutPixel(x, y, Color(1111111000));
		if (p >= 0)
		{
			if (interchange== 0)
				y = y + s2;
			else
				x = x + s1;
			p = p - 2 * dx;
		}
		if (interchange == 0)
			x = x + s1;
		else
			y = y + s2;
		p = p + 2 * dy;
	}
}


float Graphics::MsAAWeight(float width, float height,float x1, float y1, float x2, float y2, float x3, float y3)
{
	static const float SamplesX[] = { -0.5,  0.5,  -0.5, 0.5 };
	 static const float SamplesY[] = { -0.5,  0.5,  1.0 / 2.0f, -0.5 };

	float w1 = width + SamplesX[0];
	float h1 = height + SamplesY[0];

	float w2 = width + SamplesX[1];
	float h2 = height + SamplesY[1];

	float w3 = width + SamplesX[2];
	float h3 = height + SamplesY[2];

	float w4 = width + SamplesX[3];
	float h4 = height + SamplesY[3];

	float total = 4.0f;
	float weight = 0.0f;
	float u = 0.0f;
	float v = 0.0f;
	if (Draw_UV(x1, y1, x2, y2, x3, y3, w1, h1, u, v))
	{
		++weight;
	}


	if (Draw_UV(x1, y1, x2, y2, x3, y3, w2, h2, u, v))
	{


		++weight;
	}


	if (Draw_UV(x1, y1, x2, y2, x3, y3, w3, h3, u, v))
	{


		++weight;

	}


	if (Draw_UV(x1, y1, x2, y2, x3, y3, w4, h4, u, v))
	{


		++weight;

	}

	float we = (float)weight / (float)total;
	return we;

}


void Graphics::Draw_FillTri(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, Cmaera &camera)
{
	/*if (y1 > y2)
	{
		swap(x1, y1, x2, y2,z1,z2);
	}
	
	if (y1 > y3)
	{
		swap(x1, y1, x3, y3,z1,z3);
	}
	if (y2 > y3)
	{
		swap(x2, y2, x3, y3,z2,z3);
	}
	if (y1 == y2)
	{
		if (x1 > x2)
		{
			swap(x1, y1, x2, y2, z1, z2);
		}
	}+
	if (y2 == y3)
	{
		if (x2 > x3)
		{
			swap(x2, y2, x3, y3, z2, z3);
		}
	}*/
		


	int minx =Min(round(x1), Min(round(x2), round(x3)));
	int maxx = Max(ceil(x1), Max(ceil(x2), ceil(x3)));
	int miny = Min(round(y1), Min(round(y2), round(y3)));
	int maxy = Max(ceil(y1), Max(ceil(y2), ceil(y3)));

	Vector4d v1(x1, y1, z1, 1.0f);

	V4d_Mul_4X4(v1, camera.ViewMATRIX.inversemscr, v1);
	V4d_Mul_4X4(v1, camera.ViewMATRIX.inversemper, v1);
	V4d_Mul_4X4(v1, camera.ViewMATRIX.inversetrans, v1);
	V4d_Mul_4X4(v1, camera.ViewMATRIX.inverseuvn, v1);
	v1.x = v1.x / v1.w;
	v1.y = v1.y / v1.w;
	v1.z = v1.z / v1.w;
	v1.w = v1.w / v1.w;
	V4d_Mul_4X4(v1, camera.PrevViewMATRIX.trans, v1);
	V4d_Mul_4X4(v1, camera.PrevViewMATRIX.uvn, v1);
	V4d_Mul_4X4(v1, camera.PrevViewMATRIX.mper, v1);
	v1.x = v1.x / v1.w;
	v1.y = v1.y / v1.w;
	v1.z = v1.z / v1.w;
	v1.w = v1.w / v1.w;
	V4d_Mul_4X4(v1, camera.PrevViewMATRIX.mscr, v1);
	v1.x = (v1.x - camera.PrevJitterX*0.5);
	v1.y = (v1.y - camera.PrevJitterY*-0.5);

	float xp1 = x1 - camera.JitterX*0.5;
	float xy1 = y1 - camera.JitterY*-0.5;

	float mox = (v1.x - xp1);
	float moy = (v1.y - xy1);


	for (int h = miny; h <= maxy; h++)
	{
		for (int w = minx; w <= maxx; w++)
		{
			int vx0 = w - x1;
			int vy0 = h - y1;
			/*if (insideangle(vx1, vy1, vx2, vy2, vx3, vy3, vx0, vy0))
			{
				PutPixel(w, h, Color(1111111100));
			}
*/			
			float u = 0.0f;
			float v = 0.0f;
			float c= 0.0f;

			float width = (float)w;
			float height = (float)h;



			if (Draw_UV(x1, y1, x2, y2, x3, y3, width, height, u, v))
			{

				
				int colo =Colors::Blue.dword* (1 - u - v)+ Colors::Red.dword*v+ Colors::Blue.dword*u;
				float y = y1 *v  + y2 * (1 - u - v) + y3 *u;
				float x = x1 * v+(1 - u - v)*y2 + x2 * v + x3 * u;
				float depth=1.0f/(1.0f/z1 * v + 1.0f/z2 * (1 - u - v) +1.0f/z3 * u);
				//float depth= 1.0f/((1.0f/z1)*x1* (1-u-v) + (1.0f/z2)*x2* v + (1.0f/z3)*x3 * u);
				
				//Color color = Colors::Green.dword*(1 - u - v) + Colors::Blue.dword*v + Colors::Red.dword*u;
				if (depth > 0.0f&&depth < 1.0f)
				{
					
				
					if (depth < PostProcess::GetApplcation()->GetDepth(w,h)|| PostProcess::GetApplcation()->GetDepth(w, h)<0.0f)
					{
						unsigned int red =  0* v + 255 * (1 - u - v) + 0 * u;
						unsigned int green = 0 *  v + 0 * (1 - u - v) + 255 * u;
						unsigned int blue = 255 * v + 0 * (1 - u - v) + 0 * u;
						PostProcess::GetApplcation()->SetDepth(w, h, depth);
						PutPixel(w, h, Color(red, green,blue));

						Vector4d v1(w, h, depth, 1.0f);
						V4d_Mul_4X4(v1,camera.ViewMATRIX.inversemscr, v1);
						V4d_Mul_4X4(v1, camera.ViewMATRIX.inversemper, v1);
						V4d_Mul_4X4(v1, camera.ViewMATRIX.inversetrans, v1);
						V4d_Mul_4X4(v1, camera.ViewMATRIX.inverseuvn, v1);
						v1.x = v1.x / v1.w;
						v1.y = v1.y / v1.w;
						v1.z = v1.z / v1.w;
						v1.w = v1.w / v1.w;
						V4d_Mul_4X4(v1, camera.PrevViewMATRIX.trans, v1);
						V4d_Mul_4X4(v1, camera.PrevViewMATRIX.uvn, v1);
						V4d_Mul_4X4(v1, camera.PrevViewMATRIX.mper, v1);
						v1.x = v1.x / v1.w;
						v1.y = v1.y / v1.w;
						v1.z = v1.z / v1.w;
						v1.w = v1.w / v1.w;
						V4d_Mul_4X4(v1, camera.PrevViewMATRIX.mscr, v1);

						float motionu = (v1.x- camera.PrevJitterX*0.5)-(w- camera.JitterX*0.5);
						float motionv = (v1.y - camera.PrevJitterY*-0.5) - (h - camera.JitterY*-0.5);
						MotionVector mo;
						mo.MotingX = motionu;
						mo.MotingY = motionv;
						PostProcess::GetApplcation()->Setmotionvector(w,h,mo);
					}
				}

				{
					/*float weight = MsAAWeight(width, height, x1, y1, x2, y2, x3, y3);
					unsigned int red = 0 * v + 255 * (1 - u - v) + 0 * u;
					unsigned int green = 0 * v + 0 * (1 - u - v) + 255 * u;
					unsigned int blue = 255 * v + 0 * (1 - u - v) + 0 * u;
					PutPixel(w, h, Color(red*weight, green*weight, blue*weight));*/
				}
			}
		}
		int n = 0;
	}

}










void Graphics::Draw_FillTri(int x1, int y1, float z1, int x2, int y2, float z2, int x3, int y3, float z3, float u1, float v1, float u2, float v2, float u3, float v3, UINT* img)
{

	/*if (x1 > x2)
	{
		swap(x1, x2);
		swap(y1,y2);
		swap(z1, z2);
		swap(u1, u2);
	}
	if (x1 > x3)
	{
		swap(x1, x3);
		swap(y1, y3);
		swap(z1, z3);
		swap(u1, u3);
	}
	if (x2 > x3)
	{
		swap(x2, x3);
		swap(y2, y3);
		swap(z2, z3);
		swap(u2, u3);
	}*/

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
	/*float  zmin= Min(z1, Min(z2, z3));
	float zmax = Max(z1, Max(z2, z3));
	float umin= Min(u1, Min(u2, u3));
	float umax = Max(u1, Max(u2, u3));
	float vmin= Min(v1, Min(v2, v3));
	float vmax = Max(v1, Max(v2, v3));*/

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

				if (h > 210) {
					Draw_UV(x1, y1, x2, y2, x3, y3, w, h, u, v);
					float c = 1.0f - u - v;
					float ux1 = u2 - u1;
					float uy1 = v2 - v1;
					float ux2 = u3 - u1;
					float uy2 = v3 - v1;
					float ux3 = u3 - u2;
					float uy3 = v3 - v3;
					float zx1 = 1 / z2 - 1 / z1;
					float zx2 = 1 / z3 - 1 / z1;
					if (zx1 == 0)
					{
						zx1 = 1;
					}

					if (zx2 == 0)
					{
						zx2 = 1;
					}
					float s = 0.f;
					if (x2 - x1 != 0)
					{
						s = (w - x1) / (float)(x2 - x1);
					}

					float t = 0.f;
					if (x3 - x1 != 0)
					{
						t = (w - x1) / (float)(x3 - x1);

					}

					float zt = 1 / z2 + s * (1 / z2 - 1 / z1);


					/*float ux0 =u1+ux1*u+ux2*v;
					float uy0 =v1+uy1*u+uy2*v;
					*/
					///*float ux0 = (u1/(float)z1 + u * (ux1/(float)zx1) + (ux2/(float)zx2) * v);
					//float uy0 = v1/(float)z1 + (uy1 /(float)zx1)* u + (uy2/(float)zx2) * v;*/
					/*float ux0 = zx0* (c*u1/z1  +   u2*v/z2 + u3 * u/z3);
					float uy0 = zx0*(c*v1/z1  + v2 *  v/z2+ v3 * u/z3);*/


					//float s = (ux0 - u1) / (float)(u2 - u1);

					//float zt = 1 / z1 + s * (1 / z2 - 1 / z1);

					zt = 1 / z1 + (1 / z2 - 1 / z1)* u + (1 / z3 - 1 / z1) * v;

					/*float ux0 =  (u1 *c  + v*u2  + u*u3 );
					float  uy0=  (v1 *c + v *v2 + u*v3);*/

					/*float ux0 =1/zt * (u1 *c/z1  + v*u2 / z2 + u*u3 / z3);
				   float  uy0= 1/zt * (v1 *c/z1 + v *v2/ z2 + u*v3 / z3);*/

					float ux0 = 1 / zt * (u1 / z1 + u * (u2 / z2 - u1 / z1) + v * (u3 / z3 - u1 / z1));
					float  uy0 = 1 / zt * (v1 / z1 + u * (v2 / z2 - v1 / z1) + v * (v3 / z3 - v1 / z1));

					float width = 255.f * ux0;
					float height = 255.f* uy0;



					int hde = ceil(width);
					int lde = (UINT)width;
					if (width - lde >= hde - width)
					{
						width = hde;
					}
					else
					{
						width = lde;
					}

					hde = ceil(height);
					lde = (UINT)height;
					if (height - lde >= hde - height)
					{
						height = hde;
					}
					else
					{
						height = lde;
					}
					int de = 256.f * height + width;

					UINT color = img[de];
					Color col(color);
					Color tol;
					PutPixel(w, h, color);

				}
			}



		}
	}
}


void Graphics::Draw_FillTri(int x1, int y1, float z1, int x2, int y2, float z2, int x3, int y3, float z3, float u1, float v1, float u2, float v2, float u3, float v3, UINT* img, Light *light, const  Cmaera &camera,Vector4d norml)
{

	/*if (x1 > x2)
	{
		swap(x1, x2);
		swap(y1,y2);
		swap(z1, z2);
		swap(u1, u2);
	}
	if (x1 > x3)
	{
		swap(x1, x3);
		swap(y1, y3);
		swap(z1, z3);
		swap(u1, u3);
	}
	if (x2 > x3)
	{
		swap(x2, x3);
		swap(y2, y3);
		swap(z2, z3);
		swap(u2, u3);
	}*/

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
	/*float  zmin= Min(z1, Min(z2, z3));
	float zmax = Max(z1, Max(z2, z3));
	float umin= Min(u1, Min(u2, u3));
	float umax = Max(u1, Max(u2, u3));
	float vmin= Min(v1, Min(v2, v3));
	float vmax = Max(v1, Max(v2, v3));*/

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
			if (u >=0 && v >=0 && c >= 0)
			{

				if (h > 210) {
					Draw_UV(x1, y1, x2, y2, x3, y3, w, h, u, v);
					float c = 1.0f - u - v;
					float ux1 = u2 - u1;
					float uy1 = v2 - v1;
					float ux2 = u3 - u1;
					float uy2 = v3 - v1;
					float ux3 = u3 - u2;
					float uy3 = v3 - v3;
					float zx1 = 1/z2 - 1/z1;
					float zx2 = 1/z3 -1/ z1;
					if (zx1 == 0)
					{
						zx1 = 1;
					}

					if (zx2 == 0)
					{
						zx2 = 1;
					}
					float s = 0.f;
					if (x2 - x1!= 0)
					{
						s = (w - x1) / (float)(x2 - x1);
					}
					
					float t = 0.f;
					if (x3 - x1 != 0)
					{
						t = (w - x1) / (float)(x3 - x1);
						
					}
					
					float zt = 1 / z2 + s*(1 / z2 - 1 / z1);

			
					/*float ux0 =u1+ux1*u+ux2*v;
					float uy0 =v1+uy1*u+uy2*v;
					*/
					///*float ux0 = (u1/(float)z1 + u * (ux1/(float)zx1) + (ux2/(float)zx2) * v);
					//float uy0 = v1/(float)z1 + (uy1 /(float)zx1)* u + (uy2/(float)zx2) * v;*/
					/*float ux0 = zx0* (c*u1/z1  +   u2*v/z2 + u3 * u/z3);
					float uy0 = zx0*(c*v1/z1  + v2 *  v/z2+ v3 * u/z3);*/


					//float s = (ux0 - u1) / (float)(u2 - u1);

					//float zt = 1 / z1 + s * (1 / z2 - 1 / z1);

					zt = 1 / z1  + (1 / z2 -1/z1)* u + (1/z3-1/z1) * v;

					/*float ux0 =  (u1 *c  + v*u2  + u*u3 );
					float  uy0=  (v1 *c + v *v2 + u*v3);*/

					 /*float ux0 =1/zt * (u1 *c/z1  + v*u2 / z2 + u*u3 / z3);
					float  uy0= 1/zt * (v1 *c/z1 + v *v2/ z2 + u*v3 / z3);*/

					float ux0 = 1 / zt * (u1/z1 + u * (u2/z2-u1/z1)+v*(u3/z3-u1/z1));
					float  uy0 = 1 / zt * (v1 / z1 + u * (v2 / z2 - v1 / z1) + v * (v3 / z3 - v1 / z1));

					float width = 255.f * ux0;
					float height = 255.f* uy0;



					int hde = ceil(width);
					int lde = (UINT)width;
					if (width - lde >= hde - width)
					{
						width = hde;
					}
					else
					{
						width = lde;
					}

					hde = ceil(height);
					lde = (UINT)height;
					if (height - lde >= hde - height)
					{
						height = hde;
					}
					else
					{
						height = lde;
					}
					int de = 256.f * height + width;

					UINT color = img[de];
					Color col(color);
					Color tol;

					/*for (int i = 0; i < 3; i++)
					{
						if (light[i].getlightype == Lighttype::Csepc)
						{

							Vector4d target = light[i].getpostion();
							Vector4d v = camera.gettarget();

							NormlVecotr(&target);
							NormlVecotr(&v);

							Vector4d h;
							addVector(target, v, h);

							float length = sqrt(h.x*h.x + h.y*h.y + h.z*h.z);

							h.x = h.x / length;
							h.y = h.y / length;
							h.z = h.z / length;

							float flenght = getlength(target, v);

							float alpha =fmax(DotProduct(&h,&norml),0.f);
							UINT r = light[i].getr()*alpha *col.GetR()*dlength(light[i].getdmax(), light[i].getdmin(), flenght) + tol.GetR();
							UINT g = light[i].getg()*alpha *col.GetG()*dlength(light[i].getdmax(), light[i].getdmin(), flenght) + tol.GetG();
							UINT b = light[i].getb()*alpha *col.GetB() *dlength(light[i].getdmax(), light[i].getdmin(), flenght) + tol.GetB();
							tol.SetR(r);
							tol.SetG(g);
							tol.SetR(b);


						}
						else if (light[i].getlightype == Lighttype::Cdiff)
						{
							Vector4d target = light[i].getpostion();
							Vector4d v = camera.gettarget();
							NormlVecotr(&target);
							NormlVecotr(&v);
							float flenght = getlength(target, v);
							float alpha = fmax(DotProduct(&target, &norml),0.f);
							UINT r =light[i].getr()*alpha *col.GetR()*dlength(light[i].getdmax(), light[i].getdmin(), flenght) + tol.GetR();
							UINT g =light[i].getg()*alpha *col.GetG()*dlength(light[i].getdmax(), light[i].getdmin(), flenght) + tol.GetG();
							UINT b =light[i].getb()*alpha *col.GetB()*dlength(light[i].getdmax(), light[i].getdmin(), flenght) + tol.GetB();
							tol.SetR(r);
							tol.SetG(g);
							tol.SetR(b);
						}
						else
						{
							UINT r = light[i].getr()*col.GetR() + tol.GetR();
							UINT g = light[i].getg()*col.GetG() + tol.GetG();
							UINT b = light[i].getb()*col.GetB() + tol.GetB();
							tol.SetR(r);
							tol.SetG(g);
							tol.SetR(b);

						}

					}*/

					//for (int i = 0; i < 3; i++)
					//{
					//	if (light[i].getlightype == Lighttype::Csepc)
					//	{
					//		
					//		Vector4d target = light[i].getpostion();
					//		Vector4d v = camera.gettarget();

					//		NormlVecotr(&target);
					//		NormlVecotr(&v);

					//		Vector4d h;
					//		addVector(target, v, h);
					//		
					//		float length = sqrt(h.x*h.x + h.y*h.y + h.z*h.z);

					//		h.x = h.x / length;
					//		h.y = h.y / length;
					//		h.z = h.z / length;

					//		float flenght = getlength(target, v);

					//		float alpha =fmax(DotProduct(&h,&norml),0.f);
					//		UINT r = light[i].getr()*alpha *col.GetR()*dlength(light[i].getdmax(), light[i].getdmin(), flenght) + tol.GetR();
					//		UINT g = light[i].getg()*alpha *col.GetG()*dlength(light[i].getdmax(), light[i].getdmin(), flenght) + tol.GetG();
					//		UINT b = light[i].getb()*alpha *col.GetB() *dlength(light[i].getdmax(), light[i].getdmin(), flenght) + tol.GetB();
					//		tol.SetR(r);
					//		tol.SetG(g);
					//		tol.SetR(b);

					//	
					//	}
					//	else if (light[i].getlightype == Lighttype::Cdiff)
					//	{
					//		Vector4d target = light[i].getpostion();
					//		Vector4d v = camera.gettarget();	
					//		NormlVecotr(&target);
					//		NormlVecotr(&v);
					//		float flenght = getlength(target, v);
					//		float alpha = fmax(DotProduct(&target, &norml),0.f);
					//		UINT r =light[i].getr()*alpha *col.GetR()*dlength(light[i].getdmax(), light[i].getdmin(), flenght) + tol.GetR();
					//		UINT g =light[i].getg()*alpha *col.GetG()*dlength(light[i].getdmax(), light[i].getdmin(), flenght) + tol.GetG();
					//		UINT b =light[i].getb()*alpha *col.GetB()*dlength(light[i].getdmax(), light[i].getdmin(), flenght) + tol.GetB();
					//		tol.SetR(r);
					//		tol.SetG(g);
					//		tol.SetR(b);
					//	}
					//	else
					//	{
					//		UINT r = light[i].getr()*col.GetR() + tol.GetR();
					//		UINT g = light[i].getg()*col.GetG() + tol.GetG();
					//		UINT b = light[i].getb()*col.GetB() + tol.GetB();
					//		tol.SetR(r);
					//		tol.SetG(g);
					//		tol.SetR(b);

					//	}

					//}


					PutPixel(w, h, color);

				}
			}
		


		}
	}
}

bool  Graphics::Draw_UV(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y, float& u, float& v)
{


	Vector2d v1( x2-x1, y2 - y1);
	Vector2d v2(x3 - x1, y3-y1);
	Vector2d vx0( x1-x, y1-y);
	Vector2d vx1( x2-x,y2-y );
	Vector2d vx2( x3-x,  y3-y);

	Vector2d xv2(x2 - x, y2 - y);
	Vector2d vx3(x-x3, y - y3);
	float s = CrossProductF(v2.u,v2.v,v1.u,v1.v);
	float s1= CrossProductF(vx1.u, vx1.v,vx0.u, vx0.v);
	float s2 = CrossProductF(vx2.u, vx2.v, vx1.u, vx1.v);
	float s3 = CrossProductF(vx1.u, vx1.v, vx2.u, vx2.v);
	u = s1 / s;
	v = s2 / s;


	return u>= 0 && v >= 0 && u + v <= 1;

	/*Vector2d v0(x2 - x1, y2 - y1);
	Vector2d v1(x3 - x1, y3 - y1);
	Vector2d v3(x3 - x2, y3 - y2);
	Vector2d v2(x - x1, y - y1);

	float dot00 = DotProduct(&v0, &v0);
	float dot01 = DotProduct(&v0, &v1);
	float dot02 = DotProduct(&v0, &v2);
	float dot11 = DotProduct(&v1, &v1);
	float dot12 = DotProduct(&v1, &v2);

	float ux = dot00 * dot11 - dot01 * dot01;
	if (ux == 0)
	{
		u = 0;
		v = 0;
	}
	else
	{
		u = (dot11*dot02 - dot01 * dot12) / (dot11*dot00 - dot01 * dot01);

		v = (dot00*dot12 - dot01 * dot02) / (dot11*dot00 - dot01 * dot01);
	}
	return u > 0 && v > 0 && u + v < 1;*/
}



bool Graphics::Draw_UV(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y, float& u, float& v)
{
	/*Vector2d v1(x2 - x1, y2 - y1);
	Vector2d v2(x3 - x1, y3 - y1);
	Vector2d v3(x3 - x2, y3 - y2);
	Vector2d v0(x - x1, y - y1);
	float s = CrossProductF(v1.u,v1.v,v2.u,v2.v);
	float s1= CrossProductF(v1.u, v1.v,v0.u, v0.v);
	float s2 = CrossProductF(v0.u, v0.v,v2.u, v2.v);
	float s3 = CrossProductF(v1.u, v1.v, v2.u, v2.v);
	u = s1 / s;
	v = s2 / s;*/

	Vector2d v0(x2 - x1, y2 - y1);
	Vector2d v1(x3 - x1, y3 - y1);
	Vector2d v3(x3 - x2, y3 - y2);
	Vector2d v2(x - x1, y - y1);

	float dot00 = DotProduct(&v0, &v0);
	float dot01 = DotProduct(&v0,&v1);
	float dot02 = DotProduct(&v0, &v2);
	float dot11 = DotProduct(&v1, &v1);
	float dot12 = DotProduct(&v1, &v2);
	
	float ux = dot00 * dot11 - dot01 * dot01;
	if (ux == 0)
	{
		u = 0;
		v = 0;
	}
	else
	{
		u = (dot11*dot02 - dot01 * dot12) / (dot11*dot00 - dot01 * dot01);

		v = (dot00*dot12 - dot01 * dot02) / (dot11*dot00 - dot01 * dot01);
	}
	
	
	

	return u > 0 && v > 0 && u + v < 1;




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

void Graphics::postprocessTemporaa(float jx, float jy, DrawX& dx)
{
	static const float SampleOffsets[9][2] =
	{
		{ -1.0f, -1.0f },
		{  0.0f, -1.0f },
		{  1.0f, -1.0f },
		{ -1.0f,  0.0f },
		{  0.0f,  0.0f },
		{  1.0f,  0.0f },
		{ -1.0f,  1.0f },
		{  0.0f,  1.0f },
		{  1.0f,  1.0f },
	};

	for (int y = 0; y < ScreenHeight; ++y)
	{
		for (int x = 0; x < ScreenWidth; ++x)
		{
			float depth = PostProcess::GetApplcation()->GetDepth(x, y);
			MotionVector motionv = PostProcess::GetApplcation()->Getmotionvector(x, y);
			int motionx = motionv.MotingX + x + 0.5;
			int motiony = motionv.MotingY + y + 0.5;
			if (motionv.MotingX != 0.0f || motionv.MotingY != 0.0f)
			{
				if (motionx >= 1279||motionx<0)
				{
					int q = 0;
				}

				if (motiony >= 719|| motiony < 0)
				{
					int q = 0;
				}

				Color historyColor = prevpSysBuffer[Graphics::ScreenWidth * motiony + motionx];
				Color currcolor = pSysBuffer[Graphics::ScreenWidth * y + x];
				MXFlaot3 history;
				history.x = (float)historyColor.GetR() / 255.0f;
				history.y = (float)historyColor.GetG() / 255.0f;
				history.z = (float)historyColor.GetB() / 255.0f;

				MXFlaot3 curr;

				curr.x = (float)currcolor.GetR() / 255.0f;
				curr.y = (float)currcolor.GetG() / 255.0f;
				curr.z = (float)currcolor.GetB() / 255.0f;

				MXFlaot3 m1;
				MXFlaot3 m2;
				MXFlaot3 sigma;
				MXFlaot3 u;

				MXFlaot3 min;
				MXFlaot3 max;
				for (int i = 0; i < 9; i++)
				{
					int x1 = x + SampleOffsets[i][0];
					int y1 = y + SampleOffsets[i][1];
					Color color = pSysBuffer[Graphics::ScreenWidth * y1 + x1];
					m1.x += color.GetR() / 255.f;
					m1.y += color.GetG() / 255.f;
					m1.z += color.GetB() / 255.f;
					m2.x += (color.GetR() / 255.f)* (color.GetR() / 255.f);
					m2.y += (color.GetG() / 255.f)*(color.GetG() / 255.f);
					m2.z += (color.GetB() / 255.f)*(color.GetB() / 255.f);

				}

				u.x = m1.x / 9.0f;
				u.y = m1.y / 9.0f;
				u.z = m1.z / 9.0f;
				sigma.x = sqrt(m2.x / 9.0f - u.x*u.x);
				sigma.y = sqrt(m2.y / 9.0f - u.y*u.y);
				sigma.z = sqrt(m2.z / 9.0f - u.z*u.z);
				min.x = u.x - sigma.x;
				min.y = u.y - sigma.y;
				min.z = u.z - sigma.z;
				max.x = u.x + sigma.x;
				max.y = u.y + sigma.y;
				max.z = u.z + sigma.z;

				/*	RGBTOYCOCG(min);
					RGBTOYCOCG(max);
					RGBTOYCOCG(u);*/

				MXFlaot3 his = TemporalClip(min, max, u, history);

				/*YCOCGTORGB(his);*/
				//YCOCGTORGB(history);
				unsigned char prevred = his.x*255.f;
				unsigned char prevgreen = his.y*255.f;
				unsigned char prevblue = his.z*255.f;

				unsigned int green = pSysBuffer[Graphics::ScreenWidth * y + x].GetG() * 0.05f + 0.95f*prevgreen;
				unsigned int red = pSysBuffer[Graphics::ScreenWidth * y + x].GetR() * 0.05f + 0.95 *prevred;
				unsigned int blue = pSysBuffer[Graphics::ScreenWidth * y + x].GetB() * 0.05f + 0.95f*prevblue;
				pSysBuffer[Graphics::ScreenWidth * y + x] = Color(red, green, blue);
			}
			else
			{
				unsigned int green = pSysBuffer[Graphics::ScreenWidth * y + x].GetG() * 0.05f + prevpSysBuffer[Graphics::ScreenWidth * y + x].GetG()*0.95f;
				unsigned int red = pSysBuffer[Graphics::ScreenWidth * y + x].GetR() * 0.05f + prevpSysBuffer[Graphics::ScreenWidth * y + x].GetR()*0.95f;
				unsigned int blue = pSysBuffer[Graphics::ScreenWidth * y + x].GetB() * 0.05f + prevpSysBuffer[Graphics::ScreenWidth * y + x].GetB()*0.95f;
				pSysBuffer[Graphics::ScreenWidth * y + x] = Color(red, green, blue);
			}

		}
	}


}



MXFlaot3 Graphics::TemporalClip(const MXFlaot3 & min, const MXFlaot3 & max, const MXFlaot3 & avg, MXFlaot3 & out)
{
	MXFlaot3 p_clip;
	p_clip = max + min;
	p_clip.x =p_clip.x*0.5;
	p_clip.y = p_clip.y*0.5;
	p_clip.z = p_clip.z*0.5;
	MXFlaot3 e_clip = max - min;
	e_clip.x = e_clip.x*0.5;
	e_clip.y = e_clip.y*0.5;
	e_clip.z = e_clip.z*0.5;

	MXFlaot3 v_clip = out - avg;

	MXFlaot3 v_uint;
	if (e_clip.x == 0)
	{
		v_uint.x = 0;
	}
	else
	{
		v_uint.x = v_clip.x / e_clip.x;
	}
	if (e_clip.y == 0)
	{
		v_uint.y = 0;
	}
	else
	{
		v_uint.y = v_clip.y / e_clip.y;
	}
	if (e_clip.y == 0)
	{
		v_uint.z = 0;
	}
	else
	{
		v_uint.z = v_clip.z / e_clip.z;
	}

	
	float maxuint = Max(abs(v_uint.x), Max(abs(v_uint.y), abs(v_uint.z)));

	if (maxuint > 1.0)
	{
		p_clip.x = p_clip.x + v_clip.x / maxuint;
		p_clip.y = p_clip.y + v_clip.y / maxuint;
		p_clip.z= p_clip.z + v_clip.z / maxuint;
		return  p_clip;
	}
	else
	{
		return out;
	}

}






void Graphics::CopyColor()
{
	for (int y = 0; y < 720; ++y)
	{
		for (int x = 0; x < 1280; ++x)
		{
			prevpSysBuffer[Graphics::ScreenWidth * y + x] = pSysBuffer[Graphics::ScreenWidth * y + x];
		}
	}
	
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