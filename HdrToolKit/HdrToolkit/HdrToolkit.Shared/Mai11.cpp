#include "pch.h"

#include "Mai11.h"

#include "ColorConverter.h"

#include "compression_tmo.h"

#include "DirectXHelper.h"

using namespace std;
using namespace HdrToolkit;
using namespace DX;
using namespace DirectX;

SourceRenderTexture HdrToolkit::PfsTmo_Mai11(ID3D11Device* device, UINT width, UINT height, std::vector<DirectX::XMFLOAT4> image)
{
	assert(width * height == image.size());

	vector<float> red_in(width * height);
	vector<float> green_in(width * height);
	vector<float> blue_in(width * height);
	vector<float> luminance_in(width * height);

	vector<float> red_out(width*height, 0);
	vector<float> green_out(width*height, 0);
	vector<float> blue_out(width*height, 0);

	for (UINT index = 0; index < width * height; index++)
	{
		XMFLOAT4 color = image.at(index);
		float r = color.x;
		float g = color.y;
		float b = color.z;
		float l = ComputeLuminance(r, g, b);
		red_in.at(index) = r;
		green_in.at(index) = g;
		blue_in.at(index) = b;
		luminance_in.at(index) = l;
	}

	mai11_tonemap(red_in.data(), green_in.data(), blue_in.data(), width, height,
	  red_out.data(), green_out.data(), blue_out.data(),
	  luminance_in.data());

	for (UINT index = 0; index < width * height; index++)
	{
		XMFLOAT4 color;
		color.x = red_out.at(index);
		color.y = green_out.at(index);
		color.z = blue_out.at(index);
		color.w = 1.0f;

		image.at(index) = color;
	}

	SourceRenderTexture texture;
	texture.Create(device, width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, image.data(), width * sizeof(XMFLOAT4));
	return texture;
}