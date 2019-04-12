#pragma once

#include "RenderTexture.h"

#include <vector>

namespace HdrToolkit
{
	SourceRenderTexture PfsTmo_Mantiuk06(ID3D11Device* device, UINT width, UINT height, std::vector<DirectX::XMFLOAT4> image, float contrastFactor, float saturationFactor, bool bcg, int itmax = 200, float tol = 1e-3);
}