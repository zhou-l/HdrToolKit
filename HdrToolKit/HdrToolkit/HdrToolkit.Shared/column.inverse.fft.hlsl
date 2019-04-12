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

#define THREAD_GROUP_SIZE 16

Texture2D<float3> TextureSourceR  : register(t0);
Texture2D<float3> TextureSourceI  : register(t1);
RWTexture2D<float3> TextureTargetR  : register(u0);
RWTexture2D<float3> TextureTargetI  : register(u1);

cbuffer Constants : register(b0)
{
	uint PassIndex;
	uint BUTTERFLY_COUNT;
	uint LENGTH;
};

// Input Preprocess Defines:
// TRANSFORM_INVERSE: Defined for inverse fft only
// ROWPASS: defined for tranformation along the x axis
// BUTTERFLY_LUT: defined if the butterfly lookup texture should be used
#define TRANSFORM_INVERSE

static const float PI = 3.14159265f;

void GetButterflyValues(uint passIndex, uint x, out uint2 indices, out float2 weights)
{
	int sectionWidth = 2 << passIndex;
	int halfSectionWidth = sectionWidth / 2;

	int sectionStartOffset = x & ~(sectionWidth - 1);
	int halfSectionOffset = x & (halfSectionWidth - 1);
	int sectionOffset = x & (sectionWidth - 1);

	sincos(2.0*PI*sectionOffset / (float)sectionWidth, weights.y, weights.x);
	weights.y = -weights.y;

	indices.x = sectionStartOffset + halfSectionOffset;
	indices.y = sectionStartOffset + halfSectionOffset + halfSectionWidth;

	if (passIndex == 0)
	{
		indices = reversebits(indices) >> (32 - BUTTERFLY_COUNT) & (LENGTH - 1);
	}
}

[numthreads(THREAD_GROUP_SIZE, THREAD_GROUP_SIZE, 1)]
void main(uint3 position : SV_DispatchThreadID)
{
#ifdef ROWPASS
	int textureSampleX = position.x;
#else
	int textureSampleX = position.y;
#endif

	uint2 Indices;
	float2 Weights;
	GetButterflyValues(PassIndex, textureSampleX, Indices, Weights);

#ifdef ROWPASS
	float3 inputR1 = TextureSourceR.Load(uint3(Indices.x, position.y, 0));
	float3 inputI1 = TextureSourceI.Load(uint3(Indices.x, position.y, 0));

	float3 inputR2 = TextureSourceR.Load(uint3(Indices.y, position.y, 0));
	float3 inputI2 = TextureSourceI.Load(uint3(Indices.y, position.y, 0));
#else
	float3 inputR1 = TextureSourceR.Load(uint3(position.x, Indices.x, 0));
	float3 inputI1 = TextureSourceI.Load(uint3(position.x, Indices.x, 0));

	float3 inputR2 = TextureSourceR.Load(uint3(position.x, Indices.y, 0));
	float3 inputI2 = TextureSourceI.Load(uint3(position.x, Indices.y, 0));
#endif

#ifdef TRANSFORM_INVERSE
	TextureTargetR[position.xy] = (inputR1 + Weights.x * inputR2 + Weights.y * inputI2) * 0.5;
	TextureTargetI[position.xy] = (inputI1 - Weights.y * inputR2 + Weights.x * inputI2) * 0.5;
#else
	TextureTargetR[position.xy] = inputR1 + Weights.x * inputR2 - Weights.y * inputI2;
	TextureTargetI[position.xy] = inputI1 + Weights.y * inputR2 + Weights.x * inputI2;
#endif
}

