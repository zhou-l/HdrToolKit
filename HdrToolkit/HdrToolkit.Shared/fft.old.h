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

#ifndef __FFT__
#define __FFT__

#include <d3d11.h>

class CFFT;

enum FFTTextures
{
	FFTTexture_Real0,
	FFTTexture_Imaginary0,
	FFTTexture_Real1,
	FFTTexture_Imaginary1,
	FFTTexture_Count
};

enum FFTPass
{
	FFTPass_X,
	FFTPass_Y,
	FFTPass_Count
};

enum FFTPassTransform
{
	FFTPassTransform_Forward,
	FFTPassTransform_Inverse,
	FFTPassTransform_Count
};

enum FFTTechnique
{
	FFTTechnique_UAV,
	FFTTechnique_SLM,
	FFTTechnique_Count
};

// Only FFTButterflyLUT_Off
//enum FFTButterflyLUT
//{
//	FFTButterflyLUT_On,
//	FFTButterflyLUT_Off,
//	FFTButterflyLUT_Count
//};

struct CompileThreadInfo
{
	HANDLE handle;
	CFFT *fft;
	FFTTechnique technique;
	FFTPass pass;
	FFTPassTransform transform;
	// FFTButterflyLUT butterflyLUT; // Only FFTButterflyLUT_Off
	bool done;
};

// Only FFTButterflyLUT_Off
#define COMPILE_THREAD_COUNT (FFTTechnique_Count * FFTPassTransform_Count * FFTPass_Count /* * FFTButterflyLUT_Count*/)

enum CFFTState
{
	CFFTState_NotInited,
	CFFTState_Compiling,
	CFFTState_Ready
};

class CFFT
{
public:

	CFFT(bool compileThreaded = false);

	void Init(ID3D11Device *pDevice, ID3D11DeviceContext *pContext, int width, int height);
	void Shutdown();

	void Resize(int width, int height);

	void Update();

	void SetInputSRV(ID3D11ShaderResourceView *src, int width, int height);

	void Transform(FFTTechnique technique, /*FFTButterflyLUT butterflyLUT,*/ FFTPassTransform transform); // Only FFTButterflyLUT_Off

	void GetSRV(ID3D11ShaderResourceView **ppSRVR, ID3D11ShaderResourceView **ppSRVI);
	void GetTextures(ID3D11Texture2D **ppTextureR, ID3D11Texture2D **ppTextureI);
	CFFTState GetState() { return mState; }
	int GetCompiledCount() { return mCompiledCount; }

private:

	void ToggleTextures();
	void BuildButterflyTexture(FFTPass butterflyType, int width);

	void RunDispatchMultiplePass(FFTPass butterflyType, /*FFTButterflyLUT butterflyLUT,*/ FFTPassTransform transform); // Only FFTButterflyLUT_Off
	void RunDispatchSLM(FFTPass pass, /*FFTButterflyLUT butterflyLUT,*/ FFTPassTransform transform); // Only FFTButterflyLUT_Off

	static DWORD CompileThreadFunction(LPVOID *context);
	void CompileShader(FFTTechnique technique, FFTPass pass, /*FFTButterflyLUT butterflyLUT,*/ FFTPassTransform transform); // Only FFTButterflyLUT_Off

	int mTextureWidth;
	int mTextureHeight;

	int mCurrentTextureIndex;
	int mNextTextureIndex;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> mTextures[FFTTexture_Count];
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> mTextureUAV[FFTTexture_Count];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mTextureSRV[FFTTexture_Count];
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRenderTargetViews[FFTTexture_Count];

	int mPassLength[FFTPass_Count];
	int mButterflyCount[FFTPass_Count];
	// Microsoft::WRL::ComPtr<ID3D11Texture2D> mButterflyTexture[FFTPass_Count]; // Only FFTButterflyLUT_Off
	// Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mButterflySRV[FFTPass_Count]; // Only FFTButterflyLUT_Off
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> mCSButterfly[FFTTechnique_Count][FFTPass_Count]/*[FFTButterflyLUT_Count]*/[FFTPassTransform_Count]; // Only FFTButterflyLUT_Off

	Microsoft::WRL::ComPtr<ID3D11Buffer> mCSConstantBuffer;

	ID3D11DeviceContext *mpContext;
	ID3D11Device *mpDevice;

	bool mCompileThreaded;
	int mCompiledCount;
	CompileThreadInfo mThreadInfo[COMPILE_THREAD_COUNT];

	CFFTState mState;

};

#endif