/// Code based on
/// Fast Fourier Transform for Image Processing in DirectX 11 (https://software.intel.com/en-us/articles/fast-fourier-transform-for-image-processing-in-directx-11)
/// with licence
//--------------------------------------------------------------------------------------
// Copyright 2014 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "fft.h"
#include <dxgiformat.h>
#include <direct.h>
#include <string>
#include <d3dcompiler.h>
#include <WinBase.h>
#include <Synchapi.h>
#include <fstream>
#include <sstream>

#include "DirectXHelper.h"

using namespace concurrency;
using namespace Windows;

#define SAFE_RELEASE(p) (p)

//const double M_PI = 3.1415926535897932384626433832795029;

namespace Util2D
{
#define roundup(x,y) ( (int)y * (int)((x + y - 1)/y))
	void CopyTextureNoStretch(ID3D11Device* device, ID3D11DeviceContext *context, ID3D11ShaderResourceView *srcSRV, ID3D11UnorderedAccessView *dstUAV, int srcWidth, int srcHeight, int dstWidth, int dstHeight)
	{
		static Microsoft::WRL::ComPtr<ID3D11ComputeShader> mCopyCS = nullptr;
		static ID3D11Device* mDevice = nullptr;
		if (mCopyCS == nullptr || mDevice != device)
		{
			mDevice = device;

			DX::CreateShader(device, mCopyCS.ReleaseAndGetAddressOf(), L"CopyComputeShader.cso");
		}

		ID3D11UnorderedAccessView* pClearUAVs[] = { NULL, NULL };
		ID3D11ShaderResourceView* pClearSRVs[] = { NULL, NULL, NULL };

		context->PSSetShaderResources(0, 3, pClearSRVs);
		context->CSSetShaderResources(0, 1, pClearSRVs);
		context->CSSetUnorderedAccessViews(0, 2, pClearUAVs, NULL);
		context->CSSetShader(mCopyCS.Get(), NULL, 0);

		float values[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		context->ClearUnorderedAccessViewFloat(dstUAV, values);
		context->CSSetShaderResources(0, 1, &srcSRV);
		context->CSSetUnorderedAccessViews(0, 1, &dstUAV, NULL);
		int dispatchX = roundup(std::min(srcWidth, dstWidth) / 16, 16);
		int dispatchY = roundup(std::min(srcHeight, dstHeight) / 16, 16);
		context->Dispatch(dispatchX, dispatchY, 1);

		context->CSSetShaderResources(0, 1, pClearSRVs);
		context->CSSetUnorderedAccessViews(0, 2, pClearUAVs, NULL);
	}
}

void BitReverse(int *Indices, int N, int n)
{
	unsigned int mask = 0x1;
	for (int j = 0; j < N; j++)
	{
		unsigned int val = 0x0;
		int temp = int(Indices[j]);
		for (int i = 0; i < n; i++)
		{
			unsigned int t = (mask & temp);
			val = (val << 1) | t;
			temp = temp >> 1;
		}
		Indices[j] = val;
	}
}

void GetButterflyValues(int butterflyPass, int NumButterflies, int x, int *i1, int *i2, float *w1, float *w2)
{
	int sectionWidth = 2 << butterflyPass;
	int halfSectionWidth = sectionWidth / 2;

	int sectionStartOffset = x & ~(sectionWidth - 1);
	int halfSectionOffset = x & (halfSectionWidth - 1);
	int sectionOffset = x & (sectionWidth - 1);

	*w1 = float(cosl(2.0*M_PI*sectionOffset / (float)sectionWidth));
	*w2 = float(-sinl(2.0*M_PI*sectionOffset / (float)sectionWidth));

	*i1 = sectionStartOffset + halfSectionOffset;
	*i2 = sectionStartOffset + halfSectionOffset + halfSectionWidth;

	if (butterflyPass == 0)
	{
		BitReverse(i1, 1, NumButterflies);
		BitReverse(i2, 1, NumButterflies);
	}
}

CFFT::CFFT()
{
	ZeroMemory(m_textures, sizeof(m_textures));
	ZeroMemory(m_textureUAV, sizeof(m_textureUAV));
	ZeroMemory(m_textureSRV, sizeof(m_textureSRV));
	ZeroMemory(m_renderTargetViews, sizeof(m_renderTargetViews));
	ZeroMemory(m_passLength, sizeof(m_passLength));
	ZeroMemory(m_butterflyCount, sizeof(m_butterflyCount));
	ZeroMemory(m_CSButterfly, sizeof(m_CSButterfly));
	m_CSConstantBuffer = nullptr;
	m_state = CFFTState_NotInited;
}

void CFFT::BuildButterflyTexture(FFTPass butterflyType, int width)
{
	m_butterflyCount[butterflyType] = (int)(logf(float(width)) / logf(2.0));
}

void CFFT::CompileShader(FFTPass pass, FFTPassTransform transform)
{
	if (m_CSButterfly[pass][transform] == nullptr)
	{
		auto shaderName = (pass == FFTPass_X ? L"row" : L"column") + L"." + (transform == FFTPassTransform_Inverse ? L"inverse" : L"forward") + L".fft.cso";

		DX::CreateShader(m_pDevice, m_CSButterfly[pass][transform].ReleaseAndGetAddressOf(), shaderName->Data());
	}
}

void CFFT::Init(ID3D11Device *pDevice, ID3D11DeviceContext *pContext, int width, int height)
{
	if (m_pDevice != pDevice || m_pContext != pContext)
	{
		for (int passIndex = 0; passIndex < FFTPass_Count; passIndex++)
		{
			for (int transformIndex = 0; transformIndex < FFTPassTransform_Count; transformIndex++)
			{
				m_CSButterfly[passIndex][transformIndex] = nullptr;
			}
		}

		m_pContext = pContext;
		m_pDevice = pDevice;
	}

	m_textureWidth = DX::RoundUpToPowerOfTwo(width);
	m_textureHeight = DX::RoundUpToPowerOfTwo(height);
	m_passLength[FFTPass_X] = m_textureWidth;
	m_passLength[FFTPass_Y] = m_textureHeight;

	D3D11_TEXTURE2D_DESC textureDesc;
	memset(&textureDesc, 0, sizeof(textureDesc));
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.Width = m_textureWidth;
	textureDesc.Height = m_textureHeight;
	textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	for (int i = 0; i < FFTTexture_Count; i++)
	{
		m_pDevice->CreateTexture2D(&textureDesc, NULL, &m_textures[i]);
		m_pDevice->CreateUnorderedAccessView(m_textures[i].Get(), NULL, m_textureUAV[i].ReleaseAndGetAddressOf());
		m_pDevice->CreateShaderResourceView(m_textures[i].Get(), NULL, m_textureSRV[i].ReleaseAndGetAddressOf());
		m_pDevice->CreateRenderTargetView(m_textures[i].Get(), NULL, m_renderTargetViews[i].ReleaseAndGetAddressOf());
	}

	BuildButterflyTexture(FFTPass_X, m_textureWidth);
	BuildButterflyTexture(FFTPass_Y, m_textureHeight);

	CD3D11_BUFFER_DESC desc(4 * sizeof(UINT), D3D11_BIND_CONSTANT_BUFFER);
	pDevice->CreateBuffer(&desc, NULL, m_CSConstantBuffer.ReleaseAndGetAddressOf());

	// Build Compute Shaders
	for (int passIndex = 0; passIndex < FFTPass_Count; passIndex++)
	{
		for (int transformIndex = 0; transformIndex < FFTPassTransform_Count; transformIndex++)
		{
			CompileShader((FFTPass)passIndex, (FFTPassTransform)transformIndex);
		}
	}

	m_state = CFFTState_Ready;

	m_currentTextureIndex = 0;
	m_nextTextureIndex = 1;
}

void CFFT::Shutdown()
{
	// m_pDevice = nullptr;
	// m_pContext = nullptr;
	m_CSConstantBuffer = nullptr;
	m_textureWidth = -1;
	m_textureHeight = -1;

	m_state = CFFTState_NotInited;

	for (int i = 0; i < FFTTexture_Count; i++)
	{
		m_textures[i] = nullptr;
		m_textureUAV[i] = nullptr;
		m_textureSRV[i] = nullptr;
		m_renderTargetViews[i] = nullptr;
	}
}

void CFFT::Resize(int width, int height)
{
	int newWidth = DX::RoundUpToPowerOfTwo(width);
	int newHeight = DX::RoundUpToPowerOfTwo(height);

	if (newWidth != m_textureWidth || newHeight != m_textureHeight)
	{
		Init(m_pDevice, m_pContext, newWidth, newHeight);
	}
}

void CFFT::ToggleTextures()
{
	int temp = m_currentTextureIndex;
	m_currentTextureIndex = m_nextTextureIndex;
	m_nextTextureIndex = temp;
}

void CFFT::GetDimensions(int& width, int& height)
{
	width = m_textureWidth;
	height = m_textureHeight;
}

void CFFT::GetSRV(ID3D11ShaderResourceView **ppSRVR, ID3D11ShaderResourceView **ppSRVI)
{
	if (ppSRVR)
	{
		*ppSRVR = m_textureSRV[FFTTexture_Real0 + 2 * m_currentTextureIndex].Get();
	}
	if (ppSRVI)
	{
		*ppSRVI = m_textureSRV[FFTTexture_Imaginary0 + 2 * m_currentTextureIndex].Get();
	}
}

void CFFT::GetUAV(ID3D11UnorderedAccessView **ppUAVR, ID3D11UnorderedAccessView **ppUAVI)
{
	if (ppUAVR)
	{
		*ppUAVR = m_textureUAV[FFTTexture_Real0 + 2 * m_currentTextureIndex].Get();
	}
	if (ppUAVI)
	{
		*ppUAVI = m_textureUAV[FFTTexture_Imaginary0 + 2 * m_currentTextureIndex].Get();
	}
}

void CFFT::GetRTV(ID3D11RenderTargetView **ppRTVR, ID3D11RenderTargetView **ppRTVI)
{
	if (ppRTVR)
	{
		*ppRTVR = m_renderTargetViews[FFTTexture_Real0 + 2 * m_currentTextureIndex].Get();
	}
	if (ppRTVI)
	{
		*ppRTVI = m_renderTargetViews[FFTTexture_Imaginary0 + 2 * m_currentTextureIndex].Get();
	}
}

void CFFT::GetNextSRV(ID3D11ShaderResourceView **ppSRVR, ID3D11ShaderResourceView **ppSRVI)
{
	if (ppSRVR)
	{
		*ppSRVR = m_textureSRV[FFTTexture_Real0 + 2 * m_nextTextureIndex].Get();
	}
	if (ppSRVI)
	{
		*ppSRVI = m_textureSRV[FFTTexture_Imaginary0 + 2 * m_nextTextureIndex].Get();
	}
}

void CFFT::GetNextUAV(ID3D11UnorderedAccessView **ppUAVR, ID3D11UnorderedAccessView **ppUAVI)
{
	if (ppUAVR)
	{
		*ppUAVR = m_textureUAV[FFTTexture_Real0 + 2 * m_nextTextureIndex].Get();
	}
	if (ppUAVI)
	{
		*ppUAVI = m_textureUAV[FFTTexture_Imaginary0 + 2 * m_nextTextureIndex].Get();
	}
}

void CFFT::GetNextRTV(ID3D11RenderTargetView **ppRTVR, ID3D11RenderTargetView **ppRTVI)
{
	if (ppRTVR)
	{
		*ppRTVR = m_renderTargetViews[FFTTexture_Real0 + 2 * m_nextTextureIndex].Get();
	}
	if (ppRTVI)
	{
		*ppRTVI = m_renderTargetViews[FFTTexture_Imaginary0 + 2 * m_nextTextureIndex].Get();
	}
}

void CFFT::GetTextures(ID3D11Texture2D **ppTextureR, ID3D11Texture2D **ppTextureI)
{
	if (ppTextureR)
	{
		*ppTextureR = m_textures[FFTTexture_Real0 + 2 * m_currentTextureIndex].Get();
	}
	if (ppTextureI)
	{
		*ppTextureI = m_textures[FFTTexture_Imaginary0 + 2 * m_currentTextureIndex].Get();
	}
}

void CFFT::SetInputSRV(ID3D11ShaderResourceView *srv, int width, int height)
{
	m_currentTextureIndex = 0;
	m_nextTextureIndex = 1;

	// CopySubResource can't be used because the textures are different formats
	Util2D::CopyTextureNoStretch(m_pDevice, m_pContext, srv, m_textureUAV[FFTTexture_Real0].Get(), width, height, m_textureWidth, m_textureHeight);
	float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_pContext->ClearRenderTargetView(m_renderTargetViews[FFTTexture_Imaginary0].Get(), color);
}

void CFFT::SetInputSRV(ID3D11ShaderResourceView *real, ID3D11ShaderResourceView *imag, int width, int height)
{
	m_currentTextureIndex = 0;
	m_nextTextureIndex = 1;

	// CopySubResource can't be used because the textures are different formats
	Util2D::CopyTextureNoStretch(m_pDevice, m_pContext, real, m_textureUAV[FFTTexture_Real0].Get(), width, height, m_textureWidth, m_textureHeight);
	Util2D::CopyTextureNoStretch(m_pDevice, m_pContext, imag, m_textureUAV[FFTTexture_Imaginary0].Get(), width, height, m_textureWidth, m_textureHeight);
}

void CFFT::Transform(FFTPassTransform transform)
{
	RunDispatchMultiplePass(FFTPass_X, transform);
	RunDispatchMultiplePass(FFTPass_Y, transform);
}

void CFFT::RunDispatchMultiplePass(FFTPass pass, FFTPassTransform transform)
{
	ID3D11DeviceContext *context = m_pContext;
	ID3D11SamplerState* pSS = NULL;
	context->CSSetSamplers(0, 1, &pSS);

	ID3D11UnorderedAccessView* pUAVs[] = { NULL, NULL };
	ID3D11ShaderResourceView* pSRVs[] = { NULL, NULL, NULL };

	ID3D11Buffer* buffers[]{ m_CSConstantBuffer.Get(), };

	context->PSSetShaderResources(0, 3, pSRVs);
	context->CSSetShaderResources(0, 1, pSRVs);
	context->CSSetUnorderedAccessViews(0, 2, pUAVs, NULL);
	context->CSSetConstantBuffers(0, 1, buffers);
	context->CSSetShader(m_CSButterfly[pass][transform].Get(), NULL, 0);

	for (int i = 0; i < m_butterflyCount[pass]; i++)
	{
		// update butterfly index in constant buffer
		UINT data[4]{ i, m_butterflyCount[pass], m_passLength[pass], i, };
		context->UpdateSubresource(m_CSConstantBuffer.Get(), 0, nullptr, data, 0, 0);

		UINT GroupSize = 16;

		pSRVs[0] = m_textureSRV[FFTTexture_Real0 + 2 * m_currentTextureIndex].Get();
		pSRVs[1] = m_textureSRV[FFTTexture_Imaginary0 + 2 * m_currentTextureIndex].Get();
		pSRVs[2] = NULL;
		pUAVs[0] = m_textureUAV[FFTTexture_Real0 + 2 * m_nextTextureIndex].Get();
		pUAVs[1] = m_textureUAV[FFTTexture_Imaginary0 + 2 * m_nextTextureIndex].Get();

		context->CSSetUnorderedAccessViews(0, 2, pUAVs, NULL);
		context->CSSetShaderResources(0, 3, pSRVs);

		context->Dispatch(m_textureWidth / GroupSize, m_textureHeight / GroupSize, 1);

		pSRVs[0] = NULL; pSRVs[1] = NULL; pSRVs[2] = NULL;
		context->CSSetShaderResources(0, 3, pSRVs);
		pUAVs[0] = NULL; pUAVs[1] = NULL;
		context->CSSetUnorderedAccessViews(0, 2, pUAVs, NULL);

		ToggleTextures();
	}
}
