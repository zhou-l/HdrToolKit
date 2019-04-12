#include "pch.h"

#include "Mantiuk06.h"

#include "ColorConverter.h"

#include "contrast_domain.h"

#include "DirectXHelper.h"

using namespace std;
using namespace HdrToolkit;
using namespace DX;
using namespace DirectX;

SourceRenderTexture HdrToolkit::PfsTmo_Mantiuk06(ID3D11Device* device, UINT width, UINT height, std::vector<DirectX::XMFLOAT4> image, float contrastFactor, float saturationFactor, bool bcg, int itmax, float tol)
{
	assert(width * height == image.size());

	vector<float> red(width * height);
	vector<float> green(width * height);
	vector<float> blue(width * height);
	vector<float> luminance(width * height);

	for (UINT index = 0; index < width * height; index++)
	{
		XMFLOAT4 color = image.at(index);
		float r = color.x;
		float g = color.y;
		float b = color.z;
		float l = ComputeLuminance(r, g, b);
		red.at(index) = r;
		green.at(index) = g;
		blue.at(index) = b;
		luminance.at(index) = l;
	}

	int result = tmo_mantiuk06_contmap(width, height, red.data(), green.data(), blue.data(), luminance.data(), contrastFactor, saturationFactor, bcg, itmax, tol);

	ThrowIfFailed((HRESULT)result);

	for (UINT index = 0; index < width * height; index++)
	{
		XMFLOAT4 color;
		color.x = red.at(index);
		color.y = green.at(index);
		color.z = blue.at(index);
		color.w = 1.0f;

		image.at(index) = color;
	}

	SourceRenderTexture texture;
	texture.Create(device, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, image.data(), width * sizeof(XMFLOAT4));
	return texture;
}