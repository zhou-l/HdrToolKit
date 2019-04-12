#pragma once

#include "RenderTexture.h"

#include <vector>

namespace HdrToolkit
{
	SourceRenderTexture PfsTmo_Mai11(ID3D11Device* device, UINT width, UINT height, std::vector<DirectX::XMFLOAT4> image);
}