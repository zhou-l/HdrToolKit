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

#pragma once

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

enum CFFTState
{
	CFFTState_NotInited,
	CFFTState_Ready
};

class CFFT
{
public:

	CFFT();

	void Init(ID3D11Device *pDevice, ID3D11DeviceContext *pContext, int width, int height);
	void Shutdown();

	void Resize(int width, int height);

	void SetInputSRV(ID3D11ShaderResourceView *src, int width, int height);
	void SetInputSRV(ID3D11ShaderResourceView *real, ID3D11ShaderResourceView *imag, int width, int height);

	void Transform(FFTPassTransform transform);

	void GetDimensions(int& width, int& height);
	void GetSRV(ID3D11ShaderResourceView **ppSRVR, ID3D11ShaderResourceView **ppSRVI);
	void GetUAV(ID3D11UnorderedAccessView **ppUAVR, ID3D11UnorderedAccessView **ppUAVI);
	void GetRTV(ID3D11RenderTargetView **ppRTVR, ID3D11RenderTargetView **ppRTVI);
	void GetNextSRV(ID3D11ShaderResourceView **ppSRVR, ID3D11ShaderResourceView **ppSRVI);
	void GetNextUAV(ID3D11UnorderedAccessView **ppUAVR, ID3D11UnorderedAccessView **ppUAVI);
	void GetNextRTV(ID3D11RenderTargetView **ppRTVR, ID3D11RenderTargetView **ppRTVI);
	void GetTextures(ID3D11Texture2D **ppTextureR, ID3D11Texture2D **ppTextureI);
	CFFTState GetState() { return m_state; }

private:

	void ToggleTextures();
	void BuildButterflyTexture(FFTPass butterflyType, int width);

	void RunDispatchMultiplePass(FFTPass butterflyType, FFTPassTransform transform);

	void CompileShader(FFTPass pass, FFTPassTransform transform);

	int m_textureWidth;
	int m_textureHeight;

	int m_currentTextureIndex;
	int m_nextTextureIndex;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_textures[FFTTexture_Count];
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_textureUAV[FFTTexture_Count];
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureSRV[FFTTexture_Count];
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetViews[FFTTexture_Count];

	int m_passLength[FFTPass_Count];
	int m_butterflyCount[FFTPass_Count];
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_CSButterfly[FFTPass_Count][FFTPassTransform_Count];

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_CSConstantBuffer;

	ID3D11DeviceContext *m_pContext;
	ID3D11Device *m_pDevice;

	CFFTState m_state;
};