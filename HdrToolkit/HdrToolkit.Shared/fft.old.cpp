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
#include "fft.old.h.h"
#include <dxgiformat.h>
#include <direct.h>
#include <string>
#include <d3dcompiler.h>
#include <WinBase.h>
#include <Synchapi.h>
#include <fstream>
#include <sstream>

using namespace concurrency;
using namespace Windows;

#define SAFE_RELEASE(p) (p)

const double M_PI = 3.1415926535897932384626433832795029;
namespace Util2D
{
	HRESULT CompileShaderFromFile(void const* bytes, size_t size, LPCSTR szShaderName, LPCSTR szEntryPoint, CONST D3D_SHADER_MACRO* pDefines, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
	{
		HRESULT hr = S_OK;

		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL0;
#if defined( DEBUG ) || defined( _DEBUG )
		// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
		// Setting this flag improves the shader debugging experience, but still allows 
		// the shaders to be optimized and to run exactly the way they will run in 
		// the release configuration of this program.
		dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

		Microsoft::WRL::ComPtr<ID3DBlob> errMsgs;
		hr = D3DCompile2(bytes, size, szShaderName, pDefines, nullptr, szEntryPoint, szShaderModel, dwShaderFlags, 0, 0, nullptr, 0, ppBlobOut, errMsgs.GetAddressOf());
		if (FAILED(hr))
		{
			if (errMsgs != NULL)
				OutputDebugStringA((char*)errMsgs->GetBufferPointer());
			if (errMsgs) errMsgs->Release();
			return hr;
		}
		if (errMsgs) errMsgs->Release();

		return S_OK;
	}

	HRESULT CompileComputeShaderFromFile(ID3D11Device *device, void const* bytes, size_t size, LPCSTR szShaderName, LPCSTR szEntryPoint, CONST D3D_SHADER_MACRO* pDefines, LPCSTR szShaderModel, ID3D11ComputeShader **ppOutShader)
	{
		ID3DBlob *pBlob;
		HRESULT hr = CompileShaderFromFile(bytes, size, szShaderName, szEntryPoint, pDefines, szShaderModel, &pBlob);
		if (hr == S_OK)
		{
			device->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, ppOutShader);
		}
		return hr;
	}

#define min(x,y) ((x) < (y) ? (x) : (y))
#define roundup(x,y) ( (int)y * (int)((x + y - 1)/y))
	void CopyTextureNoStretch(ID3D11Device* device, ID3D11DeviceContext *context, ID3D11ShaderResourceView *srcSRV, ID3D11UnorderedAccessView *dstUAV, int srcWidth, int srcHeight, int dstWidth, int dstHeight)
	{
		static Microsoft::WRL::ComPtr<ID3D11ComputeShader> mCopyCS = nullptr;
		if (mCopyCS == nullptr)
		{
			std::ifstream input("copycs.hlsl");
			std::string line;
			std::stringstream stringstream;

			while (std::getline(input, line))
			{
				stringstream << line << "\n";
			}

			auto shaderCode = stringstream.str();

			CompileComputeShaderFromFile(device, shaderCode.data(), shaderCode.size() * sizeof(char), "copycs.hlsl", "CopyTextureCS", NULL, "cs_5_0", &mCopyCS);
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
		int dispatchX = roundup(min(srcWidth, dstWidth) / 16, 16);
		int dispatchY = roundup(min(srcHeight, dstHeight) / 16, 16);
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

static int RoundUpToPowerOfTwo(int in)
{
	int i = 1;
	for (; i < in; i *= 2);
	return i;
}

CFFT::CFFT(bool compileThreaded)
{
	ZeroMemory(mTextures, sizeof(mTextures));
	ZeroMemory(mTextureUAV, sizeof(mTextureUAV));
	ZeroMemory(mTextureSRV, sizeof(mTextureSRV));
	ZeroMemory(mRenderTargetViews, sizeof(mRenderTargetViews));
	ZeroMemory(mPassLength, sizeof(mPassLength));
	ZeroMemory(mButterflyCount, sizeof(mButterflyCount));
	// ZeroMemory(mButterflyTexture, sizeof(mButterflyTexture)); // Only FFTButterflyLUT_Off
	// ZeroMemory(mButterflySRV, sizeof(mButterflySRV)); // Only FFTButterflyLUT_Off
	ZeroMemory(mCSButterfly, sizeof(mCSButterfly));
	ZeroMemory(mThreadInfo, sizeof(mThreadInfo));
	mCompileThreaded = compileThreaded;
	mCSConstantBuffer = nullptr;
	mState = CFFTState_NotInited;
}

void CFFT::BuildButterflyTexture(FFTPass butterflyType, int width)
{
	D3D11_SUBRESOURCE_DATA butterflyData;
	ID3D11Device *device = mpDevice;

	mButterflyCount[butterflyType] = (int)(logf(float(width)) / logf(2.0));
	return; // Only FFTButterflyLUT_Off

	int size = 4 * sizeof(float) * width * mButterflyCount[butterflyType];
	float *butterflyBuffer = (float*)malloc(size);

	for (int pass = 0; pass < mButterflyCount[butterflyType]; pass++)
	{
		for (int x = 0; x < width; x++)
		{
			int i1, i2;
			float w1, w2;
			GetButterflyValues(pass, mButterflyCount[butterflyType], x, &i1, &i2, &w1, &w2);
			float *writeLoc = &butterflyBuffer[pass * width * 4 + x * 4];
			writeLoc[0] = (float)i1; // R
			writeLoc[1] = (float)i2; // G
			writeLoc[2] = w1; // B
			writeLoc[3] = w2; // A
		}
	}

	butterflyData.pSysMem = butterflyBuffer;
	butterflyData.SysMemPitch = 4 * sizeof(float) * width;
	butterflyData.SysMemSlicePitch = 0;

	D3D11_TEXTURE2D_DESC textureDesc;
	memset(&textureDesc, 0, sizeof(textureDesc));
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.Width = width;
	textureDesc.Height = mButterflyCount[butterflyType];
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	// HRESULT hr = device->CreateTexture2D(&textureDesc, &butterflyData, &mButterflyTexture[butterflyType]);
	// hr = device->CreateShaderResourceView(mButterflyTexture[butterflyType].Get(), NULL, &mButterflySRV[butterflyType]);

	free(butterflyBuffer);
}

void CFFT::CompileShader(FFTTechnique technique, FFTPass pass, /*FFTButterflyLUT butterflyLUT,*/ FFTPassTransform transform)
{
	std::string butterflyCountStr = std::to_string(mButterflyCount[pass]);
	std::string lengthStr = std::to_string(mPassLength[pass]);
	const D3D_SHADER_MACRO defines[] =
	{
		"LENGTH", lengthStr.c_str(),
		"BUTTERFLY_COUNT", butterflyCountStr.c_str(),
		transform == FFTPassTransform_Forward ? "TRANSFORM_FORWARD" : "TRANSFORM_INVERSE", "",
		pass == FFTPass_X ? "ROWPASS" : "COLPASS", "",
		/*butterflyLUT == FFTButterflyLUT_On ? "BUTTERFLY_LUT" :*/ "BUTTERFLY_COMPUTE", "", // Only FFTButterflyLUT_Off
		NULL, NULL
	};
	switch (technique)
	{
	case FFTTechnique_UAV:
	{
		std::ifstream input("fftuav.hlsl");
		std::string line;
		std::stringstream stringstream;

		while (std::getline(input, line))
		{
			stringstream << line << "\n";
		}

		auto shaderCode = stringstream.str();

		Util2D::CompileComputeShaderFromFile(mpDevice, shaderCode.data(), shaderCode.size() * sizeof(char), "fftuav.hlsl", "Butterfly", defines, "cs_5_0", &mCSButterfly[technique][pass]/*[butterflyLUT]*/[transform]); // Only FFTButterflyLUT_Off
	} break;

	case FFTTechnique_SLM:
	{
		std::ifstream input("fftslm.hlsl");
		std::string line;
		std::stringstream stringstream;

		while (std::getline(input, line))
		{
			stringstream << line << "\n";
		}

		auto shaderCode = stringstream.str();

		Util2D::CompileComputeShaderFromFile(mpDevice, shaderCode.data(), shaderCode.size() * sizeof(char), "fftslm.hlsl", "ButterflySLM", defines, "cs_5_0", &mCSButterfly[technique][pass]/*[butterflyLUT]*/[transform]); // Only FFTButterflyLUT_Off
	} break;
	};
}

DWORD CFFT::CompileThreadFunction(LPVOID *context)
{
	CompileThreadInfo *info = (CompileThreadInfo*)context;
	info->fft->CompileShader(info->technique, info->pass, /*info->butterflyLUT,*/ info->transform); // Only FFTButterflyLUT_Off
	info->done = true;
	return 1;
}

void CFFT::Init(ID3D11Device *pDevice, ID3D11DeviceContext *pContext, int width, int height)
{
	mpContext = pContext;
	mpDevice = pDevice;

	mTextureWidth = RoundUpToPowerOfTwo(width);
	mTextureHeight = RoundUpToPowerOfTwo(height);
	mPassLength[FFTPass_X] = mTextureWidth;
	mPassLength[FFTPass_Y] = mTextureHeight;

	D3D11_TEXTURE2D_DESC textureDesc;
	memset(&textureDesc, 0, sizeof(textureDesc));
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.Width = mTextureWidth;
	textureDesc.Height = mTextureHeight;
	textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	for (int i = 0; i < FFTTexture_Count; i++)
	{
		textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		if (i == FFTTexture_Real0 || i == FFTTexture_Imaginary0)
		{
			textureDesc.BindFlags = textureDesc.BindFlags | D3D11_BIND_RENDER_TARGET;
		}
		mpDevice->CreateTexture2D(&textureDesc, NULL, &mTextures[i]);
		mpDevice->CreateUnorderedAccessView(mTextures[i].Get(), NULL, &mTextureUAV[i]);
		mpDevice->CreateShaderResourceView(mTextures[i].Get(), NULL, &mTextureSRV[i]);

	}
	mpDevice->CreateRenderTargetView(mTextures[FFTTexture_Real0].Get(), NULL, &mRenderTargetViews[FFTTexture_Real0]);
	mpDevice->CreateRenderTargetView(mTextures[FFTTexture_Imaginary0].Get(), NULL, &mRenderTargetViews[FFTTexture_Imaginary0]);

	BuildButterflyTexture(FFTPass_X, mTextureWidth);
	BuildButterflyTexture(FFTPass_Y, mTextureHeight);

	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.ByteWidth = 16;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pDevice->CreateBuffer(&desc, NULL, &mCSConstantBuffer);

	// Build Compute Shaders
	int threadIndex = 0;
	for (int techniqueIndex = 0; techniqueIndex < FFTTechnique_Count; techniqueIndex++)
	{
		for (int passIndex = 0; passIndex < FFTPass_Count; passIndex++)
		{
			// for (int butterflyLUTIndex = 0; butterflyLUTIndex < FFTButterflyLUT_Count; butterflyLUTIndex++)
			{
				for (int transformIndex = 0; transformIndex < FFTPassTransform_Count; transformIndex++)
				{
					if (mCompileThreaded)
					{
						mThreadInfo[threadIndex].fft = this;
						mThreadInfo[threadIndex].technique = (FFTTechnique)techniqueIndex;
						mThreadInfo[threadIndex].pass = (FFTPass)passIndex;
						mThreadInfo[threadIndex].transform = (FFTPassTransform)transformIndex;
						// mThreadInfo[threadIndex].butterflyLUT = (FFTButterflyLUT)butterflyLUTIndex;
						mThreadInfo[threadIndex].done = false;
						mThreadInfo[threadIndex].handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&CompileThreadFunction, &mThreadInfo[threadIndex], 0, NULL);
						threadIndex++;
					}
					else
					{
						CompileShader((FFTTechnique)techniqueIndex, (FFTPass)passIndex, /*(FFTButterflyLUT)butterflyLUTIndex,*/ (FFTPassTransform)transformIndex);
					}
				}
			}
		}
	}

	mState = mCompileThreaded ? CFFTState_Compiling : CFFTState_Ready;

	mCurrentTextureIndex = 0;
}

void CFFT::Update()
{
	if (mState == CFFTState_Compiling)
	{
		mCompiledCount = 0;
		for (int i = 0; i < COMPILE_THREAD_COUNT; i++)
		{
			mCompiledCount += mThreadInfo[i].done ? 1 : 0;
		}
		mState = (mCompiledCount == COMPILE_THREAD_COUNT) ? CFFTState_Ready : CFFTState_Compiling;
	}
}

void CFFT::Resize(int width, int height)
{
	int newWidth = RoundUpToPowerOfTwo(width);
	int newHeight = RoundUpToPowerOfTwo(height);

	if (newWidth != mTextureWidth || newHeight != mTextureHeight)
	{
		Shutdown();
		Init(mpDevice, mpContext, newWidth, newHeight);
	}
}

void CFFT::Shutdown()
{
	// wait for thread compiling to complete:
	for (int i = 0; i < COMPILE_THREAD_COUNT; i++)
	{
		// WaitForSingleObject(mThreadInfo[i].handle, INFINITE);
		CloseHandle(mThreadInfo[i].handle);
		mThreadInfo[i].handle = NULL;
	}

	for (int i = 0; i < FFTTexture_Count; i++)
	{
		SAFE_RELEASE(mTextures[i]);
		SAFE_RELEASE(mTextureUAV[i]);
		SAFE_RELEASE(mTextureSRV[i]);
		SAFE_RELEASE(mRenderTargetViews[i]);
	}

	for (int i = 0; i < FFTPass_Count; i++)
	{
		// SAFE_RELEASE(mButterflyTexture[i]); // Only FFTButterflyLUT_Off
		// SAFE_RELEASE(mButterflySRV[i]); // Only FFTButterflyLUT_Off
	}

	SAFE_RELEASE(mCSConstantBuffer);
	for (int passIndex = 0; passIndex < FFTPass_Count; passIndex++)
	{
		for (int transformIndex = 0; transformIndex < FFTPassTransform_Count; transformIndex++)
		{
			// for (int butterflyLUTIndex = 0; butterflyLUTIndex < FFTButterflyLUT_Count; butterflyLUTIndex++)
			{
				for (int techniqueIndex = 0; techniqueIndex < FFTTechnique_Count; techniqueIndex++)
				{
					SAFE_RELEASE(mCSButterfly[techniqueIndex][passIndex]/*[butterflyLUTIndex]*/[transformIndex]); // Only FFTButterflyLUT_Off
				}
			}
		}
	}
}

void CFFT::ToggleTextures()
{
	int temp = mCurrentTextureIndex;
	mCurrentTextureIndex = mNextTextureIndex;
	mNextTextureIndex = temp;
}

void CFFT::GetSRV(ID3D11ShaderResourceView **ppSRVR, ID3D11ShaderResourceView **ppSRVI)
{
	if (ppSRVR)
	{
		*ppSRVR = mTextureSRV[FFTTexture_Real0 + 2 * mCurrentTextureIndex].Get();
	}
	if (ppSRVI)
	{
		*ppSRVI = mTextureSRV[FFTTexture_Imaginary0 + 2 * mCurrentTextureIndex].Get();
	}
}

void CFFT::GetTextures(ID3D11Texture2D **ppTextureR, ID3D11Texture2D **ppTextureI)
{
	if (ppTextureR)
	{
		*ppTextureR = mTextures[FFTTexture_Real0 + 2 * mCurrentTextureIndex].Get();
	}
	if (ppTextureI)
	{
		*ppTextureI = mTextures[FFTTexture_Imaginary0 + 2 * mCurrentTextureIndex].Get();
	}
}

void CFFT::SetInputSRV(ID3D11ShaderResourceView *srv, int width, int height)
{
	mCurrentTextureIndex = 0;
	mNextTextureIndex = 1;

	// CopySubResource can't be used because the textures are different formats
	Util2D::CopyTextureNoStretch(mpDevice, mpContext, srv, mTextureUAV[FFTTexture_Real0].Get(), width, height, mTextureWidth, mTextureHeight);
	float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	mpContext->ClearRenderTargetView(mRenderTargetViews[FFTTexture_Imaginary0].Get(), color);
}

void CFFT::Transform(FFTTechnique technique, /*FFTButterflyLUT butterflyLUT,*/ FFTPassTransform transform)
{
	if (technique == FFTTechnique_UAV)
	{
		RunDispatchMultiplePass(FFTPass_X, /*butterflyLUT,*/ transform); // Only FFTButterflyLUT_Off
		RunDispatchMultiplePass(FFTPass_Y, /*butterflyLUT,*/ transform); // Only FFTButterflyLUT_Off
	}
	else
	{
		RunDispatchSLM(FFTPass_X, /*butterflyLUT,*/ transform); // Only FFTButterflyLUT_Off
		RunDispatchSLM(FFTPass_Y, /*butterflyLUT,*/ transform); // Only FFTButterflyLUT_Off
	}
}

void CFFT::RunDispatchMultiplePass(FFTPass pass, /*FFTButterflyLUT butterflyLUT,*/ FFTPassTransform transform)
{
	ID3D11DeviceContext *context = mpContext;
	ID3D11SamplerState* pSS = NULL;
	context->CSSetSamplers(0, 1, &pSS);

	ID3D11UnorderedAccessView* pUAVs[] = { NULL, NULL };
	ID3D11ShaderResourceView* pSRVs[] = { NULL, NULL, NULL };

	context->PSSetShaderResources(0, 3, pSRVs);
	context->CSSetShaderResources(0, 1, pSRVs);
	context->CSSetUnorderedAccessViews(0, 2, pUAVs, NULL);
	context->CSSetConstantBuffers(0, 1, &mCSConstantBuffer);
	context->CSSetShader(mCSButterfly[FFTTechnique_UAV][pass]/*[butterflyLUT]*/[transform].Get(), NULL, 0); // Only FFTButterflyLUT_Off

	for (int i = 0; i < mButterflyCount[pass]; i++)
	{
		// update butterfly index in constant buffer
		D3D11_MAPPED_SUBRESOURCE MappedResource;
		context->Map(mCSConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
		*(UINT*)MappedResource.pData = i;
		context->Unmap(mCSConstantBuffer.Get(), 0);

		UINT GroupSize = 16;

		pSRVs[0] = mTextureSRV[FFTTexture_Real0 + 2 * mCurrentTextureIndex].Get();
		pSRVs[1] = mTextureSRV[FFTTexture_Imaginary0 + 2 * mCurrentTextureIndex].Get();
		pSRVs[2] = /*mButterflySRV[pass].Get();*/ NULL; // Only FFTButterflyLUT_Off
		pUAVs[0] = mTextureUAV[FFTTexture_Real0 + 2 * mNextTextureIndex].Get();
		pUAVs[1] = mTextureUAV[FFTTexture_Imaginary0 + 2 * mNextTextureIndex].Get();

		context->CSSetUnorderedAccessViews(0, 2, pUAVs, NULL);
		context->CSSetShaderResources(0, 3, pSRVs);

		context->Dispatch(mTextureWidth / GroupSize, mTextureHeight / GroupSize, 1);

		pSRVs[0] = NULL; pSRVs[1] = NULL; pSRVs[2] = NULL;
		context->CSSetShaderResources(0, 3, pSRVs);
		pUAVs[0] = NULL; pUAVs[1] = NULL;
		context->CSSetUnorderedAccessViews(0, 2, pUAVs, NULL);

		ToggleTextures();
	}
}

void CFFT::RunDispatchSLM(FFTPass pass, /*FFTButterflyLUT butterflyLUT,*/ FFTPassTransform transform)
{
	ID3D11SamplerState* pSS = NULL;
	ID3D11DeviceContext *context = mpContext;
	context->CSSetSamplers(0, 1, &pSS);

	ID3D11UnorderedAccessView* pUAVs[] = { NULL, NULL };
	ID3D11ShaderResourceView* pSRVs[] = { NULL, NULL, NULL };

	context->CSSetShaderResources(0, 3, pSRVs);
	context->CSSetUnorderedAccessViews(0, 2, pUAVs, NULL);
	context->CSSetShader(mCSButterfly[FFTTechnique_SLM][pass]/*[butterflyLUT]*/[transform].Get(), NULL, 0); // Only FFTButterflyLUT_Off

	pSRVs[0] = mTextureSRV[FFTTexture_Real0 + 2 * mCurrentTextureIndex].Get();
	pSRVs[1] = mTextureSRV[FFTTexture_Imaginary0 + 2 * mCurrentTextureIndex].Get();
	pSRVs[2] = /*(butterflyLUT == FFTButterflyLUT_On) ? mButterflySRV[pass].Get() :*/ NULL; // Only FFTButterflyLUT_Off
	pUAVs[0] = mTextureUAV[FFTTexture_Real0 + 2 * mNextTextureIndex].Get();
	pUAVs[1] = mTextureUAV[FFTTexture_Imaginary0 + 2 * mNextTextureIndex].Get();

	context->CSSetUnorderedAccessViews(0, 2, pUAVs, NULL);
	context->CSSetShaderResources(0, 3, pSRVs);

	context->Dispatch(1, mPassLength[(pass + 1) % 2], 1);

	pSRVs[0] = NULL; pSRVs[1] = NULL; pSRVs[2] = NULL;
	context->CSSetShaderResources(0, 3, pSRVs);
	pUAVs[0] = NULL; pUAVs[1] = NULL;
	context->CSSetUnorderedAccessViews(0, 2, pUAVs, NULL);

	ToggleTextures();
}
